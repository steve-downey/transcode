// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/detail/tables/gbk.hpp>
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
// whatwg::encode<whatwg::encoding::gb18030>
// ---------------------------------------------------------------------------

TEST_CASE("gb18030 encode ASCII 'A' -> 0x41", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("gb18030 encode ASCII NUL -> 0x00", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("gb18030 encode U+20AC (euro) -> 0xA2 0xE3 (GBK 2-byte)", "[transcoding::gb18030_encode]") {
    // In encode direction, U+20AC uses the 2-byte GBK encoding 0xA2 0xE3
    // Only decoding of byte 0x80 gives U+20AC (not the other way around)
    std::vector<char32_t> cps{U'€'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xA2);
    CHECK(static_cast<unsigned char>(result[1]) == 0xE3);
}

TEST_CASE("gb18030 encode U+4E02 -> 0x81 0x40 (GBK 2-byte)", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{U'\x4E02'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0x81);
    CHECK(static_cast<unsigned char>(result[1]) == 0x40);
}

TEST_CASE("gb18030 encode U+10000 -> 0x90 0x30 0x81 0x30 (4-byte)", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{U'\U00010000'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 4);
    CHECK(static_cast<unsigned char>(result[0]) == 0x90);
    CHECK(static_cast<unsigned char>(result[1]) == 0x30);
    CHECK(static_cast<unsigned char>(result[2]) == 0x81);
    CHECK(static_cast<unsigned char>(result[3]) == 0x30);
}

TEST_CASE("gb18030 encode U+E5E5 yields replacement byte", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{static_cast<char32_t>(0xE5E5)};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("gb18030 encode mixed ASCII and 2-byte", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{U'A', U'\x4E02'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0x81);
    CHECK(static_cast<unsigned char>(result[2]) == 0x40);
}

TEST_CASE("gb18030 encode pipe syntax", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg::encode<whatwg::encoding::gb18030>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("gb18030 encode consteval ASCII", "[transcoding::gb18030_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg::encode<whatwg::encoding::gb18030>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg::encode_or_error<whatwg::encoding::gb18030>
// ---------------------------------------------------------------------------

TEST_CASE("gb18030 or_error encode ASCII 'A'", "[transcoding::gb18030_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("gb18030 or_error encode U+4E02", "[transcoding::gb18030_encode_or_error]") {
    std::vector<char32_t> cps{U'\x4E02'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0x81);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0x40);
}

TEST_CASE("gb18030 or_error encode U+E5E5 yields unmapped_codepoint", "[transcoding::gb18030_encode_or_error]") {
    std::vector<char32_t> cps{static_cast<char32_t>(0xE5E5)};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg::error::unmapped_codepoint);
}

// Coverage: U+E7C7 (PUA) exercises gb18030_ranges_encode special case (pointer 7457).
TEST_CASE("gb18030 encode U+E7C7 -> 0x81 0x35 0xF4 0x37 (4-byte special case)", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{static_cast<char32_t>(0xE7C7)};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 4);
    CHECK(static_cast<unsigned char>(result[0]) == 0x81);
    CHECK(static_cast<unsigned char>(result[1]) == 0x35);
    CHECK(static_cast<unsigned char>(result[2]) == 0xF4);
    CHECK(static_cast<unsigned char>(result[3]) == 0x37);
}

// Coverage: U+0080 (C1 control) exercises 4-byte range encoding with binary search hi-branch.
TEST_CASE("gb18030 encode U+0080 -> 0x81 0x30 0x81 0x30 (4-byte range)", "[transcoding::gb18030_encode]") {
    std::vector<char32_t> cps{static_cast<char32_t>(0x0080)};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 4);
    CHECK(static_cast<unsigned char>(result[0]) == 0x81);
    CHECK(static_cast<unsigned char>(result[1]) == 0x30);
    CHECK(static_cast<unsigned char>(result[2]) == 0x81);
    CHECK(static_cast<unsigned char>(result[3]) == 0x30);
}

TEST_CASE("gb18030 or_error encode U+10000 (4-byte)", "[transcoding::gb18030_encode_or_error]") {
    std::vector<char32_t> cps{U'\U00010000'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::gb18030>);
    REQUIRE(result.size() == 4);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    REQUIRE(result[2].has_value());
    REQUIRE(result[3].has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0x90);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0x30);
    CHECK(static_cast<unsigned char>(result[2].value()) == 0x81);
    CHECK(static_cast<unsigned char>(result[3].value()) == 0x30);
}

// ---------------------------------------------------------------------------
// The two-byte index lookup
// ---------------------------------------------------------------------------

TEST_CASE("gb18030 encode agrees with a linear scan of index gb18030 for every two-byte code point",
          "[transcoding::gb18030_encode]") {
    for (int pointer = 0; pointer < 23940; ++pointer) {
        char32_t cp = detail::tables::gbk[pointer];
        if (cp == 0)
            continue;
        int first = 0;
        while (detail::tables::gbk[first] != cp)
            ++first;
        int trail = (first % 190) + 0x40;
        if (trail >= 0x7F)
            ++trail;
        std::vector<char32_t> cps{cp};
        auto                  result = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
        REQUIRE(result.size() == 2);
        CHECK(static_cast<unsigned char>(result[0]) == (first / 190) + 0x81);
        CHECK(static_cast<unsigned char>(result[1]) == trail);
    }
}

TEST_CASE("gb18030 encode then decode round-trips every two-byte code point", "[transcoding::gb18030_encode]") {
    for (char32_t cp : detail::tables::gbk) {
        if (cp == 0)
            continue;
        std::vector<char32_t> cps{cp};
        auto                  bytes = collect(cps | whatwg::encode<whatwg::encoding::gb18030>);
        std::vector<char32_t> back;
        for (char32_t c : bytes | whatwg::decode<whatwg::encoding::gb18030>)
            back.push_back(c);
        REQUIRE(back.size() == 1);
        CHECK(back[0] == cp);
    }
}
