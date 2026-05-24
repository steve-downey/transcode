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
// whatwg_decode<codec::shift_jis>
// ---------------------------------------------------------------------------

TEST_CASE("shift_jis decode ASCII passthrough 'A'", "[transcoding::shift_jis]") {
    // ASCII bytes (0x00-0x7F) pass through unchanged
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("shift_jis decode ASCII passthrough NUL", "[transcoding::shift_jis]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("shift_jis decode half-width katakana 0xA1 -> U+FF61", "[transcoding::shift_jis]") {
    // 0xA1 -> U+FF61 HALFWIDTH IDEOGRAPHIC FULL STOP
    std::vector<char> bytes{'\xA1'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFF61');
}

TEST_CASE("shift_jis decode half-width katakana 0xDF -> U+FF9F", "[transcoding::shift_jis]") {
    // 0xDF -> U+FF61 + (0xDF - 0xA1) = U+FF9F HALFWIDTH KATAKANA VOICED ITERATION MARK
    std::vector<char> bytes{'\xDF'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFF9F');
}

TEST_CASE("shift_jis decode multibyte pointer 0: 0x81 0x40 -> U+3000", "[transcoding::shift_jis]") {
    // pointer 0: lead_offset=0 (0x81-0x81=0), trail_offset=0 (0x40-0x40=0)
    // pointer=0 -> U+3000 (IDEOGRAPHIC SPACE)
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x3000');
}

TEST_CASE("shift_jis decode multibyte 0x88 0xEA -> U+4E00", "[transcoding::shift_jis]") {
    // pointer 1485: lead_offset=7 (0x88-0x81=7), trail_offset=169 (0xEA-0x41=169)
    // 7*188+169 = 1316+169 = 1485 -> U+4E00 (一)
    std::vector<char> bytes{'\x88', '\xEA'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E00');
}

TEST_CASE("shift_jis decode invalid lead byte 0x80 -> U+FFFD", "[transcoding::shift_jis]") {
    // 0x80 is not ASCII, not katakana, not a valid lead byte -> U+FFFD
    std::vector<char> bytes{'\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("shift_jis decode invalid lead byte 0xA0 -> U+FFFD", "[transcoding::shift_jis]") {
    // 0xA0 is not a valid Shift_JIS byte -> U+FFFD
    std::vector<char> bytes{'\xA0'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("shift_jis decode invalid trail byte 0x81 0x3F -> U+FFFD", "[transcoding::shift_jis]") {
    // Trail byte 0x3F < 0x40: invalid trail byte -> U+FFFD
    std::vector<char> bytes{'\x81', '\x3F'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("shift_jis decode invalid trail byte 0x81 0x7F -> U+FFFD", "[transcoding::shift_jis]") {
    // Trail byte 0x7F is excluded -> U+FFFD
    std::vector<char> bytes{'\x81', '\x7F'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("shift_jis decode truncated lead byte 0x81 -> U+FFFD", "[transcoding::shift_jis]") {
    // Lead byte with no trail byte -> U+FFFD
    std::vector<char> bytes{'\x81'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("shift_jis decode mixed ASCII and multibyte", "[transcoding::shift_jis]") {
    // 'A' then 0x81 0x40 (U+3000)
    std::vector<char> bytes{'\x41', '\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::shift_jis>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x3000');
}

TEST_CASE("shift_jis decode pipe syntax with span", "[transcoding::shift_jis]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::shift_jis>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("shift_jis decode consteval ASCII", "[transcoding::shift_jis]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::shift_jis>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::shift_jis>
// ---------------------------------------------------------------------------

TEST_CASE("shift_jis or_error: ASCII passthrough", "[transcoding::shift_jis_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("shift_jis or_error: half-width katakana 0xA1 -> U+FF61", "[transcoding::shift_jis_or_error]") {
    std::vector<char> bytes{'\xA1'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\xFF61');
}

TEST_CASE("shift_jis or_error: valid multibyte 0x81 0x40 -> U+3000", "[transcoding::shift_jis_or_error]") {
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x3000');
}

TEST_CASE("shift_jis or_error: invalid lead byte 0x80 yields error", "[transcoding::shift_jis_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("shift_jis or_error: truncated sequence yields error", "[transcoding::shift_jis_or_error]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("shift_jis or_error: invalid trail byte yields error", "[transcoding::shift_jis_or_error]") {
    std::vector<char> bytes{'\x81', '\x7F'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}
