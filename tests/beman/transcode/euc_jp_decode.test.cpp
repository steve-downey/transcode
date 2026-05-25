// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char32_t> collect(View&& v) {
    std::vector<char32_t> result;
    for (char32_t cp : v)
        result.push_back(cp);
    return result;
}

template <typename View>
std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg_decode<codec::euc_jp>
// ---------------------------------------------------------------------------

TEST_CASE("euc_jp decode ASCII passthrough 'A' (0x41)", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("euc_jp decode ASCII NUL (0x00)", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("euc_jp decode JIS X 0208 0xA1 0xA1 -> U+3000 (pointer 0)", "[transcoding::euc_jp]") {
    // pointer = (0xA1 - 0xA1) * 94 + (0xA1 - 0xA1) = 0 -> U+3000 (IDEOGRAPHIC SPACE)
    std::vector<char> bytes{'\xA1', '\xA1'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x3000');
}

TEST_CASE("euc_jp decode JIS X 0208 0xB0 0xEC -> U+4E00 (一)", "[transcoding::euc_jp]") {
    // pointer = (0xB0 - 0xA1) * 94 + (0xEC - 0xA1) = 15*94 + 75 = 1410 + 75 = 1485
    // tables::shift_jis[1485] = U+4E00
    std::vector<char> bytes{'\xB0', '\xEC'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E00');
}

TEST_CASE("euc_jp decode half-width katakana 0x8E 0xA1 -> U+FF61", "[transcoding::euc_jp]") {
    // SS2 prefix (0x8E) + 0xA1 -> U+FF61 HALFWIDTH IDEOGRAPHIC FULL STOP
    std::vector<char> bytes{'\x8E', '\xA1'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFF61');
}

TEST_CASE("euc_jp decode half-width katakana 0x8E 0xDF -> U+FF9F", "[transcoding::euc_jp]") {
    // SS2 prefix (0x8E) + 0xDF -> U+FF61 + (0xDF - 0xA1) = U+FF9F
    std::vector<char> bytes{'\x8E', '\xDF'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFF9F');
}

TEST_CASE("euc_jp decode JIS X 0212 0x8F 0xA2 0xAF -> U+02D8 (BREVE)", "[transcoding::euc_jp]") {
    // SS3 prefix (0x8F) + b1=0xA2 + b2=0xAF
    // pointer = (0xA2 - 0xA1) * 94 + (0xAF - 0xA1) = 1*94 + 14 = 108
    // euc_jp_jis0212[108] = U+02D8 (BREVE)
    std::vector<char> bytes{'\x8F', '\xA2', '\xAF'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x02D8');
}

TEST_CASE("euc_jp decode invalid lead byte 0x80 -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode invalid lead byte 0xFF -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\xFF'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode bad trail in JIS X 0208: 0xA1 0xA0 -> U+FFFD", "[transcoding::euc_jp]") {
    // 0xA0 < 0xA1: invalid trail byte
    std::vector<char> bytes{'\xA1', '\xA0'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode truncated JIS X 0208: 0xA1 only -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\xA1'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode truncated SS2: 0x8E only -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x8E'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode SS2 with bad trail 0x8E 0xA0 -> U+FFFD", "[transcoding::euc_jp]") {
    // 0xA0 < 0xA1: invalid for half-width katakana
    std::vector<char> bytes{'\x8E', '\xA0'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode truncated SS3: 0x8F 0xA2 -> U+FFFD", "[transcoding::euc_jp]") {
    // SS3 needs 2 trail bytes; only 1 given
    std::vector<char> bytes{'\x8F', '\xA2'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode mixed: ASCII + JIS X 0208", "[transcoding::euc_jp]") {
    // 'A' (0x41) then 0xA1 0xA1 (U+3000)
    std::vector<char> bytes{'\x41', '\xA1', '\xA1'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x3000');
}

TEST_CASE("euc_jp decode pipe syntax with span", "[transcoding::euc_jp]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::euc_jp>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("euc_jp decode consteval ASCII", "[transcoding::euc_jp]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::euc_jp>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::euc_jp>
// ---------------------------------------------------------------------------

TEST_CASE("euc_jp or_error: ASCII passthrough", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("euc_jp or_error: JIS X 0208 0xA1 0xA1 -> U+3000", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\xA1', '\xA1'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x3000');
}

TEST_CASE("euc_jp or_error: half-width katakana 0x8E 0xA1 -> U+FF61", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\x8E', '\xA1'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\xFF61');
}

TEST_CASE("euc_jp or_error: JIS X 0212 0x8F 0xA2 0xAF -> U+02D8", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\x8F', '\xA2', '\xAF'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x02D8');
}

TEST_CASE("euc_jp or_error: invalid lead byte yields error", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("euc_jp or_error: truncated JIS X 0208 yields error", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\xA1'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("euc_jp or_error: bad trail in JIS X 0208 yields error", "[transcoding::euc_jp_or_error]") {
    std::vector<char> bytes{'\xA1', '\xA0'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// ---------------------------------------------------------------------------
// Coverage: SS3 (JIS X 0212) error paths
// ---------------------------------------------------------------------------

TEST_CASE("euc_jp decode truncated SS3: 0x8F only -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x8F'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode SS3 invalid first trail: 0x8F 0x00 -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x8F', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() >= 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode SS3 invalid second trail: 0x8F 0xA1 0x00 -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x8F', '\xA1', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() >= 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode SS3 unmapped JIS X 0212 pointer 0: 0x8F 0xA1 0xA1 -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\x8F', '\xA1', '\xA1'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_jp decode unmapped JIS X 0208 pointer 108: 0xA2 0xAF -> U+FFFD", "[transcoding::euc_jp]") {
    std::vector<char> bytes{'\xA2', '\xAF'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}
