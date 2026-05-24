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
// whatwg_encode<codec::euc_jp>
// ---------------------------------------------------------------------------

TEST_CASE("euc_jp encode ASCII 'A' -> 0x41", "[transcoding::euc_jp_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("euc_jp encode ASCII NUL -> 0x00", "[transcoding::euc_jp_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("euc_jp encode U+00A5 (YEN SIGN) -> 0x5C", "[transcoding::euc_jp_encode]") {
    // WHATWG special case
    std::vector<char32_t> cps{U'\x00A5'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x5C');
}

TEST_CASE("euc_jp encode U+203E (OVERLINE) -> 0x7E", "[transcoding::euc_jp_encode]") {
    // WHATWG special case
    std::vector<char32_t> cps{U'\x203E'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x7E');
}

TEST_CASE("euc_jp encode U+FF61 (half-width katakana) -> 0x8E 0xA1", "[transcoding::euc_jp_encode]") {
    // U+FF61 -> SS2 prefix 0x8E + 0xA1
    std::vector<char32_t> cps{U'\xFF61'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x8E');
    CHECK(result[1] == '\xA1');
}

TEST_CASE("euc_jp encode U+FF9F (half-width katakana) -> 0x8E 0xDF", "[transcoding::euc_jp_encode]") {
    // U+FF9F -> SS2 prefix 0x8E + 0xA1 + (0xFF9F - 0xFF61) = 0xDF
    std::vector<char32_t> cps{U'\xFF9F'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x8E');
    CHECK(result[1] == '\xDF');
}

TEST_CASE("euc_jp encode U+3000 -> 0xA1 0xA1 (JIS X 0208 pointer 0)", "[transcoding::euc_jp_encode]") {
    // pointer 0 -> lead=(0/94)+0xA1=0xA1, trail=(0%94)+0xA1=0xA1
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xA1');
    CHECK(result[1] == '\xA1');
}

TEST_CASE("euc_jp encode U+4E00 (一) -> 0xB0 0xEC (JIS X 0208 pointer 1485)", "[transcoding::euc_jp_encode]") {
    // pointer 1485: lead=(1485/94)+0xA1 = 15+0xA1=0xB0, trail=(1485%94)+0xA1 = 75+0xA1=0xEC
    std::vector<char32_t> cps{U'\x4E00'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xB0');
    CHECK(result[1] == '\xEC');
}

TEST_CASE("euc_jp encode U+02D8 (BREVE) -> 0x8F 0xA2 0xAF (JIS X 0212 pointer 108)",
          "[transcoding::euc_jp_encode]") {
    // pointer 108 in jis0212: lead=(108/94)+0xA1=1+0xA1=0xA2, trail=(108%94)+0xA1=14+0xA1=0xAF
    // Encoded as: 0x8F (SS3) + 0xA2 + 0xAF
    std::vector<char32_t> cps{U'\x02D8'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x8F');
    CHECK(result[1] == '\xA2');
    CHECK(result[2] == '\xAF');
}

TEST_CASE("euc_jp encode unmapped codepoint -> '?'", "[transcoding::euc_jp_encode]") {
    // U+1F600 (GRINNING FACE emoji) is not in EUC-JP -> '?'
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("euc_jp encode mixed ASCII and JIS X 0208", "[transcoding::euc_jp_encode]") {
    // 'A' + U+3000
    std::vector<char32_t> cps{U'A', U'\x3000'};
    auto                  result = collect(cps | whatwg_encode<codec::euc_jp>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(result[1] == '\xA1');
    CHECK(result[2] == '\xA1');
}

TEST_CASE("euc_jp encode pipe syntax", "[transcoding::euc_jp_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg_encode<codec::euc_jp>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("euc_jp encode consteval ASCII", "[transcoding::euc_jp_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg_encode<codec::euc_jp>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::euc_jp>
// ---------------------------------------------------------------------------

TEST_CASE("euc_jp or_error encode ASCII 'A'", "[transcoding::euc_jp_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("euc_jp or_error encode U+3000 -> 0xA1 0xA1", "[transcoding::euc_jp_encode_or_error]") {
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == '\xA1');
    CHECK(result[1].value() == '\xA1');
}

TEST_CASE("euc_jp or_error encode U+02D8 -> 0x8F 0xA2 0xAF", "[transcoding::euc_jp_encode_or_error]") {
    std::vector<char32_t> cps{U'\x02D8'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    REQUIRE(result[2].has_value());
    CHECK(result[0].value() == '\x8F');
    CHECK(result[1].value() == '\xA2');
    CHECK(result[2].value() == '\xAF');
}

TEST_CASE("euc_jp or_error encode unmapped codepoint yields error", "[transcoding::euc_jp_encode_or_error]") {
    // U+1F600 -> unmapped_codepoint error
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::euc_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}
