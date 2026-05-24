// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <vector>

namespace {
// Helper: collect all code points from a whatwg_decode view into a vector.
template <typename View>
std::vector<char32_t> collect(View&& v) {
    std::vector<char32_t> result;
    for (char32_t cp : v)
        result.push_back(cp);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_decode_view satisfies input_range", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    auto              view = bytes | whatwg_decode<codec::utf_8>;
    static_assert(std::ranges::input_range<decltype(view)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(view)>, char32_t>);
}

// All WHATWG codecs share the 7-bit ASCII base (U+0000–U+007F).
TEST_CASE("whatwg_decode 7-bit ASCII shared base", "[transcoding::whatwg_decode]") {
    std::vector<char> ascii{'H', 'e', 'l', 'l', 'o'};
    CHECK(collect(ascii | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
}

// Step 5: valid multi-byte UTF-8 sequences

TEST_CASE("whatwg_decode 2-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+00E9 é = 0xC3 0xA9
    std::vector<char> bytes{'\xC3', '\xA9'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'é'});
}

TEST_CASE("whatwg_decode 3-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+20AC € = 0xE2 0x82 0xAC
    std::vector<char> bytes{'\xE2', '\x82', '\xAC'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'€'});
}

TEST_CASE("whatwg_decode 4-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+1F600 😀 = 0xF0 0x9F 0x98 0x80
    std::vector<char> bytes{'\xF0', '\x9F', '\x98', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'\U0001F600'});
}

TEST_CASE("whatwg_decode mixed ASCII and multi-byte", "[transcoding::whatwg_decode]") {
    // "Hi" + é (0xC3 0xA9) + "!"
    std::vector<char> bytes{'H', 'i', '\xC3', '\xA9', '!'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'H', U'i', U'é', U'!'});
}

// Step 6: WHATWG-specific error replacement behavior

TEST_CASE("whatwg_decode invalid lead byte 0xFF", "[transcoding::whatwg_decode]") {
    // 0xFF is never valid in UTF-8; followed by ASCII 'A' which is re-processed.
    std::vector<char> bytes{'\xFF', 'A'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�', U'A'});
}

TEST_CASE("whatwg_decode 0xC0 pre-rejected continuation re-processed", "[transcoding::whatwg_decode]") {
    // WHATWG pre-rejects 0xC0 immediately (→ U+FFFD); 0x80 is then an
    // unexpected continuation byte → second U+FFFD.
    std::vector<char> bytes{'\xC0', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�', U'�'});
}

TEST_CASE("whatwg_decode bad continuation causes re-processing", "[transcoding::whatwg_decode]") {
    // 0xC3 starts a 2-byte sequence; '(' (0x28) is not a continuation byte.
    // Result: U+FFFD for the incomplete sequence, then '(' re-processed as ASCII.
    std::vector<char> bytes{'\xC3', '\x28'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�', U'('});
}

TEST_CASE("whatwg_decode surrogate codepoint rejected", "[transcoding::whatwg_decode]") {
    // U+D800 encoded as UTF-8: 0xED 0xA0 0x80 — continuation bytes are valid,
    // but the assembled codepoint is a surrogate.
    std::vector<char> bytes{'\xED', '\xA0', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�'});
}

TEST_CASE("whatwg_decode truncated sequence at end of input", "[transcoding::whatwg_decode]") {
    // 0xC3 is a 2-byte lead; no continuation follows → one U+FFFD.
    std::vector<char> bytes{'\xC3'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�'});
}

TEST_CASE("whatwg_decode consteval 2-byte", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_two_byte = []() consteval {
        constexpr char        bytes[] = {'\xC3', '\xA9'};
        std::span<const char> sp(bytes, 2);
        return *(sp | whatwg_decode<codec::utf_8>).begin();
    };
    CHECK(constify(decode_two_byte()) == U'é');
}

// Step 14: codec::replacement tests

TEST_CASE("whatwg_decode replacement empty input", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{};
    CHECK(collect(bytes | whatwg_decode<codec::replacement>).empty());
}

TEST_CASE("whatwg_decode replacement single byte", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'X'};
    CHECK(collect(bytes | whatwg_decode<codec::replacement>) == std::vector<char32_t>{U'\xFFFD'});
}

TEST_CASE("whatwg_decode replacement multiple bytes yields one U+FFFD", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'H', 'e', 'l', 'l', 'o'};
    auto              result = collect(bytes | whatwg_decode<codec::replacement>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("whatwg_decode replacement consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_replacement = []() consteval {
        constexpr char        bytes[] = {'a', 'b', 'c'};
        std::span<const char> sp(bytes, 3);
        return *(sp | whatwg_decode<codec::replacement>).begin();
    };
    CHECK(constify(decode_replacement()) == U'\xFFFD');
}

// Step 15: codec::x_user_defined tests

TEST_CASE("whatwg_decode x_user_defined ASCII", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A', 'z'};
    CHECK(collect(bytes | whatwg_decode<codec::x_user_defined>) == std::vector<char32_t>{U'A', U'z'});
}

TEST_CASE("whatwg_decode x_user_defined high bytes", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'\x80', '\xFF'};
    auto              result = collect(bytes | whatwg_decode<codec::x_user_defined>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == char32_t(0xF780));
    CHECK(result[1] == char32_t(0xF7FF));
}

TEST_CASE("whatwg_decode x_user_defined consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_x = []() consteval {
        constexpr char        bytes[] = {'\x80'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::x_user_defined>).begin();
    };
    CHECK(constify(decode_x()) == char32_t(0xF780));
}

// Step 16: codec::windows_1252 tests

TEST_CASE("whatwg_decode windows_1252 ASCII", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'A'});
}

TEST_CASE("whatwg_decode windows_1252 euro sign", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+20AC (Euro sign) — the iconic windows-1252 mapping
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode windows_1252 smart quotes", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'\x93', '\x94'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'\x201C', U'\x201D'});
}

TEST_CASE("whatwg_decode windows_1252 c1 control byte", "[transcoding::whatwg_decode]") {
    // WHATWG maps 0x81 to U+0081 (C1 control); hand-written table had it as error.
    std::vector<char> bytes{'\x81'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'\x0081'});
}

