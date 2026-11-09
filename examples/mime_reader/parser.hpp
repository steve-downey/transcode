// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// A parser-combinator kit for the MIME reader example, copied from specgen
// (include/beman/specgen/parse/parser.hpp, the same author and license), which
// adapted it from compile-time-scheme (src/smd/smdscheme/parser/parser.hpp).
//
// What is kept is the discipline: a parser is any callable from a cursor to
// `std::expected<parse_state<T>, parse_error>`, and the ordered choice
// `operator|` tries its second alternative only when the first failed without
// consuming input.  A branch that has eaten input has committed, so a failure
// part-way through a token is reported where it happened and not papered over
// by a sibling.  `attempt` opts one parser out of that rule.
//
// What changed: the namespace is example-local, the input is bytes, and the
// combinators the MIME grammar does not use (`digits`, `lexeme`, ...) are
// dropped.  New are `lift` (n-ary `map`, so a production reads left to right
// like its ABNF), `keyword_ci`, `take_while` and `take_while1` (a run of bytes
// as a view of the input), `attempt`, `followed_by`, `eof` and `scan_until`.

#ifndef EXAMPLES_MIME_READER_PARSER_HPP
#define EXAMPLES_MIME_READER_PARSER_HPP

#include <examples/mime_reader/cursor.hpp>

#include <cstddef>
#include <expected>
#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace mime_reader::parse {

// A failure: the offset in the input and a static description of what was
// expected there.  The offset is also the commit signal for `operator|`.
struct parse_error {
    std::size_t offset  = 0;
    const char* message = "";
};

template <class P>
concept parser_like = requires(const P& p, cursor c) { p(c); };

template <class T>
struct parse_state {
    T      value;
    cursor rest;
};

template <class T>
using parse_result = std::expected<parse_state<T>, parse_error>;

// The type a parser produces.
template <class P>
using value_of = decltype(std::declval<const P&>()(std::declval<cursor>())->value);

// A value that carries nothing, for parsers run for their effect on the cursor.
struct unit {};

template <class F>
class parser {
  public:
    constexpr explicit parser(F f);

    constexpr auto operator()(cursor cur) const;

  private:
    F f_;
};

template <class F>
parser(F) -> parser<F>;

template <class F>
constexpr parser<F>::parser(F f) : f_{std::move(f)} {}

template <class F>
constexpr auto parser<F>::operator()(cursor cur) const {
    return f_(cur);
}

// A parser with its type erased.  Every combinator nests its operands by
// value, so the type of a production built from others would contain the whole
// grammar beneath it, and a large grammar takes gigabytes to compile.  A
// grammar's named productions are `rule<T>`s, which cuts the nesting at each
// of them.  The price is one indirect call per rule invocation.
template <class T>
class rule {
  public:
    rule() = default;

    template <parser_like P>
        requires(!std::same_as<std::remove_cvref_t<P>, rule>)
    rule(P p);

    parse_result<T> operator()(cursor cur) const;

  private:
    std::function<parse_result<T>(cursor)> f_;
};

template <class T>
template <parser_like P>
    requires(!std::same_as<std::remove_cvref_t<P>, rule<T>>)
rule<T>::rule(P p) : f_{std::move(p)} {}

template <class T>
parse_result<T> rule<T>::operator()(cursor cur) const {
    return f_(cur);
}

// Succeeds on one byte satisfying `pred`.
[[nodiscard]] constexpr auto satisfy(auto pred, const char* expected) {
    return parser{[pred, expected](cursor cur) -> parse_result<char> {
        if (!cur.empty() && pred(cur.peek())) {
            return parse_state<char>{cur.peek(), cur.bump()};
        }
        return std::unexpected(parse_error{cur.offset(), expected});
    }};
}

[[nodiscard]] constexpr auto char_p(char expected) {
    return satisfy([expected](char c) { return c == expected; }, "expected a particular byte");
}

// A run of zero or more bytes satisfying `pred`, as a view of the input.
[[nodiscard]] constexpr auto take_while(auto pred) {
    return parser{[pred](cursor cur) -> parse_result<std::string_view> {
        std::string_view rest = cur.remaining();
        std::size_t      n    = 0;
        while (n < rest.size() && pred(rest[n])) {
            ++n;
        }
        return parse_state<std::string_view>{rest.substr(0, n), cur.advance(n)};
    }};
}

