// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/detail/tables/euc_kr.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
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
std::vector<std::expected<char, beman::transcoding::whatwg::error>> collect_or_error(View&& v) {
    std::vector<std::expected<char, beman::transcoding::whatwg::error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg::encode<whatwg::encoding::euc_kr>
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr encode ASCII 'A' -> 0x41", "[transcoding::euc_kr_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("euc_kr encode ASCII NUL -> 0x00", "[transcoding::euc_kr_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("euc_kr encode U+AC02 -> 0x81 0x41 (pointer 0)", "[transcoding::euc_kr_encode]") {
    // pointer 0: lead = 0/190 + 0x81 = 0x81, trail_raw = 0%190 + 0x41 = 0x41
    // 0x41 < 0x7F -> no skip, trail = 0x41
    std::vector<char32_t> cps{U'\xAC02'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x81');
    CHECK(result[1] == '\x41');
}

TEST_CASE("euc_kr encode U+AC00 (가) -> 0xB0 0xA1 (pointer 9026)", "[transcoding::euc_kr_encode]") {
    // pointer 9026: lead = 9026/190 + 0x81 = 47 + 0x81 = 0xB0
    // trail_raw = 9026%190 + 0x41 = 96 + 0x41 = 0xA1, 0xA1 >= 0x7F -> trail = 0xA2
    std::vector<char32_t> cps{U'\xAC00'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xB0');
    CHECK(result[1] == '\xA1');
}

TEST_CASE("euc_kr encode unmapped codepoint -> '?'", "[transcoding::euc_kr_encode]") {
    // U+1F600 (GRINNING FACE emoji) is not in EUC-KR
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("euc_kr encode mixed ASCII and Korean", "[transcoding::euc_kr_encode]") {
    // 'A' + U+AC00
    std::vector<char32_t> cps{U'A', U'\xAC00'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(result[1] == '\xB0');
    CHECK(result[2] == '\xA1');
}

TEST_CASE("euc_kr encode pipe syntax", "[transcoding::euc_kr_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg::encode<whatwg::encoding::euc_kr>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("euc_kr encode consteval ASCII", "[transcoding::euc_kr_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg::encode<whatwg::encoding::euc_kr>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg::encode_or_error<whatwg::encoding::euc_kr>
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr or_error encode ASCII 'A'", "[transcoding::euc_kr_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("euc_kr or_error encode U+AC00 (가) -> 0xB0 0xA1", "[transcoding::euc_kr_encode_or_error]") {
    std::vector<char32_t> cps{U'\xAC00'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == '\xB0');
    CHECK(result[1].value() == '\xA1');
}

TEST_CASE("euc_kr or_error encode unmapped codepoint yields error", "[transcoding::euc_kr_encode_or_error]") {
    // U+1F600 -> unmapped_codepoint error
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg::error::unmapped_codepoint);
}

// ---------------------------------------------------------------------------
// WHATWG byte layout and the index lookup
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr encode trail bytes run 0x41-0xFE without a gap", "[transcoding::euc_kr_encode]") {
    // Pointer 189 is the last trail of lead 0x81: trail = 189 + 0x41 = 0xFE.
    std::vector<char32_t> cps{detail::tables::euc_kr[189]};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0x81);
    CHECK(static_cast<unsigned char>(result[1]) == 0xFE);
}

TEST_CASE("euc_kr encode agrees with a linear scan of index euc-kr for every code point",
          "[transcoding::euc_kr_encode]") {
    for (int pointer = 0; pointer < 23940; ++pointer) {
        char32_t cp = detail::tables::euc_kr[pointer];
        if (cp == 0)
            continue;
        int first = 0;
        while (detail::tables::euc_kr[first] != cp)
            ++first;
        std::vector<char32_t> cps{cp};
        auto                  result = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
        REQUIRE(result.size() == 2);
        CHECK(static_cast<unsigned char>(result[0]) == (first / 190) + 0x81);
        CHECK(static_cast<unsigned char>(result[1]) == (first % 190) + 0x41);
    }
}

TEST_CASE("euc_kr encode then decode round-trips every code point in index euc-kr", "[transcoding::euc_kr_encode]") {
    for (char32_t cp : detail::tables::euc_kr) {
        if (cp == 0)
            continue;
        std::vector<char32_t> cps{cp};
        auto                  bytes = collect(cps | whatwg::encode<whatwg::encoding::euc_kr>);
        std::vector<char32_t> back;
        for (char32_t c : bytes | whatwg::decode<whatwg::encoding::euc_kr>)
            back.push_back(c);
        REQUIRE(back.size() == 1);
        CHECK(back[0] == cp);
    }
}

TEST_CASE("euc_kr encode consteval two-byte", "[transcoding::euc_kr_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode = [](char32_t cp) consteval {
        std::array<char32_t, 1> cps{cp};
        std::span<char32_t>     sp(cps);
        std::array<char, 2>     out{};
        int                     n = 0;
        for (char c : sp | whatwg::encode<whatwg::encoding::euc_kr>)
            out[static_cast<std::size_t>(n++)] = c;
        return out;
    };
    CHECK(constify(encode(U'\xAC00')) == std::array<char, 2>{'\xB0', '\xA1'});
}
