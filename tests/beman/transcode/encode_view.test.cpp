// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/encode_view.hpp>

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/detail/table_codec.hpp>
#include <beman/transcode/whatwg/error.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <iterator>
#include <list>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

// Latin-1 table: bytes 0x80-0xFF identity-map to U+0080-U+00FF
inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = table_codec<latin1_upper>;

// Sparse table: only a few codepoints mapped, 0x80 unmapped
inline constexpr std::array<char32_t, 128> sparse_table = [] {
    std::array<char32_t, 128> t{};
    t[1] = U'\x00A1';
    t[2] = U'\x00A2';
    return t;
}();

using sparse_codec = table_codec<sparse_table>;

struct replacement_aware_codec {
    constexpr encode_result encode_one(char32_t cp) {
        encode_result result{};
        if (cp == U'\xFFFD') {
            result.bytes[0] = 'R';
            result.count    = 1;
        } else {
            result.is_error = true;
        }
        return result;
    }
};

// ---------------------------------------------------------------------------
// encode_view basic tests
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: ASCII codepoints round-trip", "[encode_view]") {
    std::u32string src = U"ABC";
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == "ABC");
}

TEST_CASE("encode_view: upper-half codepoints encode correctly", "[encode_view]") {
    std::u32string src{U'\x00C0', U'\x00C1'};
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xC0');
    CHECK(result[1] == '\xC1');
}

TEST_CASE("encode_view: unmapped codepoint yields replacement byte '?'", "[encode_view]") {
    std::u32string src{U'\x1F600'}; // emoji, not in Latin-1
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("encode_view: custom codecs receive U+FFFD for ill-formed UTF-32", "[encode_view]") {
    std::u32string src{static_cast<char32_t>(0xD800)};
    std::string    result;
    for (char c : src | encode(replacement_aware_codec{}))
        result.push_back(c);
    CHECK(result == "R");
}

TEST_CASE("encode_view: empty input produces empty output", "[encode_view]") {
    std::u32string src;
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result.empty());
}

TEST_CASE("encode_view: pipe with span of char32_t", "[encode_view]") {
    std::array<char32_t, 3> src{U'H', U'i', U'!'};
    std::string             result;
    for (char c : std::span<const char32_t>(src) | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == "Hi!");
}

// ---------------------------------------------------------------------------
// encode_view with error_mode::report tests
// ---------------------------------------------------------------------------

TEST_CASE("encode_view with error_mode::report: ASCII codepoints succeed", "[encode_view]") {
    std::u32string src  = U"Hi";
    auto           view = src | encode_or_error(latin1_codec{});
    auto           it   = view.begin();

    auto r1 = *it;
    CHECK(r1.has_value());
    CHECK(r1.value() == 'H');

    ++it;
    auto r2 = *it;
    CHECK(r2.has_value());
    CHECK(r2.value() == 'i');

    ++it;
    CHECK(it == std::default_sentinel);
}

TEST_CASE("encode_view with error_mode::report: unmapped codepoint yields unexpected", "[encode_view]") {
    std::u32string src{U'\x1F600'}; // emoji, not in Latin-1
    auto           view = src | encode_or_error(latin1_codec{});
    auto           it   = view.begin();

    auto r = *it;
    CHECK(!r.has_value());
    CHECK(r.error() == whatwg::error::unmapped_codepoint);
}

TEST_CASE("encode_view with error_mode::report: custom codecs do not receive ill-formed UTF-32", "[encode_view]") {
    std::u32string src{static_cast<char32_t>(0x110000)};
    auto           view = src | encode_or_error(replacement_aware_codec{});
    auto           r    = *view.begin();
    REQUIRE(!r.has_value());
    CHECK(r.error() == whatwg::error::out_of_range);
}

TEST_CASE("encode_view with error_mode::report: sparse table unmapped yields unexpected", "[encode_view]") {
    std::u32string src{U'\x0080'}; // index 0 is 0x80, which is unmapped in sparse_table
    auto           view = src | encode_or_error(sparse_codec{});
    auto           it   = view.begin();

    auto r = *it;
    CHECK(!r.has_value());
    CHECK(r.error() == whatwg::error::unmapped_codepoint);
}

