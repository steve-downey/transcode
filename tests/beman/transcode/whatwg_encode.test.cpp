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
    for (char b : v)
        result.push_back(b);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_encode_view satisfies input_range", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  view = cps | whatwg_encode<codec::windows_1252>;
    static_assert(std::ranges::input_range<decltype(view)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(view)>, char>);
}

TEST_CASE("whatwg_encode windows_1252 ASCII passthrough", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'H', U'i'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'H', 'i'});
}

TEST_CASE("whatwg_encode windows_1252 euro sign", "[transcoding::whatwg_encode]") {
    // U+20AC (Euro sign) -> 0x80 in windows-1252
    std::vector<char32_t> cps{U'\x20AC'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'\x80'});
}

TEST_CASE("whatwg_encode windows_1252 unmapped codepoint yields question mark",
          "[transcoding::whatwg_encode]") {
    // U+4E2D (CJK) is not in windows-1252
    std::vector<char32_t> cps{U'\x4E2D'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'?'});
}

TEST_CASE("whatwg_encode iso_8859_2 A-ogonek", "[transcoding::whatwg_encode]") {
    // U+0104 (Latin capital A with ogonek) -> 0xA1 in iso-8859-2
    std::vector<char32_t> cps{U'\x0104'};
    CHECK(collect(cps | whatwg_encode<codec::iso_8859_2>) == std::vector<char>{'\xA1'});
}

TEST_CASE("whatwg_encode koi8_r Cyrillic A", "[transcoding::whatwg_encode]") {
    // U+0410 (Cyrillic capital A) -> 0xE1 in KOI8-R
    std::vector<char32_t> cps{U'\x0410'};
    CHECK(collect(cps | whatwg_encode<codec::koi8_r>) == std::vector<char>{'\xE1'});
}

TEST_CASE("whatwg_encode windows_1252 mixed ASCII and high", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'A', U'\x20AC', U'B'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) ==
          std::vector<char>{'A', '\x80', 'B'});
}

TEST_CASE("whatwg_encode windows_1252 consteval", "[transcoding::whatwg_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_euro = []() consteval {
        constexpr char32_t    cps[] = {U'\x20AC'};
        std::span<const char32_t> sp(cps, 1);
        return *(sp | whatwg_encode<codec::windows_1252>).begin();
    };
    CHECK(constify(encode_euro()) == '\x80');
}
