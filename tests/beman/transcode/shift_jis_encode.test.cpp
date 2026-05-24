// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <array>
#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char> collect(View&& v) {
    std::vector<char> result;
    for (char c : v)
        result.push_back(c);
    return result;
}

template <typename View>
std::vector<std::expected<char, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg_encode<codec::shift_jis>
// ---------------------------------------------------------------------------

TEST_CASE("shift_jis encode ASCII 'A' -> 0x41", "[transcoding::shift_jis_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("shift_jis encode ASCII NUL -> 0x00", "[transcoding::shift_jis_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("shift_jis encode U+00A5 (YEN SIGN) -> 0x5C", "[transcoding::shift_jis_encode]") {
    // WHATWG special case: U+00A5 maps to 0x5C
    std::vector<char32_t> cps{U'\x00A5'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x5C');
}

TEST_CASE("shift_jis encode U+203E (OVERLINE) -> 0x7E", "[transcoding::shift_jis_encode]") {
    // WHATWG special case: U+203E maps to 0x7E
    std::vector<char32_t> cps{U'\x203E'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x7E');
}

TEST_CASE("shift_jis encode U+FF61 (half-width katakana) -> 0xA1", "[transcoding::shift_jis_encode]") {
    // U+FF61 HALFWIDTH IDEOGRAPHIC FULL STOP -> 0xA1
    std::vector<char32_t> cps{U'\xFF61'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\xA1');
}

TEST_CASE("shift_jis encode U+FF9F (half-width katakana) -> 0xDF", "[transcoding::shift_jis_encode]") {
    // U+FF9F HALFWIDTH KATAKANA VOICED ITERATION MARK -> 0xA1 + (0xFF9F - 0xFF61) = 0xDF
    std::vector<char32_t> cps{U'\xFF9F'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\xDF');
}

TEST_CASE("shift_jis encode U+3000 -> 0x81 0x40", "[transcoding::shift_jis_encode]") {
    // pointer 0 -> lead=0x81, trail=0x40
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x81');
    CHECK(result[1] == '\x40');
}

TEST_CASE("shift_jis encode U+4E00 (一) -> 0x88 0xEA", "[transcoding::shift_jis_encode]") {
    // pointer 1485 -> lead_offset=7 (0x88), trail_offset=169 (0xEA)
    std::vector<char32_t> cps{U'\x4E00'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x88');
    CHECK(result[1] == '\xEA');
}

TEST_CASE("shift_jis encode unmapped codepoint -> '?'", "[transcoding::shift_jis_encode]") {
    // U+1F600 (GRINNING FACE) is not in the Shift_JIS table -> '?'
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("shift_jis encode mixed ASCII and multibyte", "[transcoding::shift_jis_encode]") {
    // 'A' + U+3000
    std::vector<char32_t> cps{U'A', U'\x3000'};
    auto                  result = collect(cps | whatwg_encode<codec::shift_jis>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(result[1] == '\x81');
    CHECK(result[2] == '\x40');
}

TEST_CASE("shift_jis encode pipe syntax", "[transcoding::shift_jis_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg_encode<codec::shift_jis>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("shift_jis encode consteval ASCII", "[transcoding::shift_jis_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg_encode<codec::shift_jis>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::shift_jis>
// ---------------------------------------------------------------------------

TEST_CASE("shift_jis or_error encode ASCII 'A'", "[transcoding::shift_jis_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("shift_jis or_error encode U+3000", "[transcoding::shift_jis_encode_or_error]") {
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == '\x81');
    CHECK(result[1].value() == '\x40');
}

TEST_CASE("shift_jis or_error encode unmapped codepoint yields error", "[transcoding::shift_jis_encode_or_error]") {
    // U+1F600 -> unmapped_codepoint error
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::shift_jis>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}
