// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

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
// whatwg_encode<codec::gbk>
// ---------------------------------------------------------------------------

TEST_CASE("gbk encode ASCII 'A' -> 0x41", "[transcoding::gbk_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("gbk encode ASCII NUL -> 0x00", "[transcoding::gbk_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg_encode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("gbk encode U+4E02 -> 0x81 0x40", "[transcoding::gbk_encode]") {
    // U+4E02 is at index 0 -> lead=0x81, trail=0x40
    std::vector<char32_t> cps{U'\x4E02'};
    auto                  result = collect(cps | whatwg_encode<codec::gbk>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x81');
    CHECK(result[1] == '\x40');
}

TEST_CASE("gbk encode U+4E04 -> 0x81 0x41", "[transcoding::gbk_encode]") {
    // U+4E04 is at index 1 -> lead=0x81, trail=0x41
    std::vector<char32_t> cps{U'\x4E04'};
    auto                  result = collect(cps | whatwg_encode<codec::gbk>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x81');
    CHECK(result[1] == '\x41');
}

TEST_CASE("gbk encode unmapped codepoint -> '?'", "[transcoding::gbk_encode]") {
    // U+1F600 is not in the GBK table -> '?'
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("gbk encode mixed ASCII and multibyte", "[transcoding::gbk_encode]") {
    // 'A' + U+4E02
    std::vector<char32_t> cps{U'A', U'\x4E02'};
    auto                  result = collect(cps | whatwg_encode<codec::gbk>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(result[1] == '\x81');
    CHECK(result[2] == '\x40');
}

TEST_CASE("gbk encode pipe syntax", "[transcoding::gbk_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg_encode<codec::gbk>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("gbk encode consteval ASCII", "[transcoding::gbk_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg_encode<codec::gbk>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::gbk>
// ---------------------------------------------------------------------------

TEST_CASE("gbk or_error encode ASCII 'A'", "[transcoding::gbk_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::gbk>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("gbk or_error encode U+4E02", "[transcoding::gbk_encode_or_error]") {
    std::vector<char32_t> cps{U'\x4E02'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::gbk>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == '\x81');
    CHECK(result[1].value() == '\x40');
}

TEST_CASE("gbk or_error encode unmapped codepoint yields error", "[transcoding::gbk_encode_or_error]") {
    // U+1F600 -> unmapped_codepoint error
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}