TEST_CASE("encode_view with error_mode::report: mixed success and error", "[encode_view]") {
    std::u32string                                  src{U'A', U'\x1F600', U'B'}; // A, emoji, B
    std::vector<std::expected<char, whatwg::error>> result;
    for (auto v : src | encode_or_error(latin1_codec{}))
        result.push_back(v);
    REQUIRE(result.size() == 3);
    CHECK(result[0].has_value());
    CHECK(result[0].value() == 'A');
    CHECK(!result[1].has_value());
    CHECK(result[1].error() == whatwg::error::unmapped_codepoint);
    CHECK(result[2].has_value());
    CHECK(result[2].value() == 'B');
}

// ---------------------------------------------------------------------------
// Round-trip: decode then encode
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: round-trip decode then encode with Latin-1", "[encode_view]") {
    std::string src{'\x41', '\xC0', '\xC1'}; // A, 0xC0, 0xC1 in Latin-1
    std::string result;
    for (char c : std::span<const char>(src) | decode(latin1_codec{}) | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == src);
}

TEST_CASE("encode_view: round-trip ASCII string", "[encode_view]") {
    std::string src = "hello world";
    std::string result;
    for (char c : std::span<const char>(src) | decode(latin1_codec{}) | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == src);
}

// ---------------------------------------------------------------------------
// Constexpr tests
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: constexpr encode_one via table_codec", "[encode_view]") {
    CHECK(constify(latin1_codec{}.encode_one(U'A').bytes[0]) == 'A');
    CHECK(constify(latin1_codec{}.encode_one(U'A').count) == 1);
    CHECK(constify(latin1_codec{}.encode_one(U'\x00C0').bytes[0]) == static_cast<unsigned char>('\xC0'));
    CHECK(constify(latin1_codec{}.encode_one(U'\x1F600').is_error) == true);
}

TEST_CASE("encode_view: constexpr encode view iteration", "[encode_view]") {
    constexpr auto first_byte = [] {
        std::array<char32_t, 2> src{U'H', U'i'};
        auto                    view = src | encode(latin1_codec{});
        return *view.begin();
    }();
    CHECK(constify(first_byte) == 'H');
}

TEST_CASE("encode_view: constexpr UTF-32 validation", "[encode_view]") {
    constexpr auto replacement = [] {
        std::array<char32_t, 1> src{static_cast<char32_t>(0xD800)};
        auto                    view = src | encode(replacement_aware_codec{});
        return *view.begin();
    }();
    CHECK(constify(replacement) == 'R');
}

TEST_CASE("encode_view: constexpr round-trip", "[encode_view]") {
    constexpr auto result = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xC1};
        std::array<char, 3>          out{};
        int                          i = 0;
        for (char c : src | decode(latin1_codec{}) | encode(latin1_codec{}))
            out[static_cast<std::size_t>(i++)] = c;
        return out;
    }();
    CHECK(constify(result[0]) == '\x41');
    CHECK(constify(result[1]) == '\xC0');
    CHECK(constify(result[2]) == '\xC1');
}

// ---------------------------------------------------------------------------
// base() protocol
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: view base() returns underlying range", "[encode_view]") {
    std::u32string src  = U"AB";
    auto           view = src | encode(latin1_codec{});
    CHECK(view.base().size() == 2);
    CHECK(view.base()[0] == U'A');
}

TEST_CASE("encode_view: table_codec encodes the lowest index for a duplicated code point", "[encode_view]") {
    // Two slots hold U+00C0; the first wins.  A later slot holds U+00C1 alone.
    constexpr auto twice = [] {
        std::array<char32_t, 128> t{};
        t[3]  = U'\x00C0';
        t[9]  = U'\x00C0';
        t[64] = U'\x00C1';
        return t;
    }();
    using twice_codec = table_codec<twice>;
    CHECK(twice_codec{}.encode_one(U'\x00C0').bytes[0] == 0x83);
    CHECK(twice_codec{}.encode_one(U'\x00C1').bytes[0] == 0xC0);
    CHECK(twice_codec{}.encode_one(U'\x00C2').is_error);
    CHECK(constify(twice_codec{}.encode_one(U'\x00C1').bytes[0]) == 0xC0);
}

