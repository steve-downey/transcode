// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <expected>
#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<std::expected<char, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_encode_or_error satisfies input_range", "[transcoding::whatwg_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  view = cps | whatwg_encode_or_error<codec::windows_1252>;
    static_assert(std::ranges::input_range<decltype(view)>);
    using val_t = std::ranges::range_value_t<decltype(view)>;
    static_assert(std::same_as<val_t, std::expected<char, whatwg_error>>);
}

TEST_CASE("whatwg_encode_or_error windows_1252 valid ASCII", "[transcoding::whatwg_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == 'A');
}

TEST_CASE("whatwg_encode_or_error windows_1252 valid high byte", "[transcoding::whatwg_encode_or_error]") {
    // U+20AC (Euro) -> 0x80
    std::vector<char32_t> cps{U'\x20AC'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x80');
}

TEST_CASE("whatwg_encode_or_error windows_1252 unmapped codepoint", "[transcoding::whatwg_encode_or_error]") {
    // U+4E2D (CJK) is not in windows-1252
    std::vector<char32_t> cps{U'\x4E2D'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}

TEST_CASE("whatwg_encode_or_error iso_8859_6 unmapped codepoint", "[transcoding::whatwg_encode_or_error]") {
    // U+00A3 (pound sign) is not in iso-8859-6 (Arabic)
    std::vector<char32_t> cps{U'\x00A3'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::iso_8859_6>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}

TEST_CASE("whatwg_encode_or_error consteval", "[transcoding::whatwg_encode_or_error]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_euro = []() consteval {
        constexpr char32_t        cps[] = {U'\x20AC'};
        std::span<const char32_t> sp(cps, 1);
        return (*(sp | whatwg_encode_or_error<codec::windows_1252>).begin()).value();
    };
    CHECK(constify(encode_euro()) == '\x80');
}

// ---------------------------------------------------------------------------
// UTF-8 or_error tests (step 20)
// ---------------------------------------------------------------------------

TEST_CASE("whatwg_encode_or_error UTF-8 surrogate yields unexpected", "[transcoding::whatwg_encode_or_error][utf8]") {
    std::vector<char32_t> cps{static_cast<char32_t>(0xD800)};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
}

TEST_CASE("whatwg_encode_or_error UTF-8 out_of_range yields unexpected",
          "[transcoding::whatwg_encode_or_error][utf8]") {
    std::vector<char32_t> cps{static_cast<char32_t>(0x110000)};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::out_of_range);
}

TEST_CASE("whatwg_encode_or_error UTF-8 valid ASCII has_value", "[transcoding::whatwg_encode_or_error][utf8]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == 'A');
}

TEST_CASE("whatwg_encode_or_error UTF-8 3-byte has_value correct bytes",
          "[transcoding::whatwg_encode_or_error][utf8]") {
    // U+20AC -> {0xE2, 0x82, 0xAC}
    std::vector<char32_t> cps{U'\x20AC'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 3);
    for (auto& r : result)
        REQUIRE(r.has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0xE2);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0x82);
    CHECK(static_cast<unsigned char>(result[2].value()) == 0xAC);
}
