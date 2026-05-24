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
// whatwg_encode<codec::euc_kr>
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr encode ASCII 'A' -> 0x41", "[transcoding::euc_kr_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("euc_kr encode ASCII NUL -> 0x00", "[transcoding::euc_kr_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("euc_kr encode U+AC02 -> 0x81 0x41 (pointer 0)", "[transcoding::euc_kr_encode]") {
    // pointer 0: lead = 0/190 + 0x81 = 0x81, trail_raw = 0%190 + 0x41 = 0x41
    // 0x41 < 0x7F -> no skip, trail = 0x41
    std::vector<char32_t> cps{U'\xAC02'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_kr>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x81');
    CHECK(result[1] == '\x41');
}

TEST_CASE("euc_kr encode U+AC00 (가) -> 0xB0 0xA2 (pointer 9026)", "[transcoding::euc_kr_encode]") {
    // pointer 9026: lead = 9026/190 + 0x81 = 47 + 0x81 = 0xB0
    // trail_raw = 9026%190 + 0x41 = 96 + 0x41 = 0xA1, 0xA1 >= 0x7F -> trail = 0xA2
    std::vector<char32_t> cps{U'\xAC00'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_kr>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xB0');
    CHECK(result[1] == '\xA2');
}

TEST_CASE("euc_kr encode unmapped codepoint -> '?'", "[transcoding::euc_kr_encode]") {
    // U+1F600 (GRINNING FACE emoji) is not in EUC-KR
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("euc_kr encode mixed ASCII and Korean", "[transcoding::euc_kr_encode]") {
    // 'A' + U+AC00
    std::vector<char32_t> cps{U'A', U'\xAC00'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_kr>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(result[1] == '\xB0');
    CHECK(result[2] == '\xA2');
}

TEST_CASE("euc_kr encode pipe syntax", "[transcoding::euc_kr_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg_encode<codec::euc_kr>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("euc_kr encode consteval ASCII", "[transcoding::euc_kr_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg_encode<codec::euc_kr>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::euc_kr>
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr or_error encode ASCII 'A'", "[transcoding::euc_kr_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("euc_kr or_error encode U+AC00 (가) -> 0xB0 0xA2", "[transcoding::euc_kr_encode_or_error]") {
    std::vector<char32_t> cps{U'\xAC00'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == '\xB0');
    CHECK(result[1].value() == '\xA2');
}

TEST_CASE("euc_kr or_error encode unmapped codepoint yields error", "[transcoding::euc_kr_encode_or_error]") {
    // U+1F600 -> unmapped_codepoint error
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}