TEST_CASE("whatwg_decode windows_1252 consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_euro = []() consteval {
        constexpr char        bytes[] = {'\x80'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::windows_1252>).begin();
    };
    CHECK(constify(decode_euro()) == char32_t(0x20AC));
}

// Step 18: representative tests for the 27 new single-byte codecs

TEST_CASE("whatwg_decode ibm866 ASCII", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    CHECK(collect(bytes | whatwg_decode<codec::ibm866>) == std::vector<char32_t>{U'A'});
}

TEST_CASE("whatwg_decode ibm866 box drawing", "[transcoding::whatwg_decode]") {
    // 0xB3 -> U+2502 (BOX DRAWINGS LIGHT VERTICAL)
    std::vector<char> bytes{'\xB3'};
    CHECK(collect(bytes | whatwg_decode<codec::ibm866>) == std::vector<char32_t>{U'\x2502'});
}

TEST_CASE("whatwg_decode iso_8859_2 ogonek A", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0104 (LATIN CAPITAL LETTER A WITH OGONEK)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_2>) == std::vector<char32_t>{U'\x0104'});
}

TEST_CASE("whatwg_decode koi8_r cyrillic a", "[transcoding::whatwg_decode]") {
    // 0xC1 -> U+0430 (CYRILLIC SMALL LETTER A)
    std::vector<char> bytes{'\xC1'};
    CHECK(collect(bytes | whatwg_decode<codec::koi8_r>) == std::vector<char32_t>{U'\x0430'});
}

TEST_CASE("whatwg_decode windows_1250 ogonek A", "[transcoding::whatwg_decode]") {
    // 0xA5 -> U+0104 (LATIN CAPITAL LETTER A WITH OGONEK)
    std::vector<char> bytes{'\xA5'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1250>) == std::vector<char32_t>{U'\x0104'});
}

TEST_CASE("whatwg_decode iso_8859_8_i shares iso_8859_8 table", "[transcoding::whatwg_decode]") {
    // 0xE0 -> U+05D0 (HEBREW LETTER ALEF) in both iso-8859-8 and iso-8859-8-i
    std::vector<char> bytes{'\xE0'};
    auto              r8  = collect(bytes | whatwg_decode<codec::iso_8859_8>);
    auto              r8i = collect(bytes | whatwg_decode<codec::iso_8859_8_i>);
    CHECK(r8 == r8i);
}

// Step 18: smoke tests for remaining 21 single-byte codecs (one distinctive
// high-byte mapping per codec to exercise the enum value and table lookup).

TEST_CASE("whatwg_decode iso_8859_3 H-stroke", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0126 (LATIN CAPITAL LETTER H WITH STROKE)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_3>) == std::vector<char32_t>{U'\x0126'});
}

TEST_CASE("whatwg_decode iso_8859_4 ogonek A", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0104 (LATIN CAPITAL LETTER A WITH OGONEK)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_4>) == std::vector<char32_t>{U'\x0104'});
}