TEST_CASE("encode_view: full_table_codec encodes by table position and rejects unmapped", "[encode_view]") {
    constexpr auto flipped = [] {
        std::array<char32_t, 256> t{};
        for (std::size_t i = 1; i < 256; ++i)
            t[i] = static_cast<char32_t>(255 - i);
        return t;
    }();
    using flipped_codec = full_table_codec<flipped>;
    CHECK(flipped_codec{}.encode_one(U'A').bytes[0] == 255 - 'A');
    CHECK(flipped_codec{}.encode_one(U'\x00FE').bytes[0] == 1);
    // Position 0 holds 0, the unmapped marker, so neither U+00FF nor U+0000 encodes.
    CHECK(flipped_codec{}.encode_one(U'\x00FF').is_error);
    CHECK(flipped_codec{}.encode_one(U'\0').is_error);
    CHECK(constify(flipped_codec{}.encode_one(U'A').bytes[0]) == 255 - 'A');
}

// ---------------------------------------------------------------------------
// borrowed_range propagation
// ---------------------------------------------------------------------------

namespace {
using u32_span_t   = std::span<const char32_t>;
using enc_borrowed = decltype(std::declval<u32_span_t>() | encode(latin1_codec{}));
using enc_owning   = decltype(std::u32string{} | encode(latin1_codec{}));

static_assert(std::ranges::borrowed_range<enc_borrowed>);
static_assert(!std::ranges::borrowed_range<enc_owning>);
static_assert(std::ranges::borrowed_range<decltype(std::declval<u32_span_t>() | encode_or_error(latin1_codec{}))>);

constexpr unsigned char find_through_span() {
    constexpr std::array<char32_t, 3> cps{U'A', U'B', U'\x00C0'};
    auto it = std::ranges::find(std::span<const char32_t>{cps} | encode(latin1_codec{}), static_cast<char>(0xC0));
    return static_cast<unsigned char>(*it);
}
} // namespace

TEST_CASE("encode_view: borrowed_range over a span", "[encode_view]") {
    std::array<char32_t, 3> cps{U'A', U'B', U'\x00C0'};
    auto it = std::ranges::find(std::span<const char32_t>{cps} | encode(latin1_codec{}), static_cast<char>(0xC0));
    static_assert(!std::same_as<decltype(it), std::ranges::dangling>);
    CHECK(static_cast<unsigned char>(*it) == 0xC0);
}

TEST_CASE("encode_view: borrowed find in a constant expression", "[encode_view]") {
    CHECK(constify(find_through_span()) == 0xC0);
}

// ---------------------------------------------------------------------------
// flushable_text_encoder: a toy shift-state encoder
// ---------------------------------------------------------------------------

namespace {
struct shift_state_encoder {
    bool shifted{false};

    constexpr encode_result encode_one(char32_t cp) {
        encode_result r{};
        if (cp < 0x80) {
            if (shifted) {
                r.bytes[r.count++] = 0x0F;
                shifted            = false;
            }
            r.bytes[r.count++] = static_cast<unsigned char>(cp);
        } else if (cp <= 0xFF) {
            if (!shifted) {
                r.bytes[r.count++] = 0x0E;
                shifted            = true;
            }
            r.bytes[r.count++] = static_cast<unsigned char>(cp);
        } else {
            r.is_error = true;
        }
        return r;
    }

    constexpr std::optional<encode_result> flush() {
        if (!shifted)
            return std::nullopt;
        shifted = false;
        encode_result r{};
        r.bytes[0] = 0x0F;
        r.count    = 1;
        return r;
    }

    friend constexpr bool operator==(const shift_state_encoder&, const shift_state_encoder&) = default;
};

static_assert(text_encoder<shift_state_encoder>);
static_assert(flushable_text_encoder<shift_state_encoder>);
static_assert(!flushable_text_encoder<latin1_codec>);

// A single-pass range of char32_t: an input range that is not a forward range.
struct input_only_scalars : std::ranges::view_interface<input_only_scalars> {
    const char32_t* first_{nullptr};
    const char32_t* last_{nullptr};

    struct iterator {
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;

        const char32_t* p_{nullptr};

        constexpr char32_t  operator*() const { return *p_; }
        constexpr iterator& operator++() {
            ++p_;
            return *this;
        }
        constexpr void        operator++(int) { ++p_; }
        friend constexpr bool operator==(const iterator& it, const iterator& e) { return it.p_ == e.p_; }
    };

    input_only_scalars() = default;
    constexpr explicit input_only_scalars(std::u32string_view s) : first_(s.data()), last_(s.data() + s.size()) {}
    constexpr iterator begin() const { return {first_}; }
    constexpr iterator end() const { return {last_}; }
};

static_assert(std::ranges::input_range<input_only_scalars>);
static_assert(!std::ranges::forward_range<input_only_scalars>);

template <typename V>
std::string collect_bytes(V&& v) {
    std::string out;
    for (auto b : v)
        out.push_back(static_cast<char>(b));
    return out;
}

constexpr std::array<char, 4> shift_encode_a_e_acute() {
    std::array<char, 4> out{};
    std::size_t         n = 0;
    for (char c : std::u32string_view{U"aé"} | encode(shift_state_encoder{}))
        out[n++] = c;
    return out;
}
} // namespace

