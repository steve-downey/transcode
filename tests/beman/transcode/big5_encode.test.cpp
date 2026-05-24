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
// whatwg_encode<codec::big5>
// ---------------------------------------------------------------------------

TEST_CASE("big5 encode ASCII 'A' -> 0x41", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("big5 encode ASCII NUL -> 0x00", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("big5 encode U+4E00 -> 0xA4 0x40 (pointer 5495)", "[transcoding::big5_encode]") {
    // lowest pointer for U+4E00 is 5495: lead=0xA4, trail=0x40
    std::vector<char32_t> cps{U'\x4E00'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xA4);
    CHECK(static_cast<unsigned char>(result[1]) == 0x40);
}

TEST_CASE("big5 encode U+00CA -> 0x88 0x66 (pointer 1137)", "[transcoding::big5_encode]") {
    // U+00CA is in the table at pointer 1137: lead=0x88, trail=0x66
    std::vector<char32_t> cps{U'\x00CA'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0x88);
    CHECK(static_cast<unsigned char>(result[1]) == 0x66);
}

TEST_CASE("big5 encode U+00EA -> 0x88 0xA7 (pointer 1168)", "[transcoding::big5_encode]") {
    // U+00EA is in the table at pointer 1168: lead=0x88, trail=0xA7
    std::vector<char32_t> cps{U'\x00EA'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0x88);
    CHECK(static_cast<unsigned char>(result[1]) == 0xA7);
}

TEST_CASE("big5 encode unmapped codepoint U+0041+extra -> '?' (non-error)", "[transcoding::big5_encode]") {
    // U+0304 (combining macron) is not individually in the Big5 table
    std::vector<char32_t> cps{U'\x0304'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("big5 encode mixed ASCII and 2-byte", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'A', U'\x4E00'};
    auto                  result = collect(cps | whatwg_encode<codec::big5>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0xA4);
    CHECK(static_cast<unsigned char>(result[2]) == 0x40);
}

TEST_CASE("big5 encode pipe syntax", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg_encode<codec::big5>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("big5 encode consteval ASCII", "[transcoding::big5_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg_encode<codec::big5>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::big5>
// ---------------------------------------------------------------------------

TEST_CASE("big5 or_error encode ASCII 'A'", "[transcoding::big5_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("big5 or_error encode U+4E00", "[transcoding::big5_encode_or_error]") {
    std::vector<char32_t> cps{U'\x4E00'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::big5>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0xA4);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0x40);
}

TEST_CASE("big5 or_error encode unmapped -> unmapped_codepoint error", "[transcoding::big5_encode_or_error]") {
    // U+0304 (combining macron) is not in Big5 table
    std::vector<char32_t> cps{U'\x0304'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}