TEST_CASE("whatwg_decode iso_8859_5 cyrillic IO", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0401 (CYRILLIC CAPITAL LETTER IO)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_5>) == std::vector<char32_t>{U'\x0401'});
}

TEST_CASE("whatwg_decode iso_8859_6 arabic hamza", "[transcoding::whatwg_decode]") {
    // 0xC1 -> U+0621 (ARABIC LETTER HAMZA)
    std::vector<char> bytes{'\xC1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_6>) == std::vector<char32_t>{U'\x0621'});
}

TEST_CASE("whatwg_decode iso_8859_7 left single quote", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+2018 (LEFT SINGLE QUOTATION MARK)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_7>) == std::vector<char32_t>{U'\x2018'});
}

TEST_CASE("whatwg_decode iso_8859_10 ogonek A", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0104 (LATIN CAPITAL LETTER A WITH OGONEK)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_10>) == std::vector<char32_t>{U'\x0104'});
}

TEST_CASE("whatwg_decode iso_8859_13 right double quote", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+201D (RIGHT DOUBLE QUOTATION MARK)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_13>) == std::vector<char32_t>{U'\x201D'});
}

TEST_CASE("whatwg_decode iso_8859_14 B-dot-above", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+1E02 (LATIN CAPITAL LETTER B WITH DOT ABOVE) — Welsh
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_14>) == std::vector<char32_t>{U'\x1E02'});
}

TEST_CASE("whatwg_decode iso_8859_15 euro sign", "[transcoding::whatwg_decode]") {
    // 0xA4 -> U+20AC (EURO SIGN) — differs from Latin-1's CURRENCY SIGN
    std::vector<char> bytes{'\xA4'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_15>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode iso_8859_16 euro sign", "[transcoding::whatwg_decode]") {
    // 0xA4 -> U+20AC (EURO SIGN)
    std::vector<char> bytes{'\xA4'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_16>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode koi8_u ukrainian IE", "[transcoding::whatwg_decode]") {
    // 0xA4 -> U+0454 (CYRILLIC SMALL LETTER UKRAINIAN IE) — differs from koi8_r
    std::vector<char> bytes{'\xA4'};
    CHECK(collect(bytes | whatwg_decode<codec::koi8_u>) == std::vector<char32_t>{U'\x0454'});
}

TEST_CASE("whatwg_decode macintosh A-diaeresis", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+00C4 (LATIN CAPITAL LETTER A WITH DIAERESIS)
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::macintosh>) == std::vector<char32_t>{U'\x00C4'});
}

TEST_CASE("whatwg_decode windows_874 thai ko kai", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0E01 (THAI CHARACTER KO KAI)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_874>) == std::vector<char32_t>{U'\x0E01'});
}

TEST_CASE("whatwg_decode windows_1251 cyrillic DJE", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+0402 (CYRILLIC CAPITAL LETTER DJE)
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1251>) == std::vector<char32_t>{U'\x0402'});
}

TEST_CASE("whatwg_decode windows_1253 greek dialytika tonos", "[transcoding::whatwg_decode]") {
    // 0xA1 -> U+0385 (GREEK DIALYTIKA TONOS)
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1253>) == std::vector<char32_t>{U'\x0385'});
}

TEST_CASE("whatwg_decode windows_1254 euro sign", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+20AC (EURO SIGN)
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1254>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode windows_1255 new sheqel", "[transcoding::whatwg_decode]") {
    // 0xA4 -> U+20AA (NEW SHEQEL SIGN)
    std::vector<char> bytes{'\xA4'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1255>) == std::vector<char32_t>{U'\x20AA'});
}

TEST_CASE("whatwg_decode windows_1256 arabic peh", "[transcoding::whatwg_decode]") {
    // 0x81 -> U+067E (ARABIC LETTER PEH)
    std::vector<char> bytes{'\x81'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1256>) == std::vector<char32_t>{U'\x067E'});
}

TEST_CASE("whatwg_decode windows_1257 euro sign", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+20AC (EURO SIGN)
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1257>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode windows_1258 euro sign", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+20AC (EURO SIGN)
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1258>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode x_mac_cyrillic cyrillic A", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+0410 (CYRILLIC CAPITAL LETTER A)
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::x_mac_cyrillic>) == std::vector<char32_t>{U'\x0410'});
}

// Coverage gap: single_byte_decode_one() error→replacement path.
// iso-8859-6 byte 0xA1 is genuinely unmapped (null entry in WHATWG table).
TEST_CASE("whatwg_decode iso_8859_6 unmapped byte yields U+FFFD", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'\xA1'};
    CHECK(collect(bytes | whatwg_decode<codec::iso_8859_6>) == std::vector<char32_t>{U'\xFFFD'});
}