TEST_CASE("encode_view: flushable encoder flushes at end of input", "[encode_view][flush]") {
    using namespace std::string_literals;
    CHECK(collect_bytes(std::u32string_view{U"aé"} | encode(shift_state_encoder{})) == "a\x0E\xE9\x0F"s);
    CHECK(collect_bytes(std::u32string_view{U"éa"} | encode(shift_state_encoder{})) == "\x0E\xE9\x0F"
                                                                                       "a"s);
    CHECK(collect_bytes(std::u32string_view{U"ab"} | encode(shift_state_encoder{})) == "ab"s);
    CHECK(collect_bytes(std::u32string_view{U""} | encode(shift_state_encoder{})).empty());
}

TEST_CASE("encode_view: flushable encoder in report mode", "[encode_view][flush]") {
    std::string out;
    for (auto r : std::u32string_view{U"aé"} | encode_or_error(shift_state_encoder{})) {
        REQUIRE(r.has_value());
        out.push_back(*r);
    }
    CHECK(out == "a\x0E\xE9\x0F");
}

TEST_CASE("encode_view: flushable encoder, forward base counts the flushed byte", "[encode_view][flush]") {
    auto v = std::u32string_view{U"aé"} | encode(shift_state_encoder{});
    CHECK(std::ranges::distance(v) == 4);
}

TEST_CASE("encode_view: flushable encoder, input-only base", "[encode_view][flush]") {
    using namespace std::string_literals;
    auto in = input_only_scalars{U"aé"};
    CHECK(collect_bytes(in | encode(shift_state_encoder{})) == "a\x0E\xE9\x0F"s);
}

TEST_CASE("encode_view: iterators before and after the flushed byte differ", "[encode_view][flush]") {
    auto v        = std::u32string_view{U"aé"} | encode(shift_state_encoder{});
    auto b        = v.begin();
    auto at_flush = std::ranges::next(b, 3); // 'a', SO, 0xE9 consumed; at the flushed SI
    auto after    = std::ranges::next(b, 4);
    CHECK(at_flush != after);
    CHECK(*at_flush == static_cast<char>(0x0F));
    CHECK(after == v.end());
    CHECK(at_flush != v.end());
}

TEST_CASE("encode_view: flushable encoder in a constant expression", "[encode_view][flush]") {
    constexpr auto bytes = constify(shift_encode_a_e_acute());
    CHECK(bytes[0] == 'a');
    CHECK(static_cast<unsigned char>(bytes[1]) == 0x0E);
    CHECK(static_cast<unsigned char>(bytes[2]) == 0xE9);
    CHECK(static_cast<unsigned char>(bytes[3]) == 0x0F);
}

// ---------------------------------------------------------------------------
// flush(): an error result is treated as an encode_one error is
// ---------------------------------------------------------------------------

namespace {
// Encodes ASCII as itself.  At the end of the input flush() returns one error
// result, which carries `Fill` bytes of '!' (none if Fill is 0), then nullopt.
template <int Fill>
struct flush_error_encoder {
    int flushes{0};