// A run of one or more bytes satisfying `pred`.
[[nodiscard]] constexpr auto take_while1(auto pred, const char* expected) {
    return parser{[pred, expected](cursor cur) -> parse_result<std::string_view> {
        std::string_view rest = cur.remaining();
        std::size_t      n    = 0;
        while (n < rest.size() && pred(rest[n])) {
            ++n;
        }
        if (n == 0) {
            return std::unexpected(parse_error{cur.offset(), expected});
        }
        return parse_state<std::string_view>{rest.substr(0, n), cur.advance(n)};
    }};
}

// The literal `text`, byte for byte.  A mismatch on the first byte fails
// without consuming, so `operator|` can still try a sibling; a mismatch after
// that has committed.
[[nodiscard]] constexpr auto keyword(std::string text) {
    return parser{[text = std::move(text)](cursor cur) -> parse_result<std::string_view> {
        std::string_view rest = cur.remaining();
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (i == rest.size() || rest[i] != text[i]) {
                return std::unexpected(parse_error{cur.offset() + i, "expected a literal"});
            }
        }
        return parse_state<std::string_view>{rest.substr(0, text.size()), cur.advance(text.size())};
    }};
}

constexpr char ascii_lower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; }

// ABNF strings are case-insensitive: `keyword_ci("base64")` matches "BASE64".
// `text` is lower case.
[[nodiscard]] constexpr auto keyword_ci(std::string text) {
    return parser{[text = std::move(text)](cursor cur) -> parse_result<std::string_view> {
        std::string_view rest = cur.remaining();
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (i == rest.size() || ascii_lower(rest[i]) != text[i]) {
                return std::unexpected(parse_error{cur.offset() + i, "expected a literal"});
            }
        }
        return parse_state<std::string_view>{rest.substr(0, text.size()), cur.advance(text.size())};
    }};
}

// Succeeds only at the end of the input.
[[nodiscard]] constexpr auto eof() {
    return parser{[](cursor cur) -> parse_result<unit> {
        if (!cur.empty()) {
            return std::unexpected(parse_error{cur.offset(), "expected the end of the input"});
        }
        return parse_state<unit>{unit{}, cur};
    }};
}

// Runs each parser in order and applies `f` to their values.  The first
// failure is the result.  `lift(f, p)` is `map`.
template <class F, parser_like... Ps>
[[nodiscard]] constexpr auto lift(F f, Ps... ps) {
    using V = std::invoke_result_t<const F&, value_of<Ps>...>;
    return parser{[f, ps = std::tuple{ps...}](cursor cur) -> parse_result<V> {
        std::tuple<std::optional<value_of<Ps>>...> values;
        parse_error                                error{};
        auto                                       step = [&]<std::size_t I>() {
            auto r = std::get<I>(ps)(cur);
            if (!r) {
                error = r.error();
                return false;
            }
            std::get<I>(values) = std::move(r->value);
            cur                 = r->rest;
            return true;
        };
        auto run = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return (step.template operator()<Is>() && ...);
        };
        if (!run(std::index_sequence_for<Ps...>{})) {
            return std::unexpected(error);
        }
        return std::apply([&](auto&... v) { return parse_state<V>{f(std::move(*v)...), cur}; }, values);
    }};
}

template <parser_like P>
[[nodiscard]] constexpr auto map(P p, auto f) {
    return lift(std::move(f), std::move(p));
}

// `pa pb`, keeping the value of `pa`.
template <parser_like PA, parser_like PB>
[[nodiscard]] constexpr auto sequence_left(PA pa, PB pb) {
    return lift([](auto a, auto&&) { return a; }, std::move(pa), std::move(pb));
}

// `pa pb`, keeping the value of `pb`.
template <parser_like PA, parser_like PB>
[[nodiscard]] constexpr auto sequence_right(PA pa, PB pb) {
    return lift([](auto&&, auto b) { return b; }, std::move(pa), std::move(pb));
}

