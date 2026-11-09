// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <examples/mime_reader/grammar.hpp>
#include <examples/mime_reader/grammar.hpp>

#include <examples/mime_reader/cursor.hpp>
#include <examples/mime_reader/parser.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mime_reader {

namespace p = parse;

namespace {

template <class T>
using rule = p::rule<T>;

// ---------------------------------------------------------------------------
// Character classes
// ---------------------------------------------------------------------------

constexpr unsigned byte_of(char c) { return static_cast<unsigned char>(c); }

// WSP = SP / HTAB
constexpr bool is_wsp(char c) { return c == ' ' || c == '\t'; }

// ftext = %d33-57 / %d59-126      ; printable US-ASCII except ":"
constexpr bool is_ftext(char c) { return byte_of(c) >= 33 && byte_of(c) <= 126 && c != ':'; }

// tspecials = "(" / ")" / "<" / ">" / "@" / "," / ";" / ":" / "\" / <"> /
//             "/" / "[" / "]" / "?" / "="
// token = 1*<any (US-ASCII) CHAR except SPACE, CTLs, or tspecials>
constexpr bool is_token_char(char c) {
    return byte_of(c) > 32 && byte_of(c) < 127 &&
           std::string_view{"()<>@,;:\\\"/[]?="}.find(c) == std::string_view::npos;
}

// especials = "(" / ")" / "<" / ">" / "@" / "," / ";" / ":" / "\" / <"> /
//             "/" / "[" / "]" / "?" / "." / "="
// token = 1*<Any CHAR except SPACE, CTLs, and especials>     (RFC 2047)
constexpr bool is_encoded_word_token_char(char c) {
    return byte_of(c) > 32 && byte_of(c) < 127 &&
           std::string_view{"()<>@,;:\\\"/[]?.="}.find(c) == std::string_view::npos;
}

// encoded-text = 1*<Any printable ASCII character other than "?" or SPACE>
constexpr bool is_encoded_text_char(char c) { return byte_of(c) > 32 && byte_of(c) < 127 && c != '?'; }

constexpr bool is_not_line_break(char c) { return c != '\r' && c != '\n'; }

constexpr bool is_not_wsp(char c) { return !is_wsp(c); }

constexpr bool is_any(char) { return true; }

std::string lower(std::string_view s) {
    std::string out;
    for (char c : s) {
        out.push_back(p::ascii_lower(c));
    }
    return out;
}

std::string to_str(std::string_view s) { return std::string{s}; }

// ---------------------------------------------------------------------------
// RFC 5322: header fields
// ---------------------------------------------------------------------------

// A line break: CRLF, or a bare LF.
//
//     eol = CRLF / LF
rule<std::string_view> eol() { return p::keyword("\r\n") | p::keyword("\n"); }

// The line break inside a folded field: the one that is followed by white space.
//
//     FWS = [*WSP CRLF] 1*WSP            ; the line break and what follows it
rule<std::string_view> fold() {
    return p::map(p::attempt(p::sequence_left(eol(), p::followed_by(p::satisfy(is_wsp, "expected white space")))),
                  [](std::string_view) { return std::string_view{}; });
}

//     field-name = 1*ftext
rule<std::string_view> field_name() { return p::take_while1(is_ftext, "expected a header field name"); }

// The field's text, folds removed and trimmed.
//
//     unstructured = (*([FWS] VCHAR) *WSP)
rule<std::string> field_body() {
    auto line_text = p::take_while1(is_not_line_break, "expected header text");
    return p::map(p::many(line_text | fold()), [](const std::vector<std::string_view>& pieces) {
        std::string text;
        for (std::string_view piece : pieces) {
            text.append(piece);
        }
        std::size_t first = text.find_first_not_of(" \t");
        if (first == std::string::npos) {
            return std::string{};
        }
        return text.substr(first, text.find_last_not_of(" \t") - first + 1);
    });
}

// The end of the last field of a message that stops there, with no final line
// break, is accepted.
rule<p::unit> field_end() {
    return p::map(eol(), [](std::string_view) { return p::unit{}; }) | p::eof();
}

//     field = field-name ":" unstructured CRLF
rule<header_field> field() {
    return p::lift(
        [](std::string_view name, char, std::string value, p::unit) {
            return header_field{std::string{name}, std::move(value)};
        },
        field_name(),
        p::char_p(':'),
        field_body(),
        field_end());
}

// A message is its fields, a blank line, and a body (RFC 5322 section 3.5); a
// body part of a multipart is the same (RFC 2046 section 5.1.1).
//
//     message   = *field [CRLF body]
//     body-part = MIME-part-headers [CRLF *OCTET]
rule<entity_syntax> body_part_syntax() {
    return p::lift([](std::vector<header_field> headers,
                      std::optional<std::string_view>,
                      std::string_view body) { return entity_syntax{std::move(headers), body}; },
                   p::many(field()),
                   p::opt(eol()),
                   p::take_while(is_any));
}

// ---------------------------------------------------------------------------
// RFC 2045: Content-Type, Content-Transfer-Encoding
// ---------------------------------------------------------------------------

// White space around a token.  RFC 2045 permits comments there too, and this
// reader does not read them.
rule<std::string_view> ows() { return p::take_while(is_wsp); }

template <p::parser_like P>
rule<p::value_of<P>> lex(P parser) {
    return p::sequence_left(p::sequence_right(ows(), std::move(parser)), ows());
}

//     token = 1*<any (US-ASCII) CHAR except SPACE, CTLs, or tspecials>
rule<std::string> token() { return p::map(p::take_while1(is_token_char, "expected a token"), to_str); }

//     quoted-string = DQUOTE *(qtext / quoted-pair) DQUOTE
//     quoted-pair   = "\" (VCHAR / WSP)
//
// qtext is every byte but the quote and the backslash that RFC 5322 allows;
// this reader takes the bytes the message has.
rule<std::string> quoted_string() {
    auto qtext       = p::satisfy([](char c) { return c != '"' && c != '\\'; }, "expected a quoted-string character");
    auto quoted_pair = p::sequence_right(p::char_p('\\'), p::satisfy(is_any, "expected a character"));
    auto inside      = p::many(qtext | quoted_pair);
    return p::sequence_right(
        p::char_p('"'),
        p::sequence_left(
            p::map(inside, [](const std::vector<char>& chars) { return std::string{chars.begin(), chars.end()}; }),
            p::char_p('"')));
}

//     parameter := attribute "=" value
//     attribute := token
//     value     := token / quoted-string
rule<media_parameter> parameter() {
    return p::lift(
        [](std::string name, char, std::string value) { return media_parameter{lower(name), std::move(value)}; },
        lex(token()),
        p::char_p('='),
        lex(token() | quoted_string()));
}

//     content := "Content-Type" ":" type "/" subtype *(";" parameter)
//     type    := token                       ; case-insensitive
//     subtype := token                       ; case-insensitive
rule<media_type> content_type() {
    return p::lift(
        [](std::string type, char, std::string subtype, std::vector<media_parameter> parameters) {
            return media_type{lower(type), lower(subtype), std::move(parameters)};
        },
        lex(token()),
        p::char_p('/'),
        lex(token()),
        p::many(p::sequence_right(p::char_p(';'), parameter())));
}

rule<transfer_encoding> mechanism_word(const char* spelling, transfer_encoding encoding) {
    return p::map(p::attempt(p::keyword_ci(spelling)), [encoding](std::string_view) { return encoding; });
}

//     mechanism := "7bit" / "8bit" / "binary" / "quoted-printable" / "base64" /
//                  ietf-token / x-token
//
// The last two are other people's mechanisms, which this reader does not
// understand, so they are not accepted.
rule<transfer_encoding> mechanism() {
    return mechanism_word("7bit", transfer_encoding::seven_bit) |
           mechanism_word("8bit", transfer_encoding::eight_bit) | mechanism_word("binary", transfer_encoding::binary) |
           mechanism_word("quoted-printable", transfer_encoding::quoted_printable) |
           mechanism_word("base64", transfer_encoding::base64);
}

// ---------------------------------------------------------------------------
// RFC 2047: encoded-words
// ---------------------------------------------------------------------------

//     encoded-word = "=?" charset "?" encoding "?" encoded-text "?="
//     charset      = token             ; the RFC 2047 token, which has no "."
//     encoding     = "Q" / "B"         ; case-insensitive
//     encoded-text = 1*<Any printable ASCII character other than "?" or SPACE>
rule<header_chunk> encoded_word() {
    auto encoding = p::satisfy([](char c) { return c == 'B' || c == 'b' || c == 'Q' || c == 'q'; }, "expected B or Q");
    return p::lift(
        [](std::string_view,
           std::string_view charset,
           char,
           char encoding,
           char,
           std::string_view text,
           std::string_view) {
            return header_chunk{header_chunk::kind::encoded_word,
                                std::string{text},
                                std::string{charset},
                                static_cast<char>(encoding == 'b' || encoding == 'B' ? 'B' : 'Q')};
        },
        p::keyword("=?"),
        p::take_while1(is_encoded_word_token_char, "expected a charset"),
        p::char_p('?'),
        encoding,
        p::char_p('?'),
        p::take_while1(is_encoded_text_char, "expected encoded-text"),
        p::keyword("?="));
}

// An encoded-word stands alone: white space, or the end, on either side.
//
//     delimiter = WSP / end of the text
rule<p::unit> word_end() { return p::followed_by(p::satisfy(is_wsp, "expected white space")) | p::eof(); }

//     word = encoded-word / <anything else up to white space>
rule<header_chunk> word() {
    auto encoded = p::attempt(p::sequence_left(encoded_word(), word_end()));
    auto plain   = p::map(p::take_while1(is_not_wsp, "expected a word"), [](std::string_view text) {
        return header_chunk{header_chunk::kind::text, std::string{text}, {}, 'Q'};
    });
    return encoded | plain;
}

//     unstructured = *(1*WSP / word)     ; the Subject: of RFC 5322, as RFC 2047 reads it
rule<std::vector<header_chunk>> unstructured() {
    auto spaces = p::map(p::take_while1(is_wsp, "expected white space"), [](std::string_view text) {
        return header_chunk{header_chunk::kind::space, std::string{text}, {}, 'Q'};
    });
    return p::many(spaces | word());
}

// ---------------------------------------------------------------------------
// RFC 2046: multipart
// ---------------------------------------------------------------------------

//     transport-padding = *LWSP-char                  ; LWSP-char = SP / HTAB
//     dash-boundary     = "--" boundary
//     close-delimiter   = delimiter "--"
//
// What follows a delimiter's boundary: true if "--" makes it the close
// delimiter, with its padding, false if it is padding and a line break.
rule<bool> after_boundary() {
    auto closing =
        p::map(p::sequence_left(p::keyword("--"), p::take_while(is_wsp)), [](std::string_view) { return true; });
    auto continuing = p::map(p::sequence_left(p::take_while(is_wsp), eol()), [](std::string_view) { return false; });
    return closing | continuing;
}

//     boundary-line = dash-boundary transport-padding CRLF
//                   / dash-boundary "--" transport-padding
rule<bool> boundary_line(const std::string& boundary) {
    return p::sequence_right(p::keyword("--" + boundary), after_boundary());
}

//     delimiter = CRLF dash-boundary
rule<bool> delimiter(const std::string& boundary) { return p::sequence_right(eol(), boundary_line(boundary)); }

//     multipart-body := [preamble CRLF] dash-boundary transport-padding CRLF
//                       body-part *encapsulation
//                       close-delimiter transport-padding
//                       [CRLF epilogue]
//     encapsulation  := delimiter transport-padding CRLF body-part
//
// A "--" boundary that merely starts with the boundary, as "--ab" does of the
// boundary "a", is not a delimiter: the bytes after it decide.
rule<std::vector<std::string_view>> multipart_body(std::string boundary) {
    auto first = p::attempt(boundary_line(boundary)) |
                 p::map(p::scan_until(delimiter(boundary), "expected the first boundary delimiter"),
                        [](p::scanned<bool> s) { return s.value; });
    auto next  = p::scan_until(delimiter(boundary), "expected a closing boundary delimiter");
    return p::parser{[first, next](p::cursor cur) -> p::parse_result<std::vector<std::string_view>> {
        std::vector<std::string_view> parts;
        auto                          start = first(cur);
        if (!start) {
            return std::unexpected(start.error());
        }
        bool closed = start->value;
        cur         = start->rest;
        while (!closed) {
            auto part = next(cur);
            if (!part) {
                return std::unexpected(part.error());
            }
            parts.push_back(part->value.skipped);
            closed = part->value.value;
            cur    = part->rest;
        }
        // The epilogue is whatever follows the close delimiter.
        return p::parse_state<std::vector<std::string_view>>{std::move(parts), cur.advance(cur.remaining().size())};
    }};
}

// Parses the whole of `input` with `parser`.
template <class P>
parsed<p::value_of<P>> run(P parser, std::string_view input) {
    auto result = p::sequence_left(std::move(parser), p::eof())(p::cursor{input});
    if (!result) {
        return std::unexpected(result.error());
    }
    return std::move(result->value);
}

} // namespace