    constexpr encode_result encode_one(char32_t cp) {
        encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    constexpr std::optional<encode_result> flush() {
        if (flushes++ != 0)
            return std::nullopt;
        encode_result r{};
        r.is_error = true;
        for (int i = 0; i < Fill; ++i)
            r.bytes[r.count++] = '!';
        return r;
    }
};

static_assert(flushable_text_encoder<flush_error_encoder<2>>);

template <int Fill>
constexpr std::array<char, 4> flush_error_replacement() {
    std::array<char, 4> out{};
    std::size_t         n = 0;
    for (char c : std::u32string_view{U"a"} | encode(flush_error_encoder<Fill>{}))
        out[n++] = c;
    return out;
}

template <int Fill>
constexpr int flush_error_report_count() {
    int errors = 0;
    int total  = 0;
    for (auto r : std::u32string_view{U"a"} | encode_or_error(flush_error_encoder<Fill>{})) {
        ++total;
        if (!r)
            ++errors;
    }
    return errors * 10 + total;
}
} // namespace

TEST_CASE("encode_view: a flush() error with bytes yields them in replacement mode", "[encode_view][flush]") {
    using namespace std::string_literals;
    CHECK(collect_bytes(std::u32string_view{U"a"} | encode(flush_error_encoder<2>{})) == "a!!"s);
    CHECK(collect_bytes(std::u32string_view{U"a"} | encode(flush_error_encoder<0>{})) == "a?"s);
}

TEST_CASE("encode_view: a flush() error is one unexpected element in report mode", "[encode_view][flush]") {
    for (auto r : std::u32string_view{U"a"} | encode_or_error(flush_error_encoder<2>{})) {
        if (!r) {
            CHECK(r.error() == whatwg::error::unmapped_codepoint);
        }
    }
    CHECK(flush_error_report_count<2>() == 12);
    CHECK(flush_error_report_count<0>() == 12);
}

TEST_CASE("encode_view: a flush() error in a constant expression", "[encode_view][flush]") {
    constexpr auto with_bytes = constify(flush_error_replacement<2>());
    CHECK(with_bytes[0] == 'a');
    CHECK(with_bytes[1] == '!');
    CHECK(with_bytes[2] == '!');
    constexpr auto without = constify(flush_error_replacement<0>());
    CHECK(without[1] == '?');
    CHECK(constify(flush_error_report_count<2>()) == 12);
    CHECK(constify(flush_error_report_count<0>()) == 12);
}

// ---------------------------------------------------------------------------
// encode_result: an error that carries its own replacement bytes
// ---------------------------------------------------------------------------

namespace {
// Encodes ASCII as itself.  Anything else is an error, and the error carries
// the two bytes "<>" as its own replacement.
struct bracket_replacement_encoder {
    constexpr encode_result encode_one(char32_t cp) {
        encode_result r{};
        if (cp < 0x80) {
            r.bytes[0] = static_cast<unsigned char>(cp);
            r.count    = 1;
        } else {
            r.is_error = true;
            r.bytes[0] = '<';
            r.bytes[1] = '>';
            r.count    = 2;
        }
        return r;
    }
};
static_assert(text_encoder<bracket_replacement_encoder>);

constexpr std::array<char, 8> bracket_encode_a_e_acute_b() {
    std::array<char, 8> out{};
    std::size_t         n = 0;
    for (auto b : std::u32string_view{U"aéb"} | encode(bracket_replacement_encoder{}))
        out[n++] = b;
    return out;
}
} // namespace

TEST_CASE("encode_view: an error with bytes yields them as the replacement", "[encode_view][replacement_bytes]") {
    using namespace std::string_literals;
    CHECK(collect_bytes(std::u32string_view{U"aéb"} | encode(bracket_replacement_encoder{})) == "a<>b"s);
    CHECK(collect_bytes(std::u32string_view{U"é"} | encode(bracket_replacement_encoder{})) == "<>"s);
    CHECK(collect_bytes(std::u32string_view{U"ab"} | encode(bracket_replacement_encoder{})) == "ab"s);
}

TEST_CASE("encode_view: an error with no bytes still yields '?'", "[encode_view][replacement_bytes]") {
    using namespace std::string_literals;
    CHECK(collect_bytes(std::u32string_view{U"aéb"} | encode(replacement_aware_codec{})) == "???"s);
}

TEST_CASE("encode_view: report mode ignores an error's replacement bytes", "[encode_view][replacement_bytes]") {
    std::vector<std::expected<char, whatwg::error>> out;
    for (auto r : std::u32string_view{U"aéb"} | encode_or_error(bracket_replacement_encoder{}))
        out.push_back(r);
    REQUIRE(out.size() == 3);
    CHECK(out[0] == 'a');
    REQUIRE_FALSE(out[1].has_value());
    CHECK(out[1].error() == whatwg::error::unmapped_codepoint);
    CHECK(out[2] == 'b');
}

TEST_CASE("encode_view: replacement bytes in a constant expression", "[encode_view][replacement_bytes]") {
    constexpr auto bytes = constify(bracket_encode_a_e_acute_b());
    CHECK(bytes[0] == 'a');
    CHECK(bytes[1] == '<');
    CHECK(bytes[2] == '>');
    CHECK(bytes[3] == 'b');
}

// ---------------------------------------------------------------------------
// random_access_encode_view: a single-byte encoder
// ---------------------------------------------------------------------------

namespace {

// Latin-1: each scalar value up to U+00FF is its own byte.
struct latin1_encoder {
    constexpr std::optional<unsigned char> encode_byte(char32_t cp) const {
        if (cp <= 0xFF)
            return static_cast<unsigned char>(cp);
        return std::nullopt;
    }
    constexpr encode_result encode_one(char32_t cp) const {
        encode_result r{};
        if (auto b = encode_byte(cp)) {
            r.bytes[0] = *b;
            r.count    = 1;
        } else {
            r.is_error = true;
        }
        return r;
    }
};

static_assert(single_byte_text_encoder<latin1_encoder>);
static_assert(!single_byte_text_encoder<replacement_aware_codec>);
static_assert(!single_byte_text_encoder<latin1_codec>);

constexpr std::array<char, 4> latin1_through_view() {
    constexpr std::array<char32_t, 4> src{U'a', U'\xE9', U'\x3042', U'z'};
    std::array<char, 4>               out{};
    auto                              v = std::span<const char32_t>{src} | encode(latin1_encoder{});
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] = v[static_cast<std::ptrdiff_t>(i)];
    return out;
}

} // namespace