// Ordered choice, `pa / pb`: `pb` is tried only when `pa` failed at the offset
// where it started.  Both must produce the same type.
template <parser_like PA, parser_like PB>
[[nodiscard]] constexpr auto operator|(PA pa, PB pb) {
    return parser{[pa, pb](cursor cur) {
        auto ra = pa(cur);
        if (ra.has_value() || ra.error().offset != cur.offset()) {
            return ra;
        }
        return pb(cur);
    }};
}

// `p` with its failure moved back to where it started, so that `operator|`
// will try the next alternative even if `p` consumed input before failing.
template <parser_like P>
[[nodiscard]] constexpr auto attempt(P p) {
    return parser{[p](cursor cur) {
        auto r = p(cur);
        if (!r.has_value()) {
            r.error().offset = cur.offset();
        }
        return r;
    }};
}

// Succeeds, consuming nothing, when `p` would succeed here: `&p`.
template <parser_like P>
[[nodiscard]] constexpr auto followed_by(P p) {
    return parser{[p](cursor cur) -> parse_result<unit> {
        auto r = p(cur);
        if (!r.has_value()) {
            return std::unexpected(parse_error{cur.offset(), r.error().message});
        }
        return parse_state<unit>{unit{}, cur};
    }};
}

// `*p`: zero or more, collected.  Always succeeds, and stops at the first
// failure of `p`, committed or not.  `p` must consume input when it succeeds.
template <parser_like P>
[[nodiscard]] constexpr auto many(P p) {
    return parser{[p](cursor cur) {
        using V = value_of<P>;
        std::vector<V> result;
        while (auto r = p(cur)) {
            result.push_back(std::move(r->value));
            cur = r->rest;
        }
        return parse_result<std::vector<V>>{parse_state<std::vector<V>>{std::move(result), cur}};
    }};
}

// `1*p`: one or more, collected.
template <parser_like P>
[[nodiscard]] constexpr auto some(P p) {
    return parser{[p](cursor cur) {
        using V = value_of<P>;
        std::vector<V> result;
        auto           first = p(cur);
        if (!first.has_value()) {
            return parse_result<std::vector<V>>{std::unexpected(first.error())};
        }
        result.push_back(std::move(first->value));
        cur = first->rest;
        while (auto r = p(cur)) {
            result.push_back(std::move(r->value));
            cur = r->rest;
        }
        return parse_result<std::vector<V>>{parse_state<std::vector<V>>{std::move(result), cur}};
    }};
}

// `[p]`: `p` if it matches without having failed part-way, else nothing.
template <parser_like P>
[[nodiscard]] constexpr auto opt(P p) {
    return parser{[p](cursor cur) {
        using V = value_of<P>;
        auto r  = p(cur);
        if (r.has_value()) {
            return parse_result<std::optional<V>>{parse_state<std::optional<V>>{std::move(r->value), r->rest}};
        }
        if (r.error().offset != cur.offset()) {
            return parse_result<std::optional<V>>{std::unexpected(r.error())};
        }
        return parse_result<std::optional<V>>{parse_state<std::optional<V>>{std::nullopt, cur}};
    }};
}

template <class T>
struct scanned {
    std::string_view skipped; // the bytes passed over
    T                value;   // what `p` produced where it matched
};

// Skips bytes until `p` matches, and yields the skipped bytes with `p`'s value.
// A failure of `p` at one offset only moves the scan on to the next, which is
// what a preamble, a body part, or an epilogue is: everything up to a marker.
template <parser_like P>
[[nodiscard]] constexpr auto scan_until(P p, const char* expected) {
    return parser{[p, expected](cursor cur) -> parse_result<scanned<value_of<P>>> {
        for (std::size_t n = 0; n <= cur.remaining().size(); ++n) {
            if (auto r = p(cur.advance(n))) {
                return parse_state<scanned<value_of<P>>>{
                    scanned<value_of<P>>{cur.remaining().substr(0, n), std::move(r->value)}, r->rest};
            }
        }
        return std::unexpected(parse_error{cur.offset(), expected});
    }};
}

} // namespace mime_reader::parse

#endif // EXAMPLES_MIME_READER_PARSER_HPP