std::optional<std::string> media_type::parameter(std::string_view name) const {
    for (const media_parameter& each : parameters) {
        if (each.name == name) {
            return each.value;
        }
    }
    return std::nullopt;
}

std::string_view to_string(transfer_encoding encoding) {
    switch (encoding) {
    case transfer_encoding::seven_bit:
        return "7bit";
    case transfer_encoding::eight_bit:
        return "8bit";
    case transfer_encoding::binary:
        return "binary";
    case transfer_encoding::base64:
        return "base64";
    case transfer_encoding::quoted_printable:
        return "quoted-printable";
    }
    return "";
}

std::string header_chunk::spelling() const {
    if (what != kind::encoded_word) {
        return text;
    }
    return "=?" + charset + "?" + std::string(1, encoding) + "?" + text + "?=";
}

parsed<entity_syntax> parse_entity_syntax(std::string_view bytes) { return run(body_part_syntax(), bytes); }

parsed<media_type> parse_content_type(std::string_view value) { return run(content_type(), value); }

parsed<transfer_encoding> parse_transfer_encoding(std::string_view value) { return run(lex(mechanism()), value); }

parsed<std::vector<header_chunk>> parse_unstructured(std::string_view value) { return run(unstructured(), value); }

parsed<std::vector<std::string_view>> parse_multipart_body(std::string_view body, std::string_view boundary) {
    return run(multipart_body(std::string{boundary}), body);
}

} // namespace mime_reader