TEST_CASE("encode_view: a single-byte encoder over a random-access base is random access",
          "[encode_view][random_access]") {
    const std::u32string src = U"a\xE9\x3042z";
    auto                 v   = std::span<const char32_t>{src} | encode(latin1_encoder{});
    using view_t             = decltype(v);
    STATIC_REQUIRE(std::ranges::random_access_range<view_t>);
    STATIC_REQUIRE(std::ranges::sized_range<view_t>);
    STATIC_REQUIRE(std::ranges::borrowed_range<view_t>);
    CHECK(v.size() == 4);
    CHECK(v.end() - v.begin() == 4);
    auto it = v.begin();
    CHECK(it[0] == 'a');
    CHECK(it[1] == '\xE9');
    CHECK(it[2] == '?');
    CHECK(it[3] == 'z');
    CHECK(*(v.end() - 1) == 'z');

    std::string reversed;
    for (char c : v | std::views::reverse)
        reversed.push_back(c);
    CHECK(reversed == "z?\xE9"
                      "a");
}

TEST_CASE("encode_view: a single-byte encoder in report mode", "[encode_view][random_access]") {
    const std::u32string src{U'a', U'\x3042', char32_t{0xD800}, char32_t{0x110000}, U'\xE9'};
    auto                 v = std::span<const char32_t>{src} | encode_or_error(latin1_encoder{});
    STATIC_REQUIRE(std::ranges::random_access_range<decltype(v)>);
    REQUIRE(v.size() == 5);
    CHECK(v[0] == 'a');
    REQUIRE_FALSE(v[1].has_value());
    CHECK(v[1].error() == whatwg::error::unmapped_codepoint);
    REQUIRE_FALSE(v[2].has_value());
    CHECK(v[2].error() == whatwg::error::surrogate_code_point);
    REQUIRE_FALSE(v[3].has_value());
    CHECK(v[3].error() == whatwg::error::out_of_range);
    CHECK(v[4] == '\xE9');
}

TEST_CASE("encode_view: a single-byte encoder replaces ill-formed UTF-32 with U+FFFD",
          "[encode_view][random_access]") {
    // U+FFFD is not representable in Latin-1, so the replacement is '?'.
    const std::u32string src{U'a', char32_t{0xD800}, char32_t{0x110000}};
    auto                 v = std::span<const char32_t>{src} | encode(latin1_encoder{});
    CHECK(v[0] == 'a');
    CHECK(v[1] == '?');
    CHECK(v[2] == '?');
}

TEST_CASE("encode_view: a single-byte encoder over a non-random-access base is an encode_view",
          "[encode_view][random_access]") {
    std::list<char32_t> src{U'a', U'\xE9'};
    auto                v = src | encode(latin1_encoder{});
    STATIC_REQUIRE(!std::ranges::random_access_range<decltype(v)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(v)>);
    CHECK(collect_bytes(v) == "a\xE9");
}

TEST_CASE("encode_view: a single-byte encoder in a constant expression", "[encode_view][random_access]") {
    constexpr auto bytes = constify(latin1_through_view());
    CHECK(bytes[0] == 'a');
    CHECK(bytes[1] == '\xE9');
    CHECK(bytes[2] == '?');
    CHECK(bytes[3] == 'z');
}
