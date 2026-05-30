// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Tests that pluggable codec views compose with ranges::to and ranges::copy,
// making dedicated bulk helpers unnecessary.

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = table_codec<latin1_upper>;

// ---------------------------------------------------------------------------
// decode(codec) | ranges::to — replaces decode_to(codec, range)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable view|to: ASCII bytes decode correctly", "[pluggable_bulk]") {
    std::string src = "ABC";
    auto result     = std::span<const char>(src) | decode(latin1_codec{}) | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result.size() == 3);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'B');
    CHECK(result[2] == U'C');
}

TEST_CASE("pluggable view|to: upper-half bytes decode via table", "[pluggable_bulk]") {
    std::array<unsigned char, 2> src{0xC0, 0xC1};
    auto                         result =
        std::span<const unsigned char>(src) | decode(latin1_codec{}) | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\x00C0');
    CHECK(result[1] == U'\x00C1');
}

TEST_CASE("pluggable view|to: empty input yields empty output", "[pluggable_bulk]") {
    std::string src;
    auto result = std::span<const char>(src) | decode(latin1_codec{}) | std::ranges::to<std::vector<char32_t>>();
    CHECK(result.empty());
}

TEST_CASE("pluggable view|to: vector<char> input", "[pluggable_bulk]") {
    std::vector<char> src{'\x41', '\xC0'};
    auto              result = src | decode(latin1_codec{}) | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x00C0');
}

// ---------------------------------------------------------------------------
// encode(codec) | ranges::to — replaces encode_to(codec, range)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable view|to: ASCII codepoints encode correctly", "[pluggable_bulk]") {
    std::u32string src    = U"ABC";
    auto           result = src | encode(latin1_codec{}) | std::ranges::to<std::string>();
    CHECK(result == "ABC");
}

TEST_CASE("pluggable view|to: upper-half codepoints encode correctly", "[pluggable_bulk]") {
    std::u32string src{U'\x00C0', U'\x00C1'};
    auto           result = src | encode(latin1_codec{}) | std::ranges::to<std::string>();
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xC0);
    CHECK(static_cast<unsigned char>(result[1]) == 0xC1);
}

TEST_CASE("pluggable view|to: unmapped codepoint yields replacement '?'", "[pluggable_bulk]") {
    std::u32string src{U'\x1F600'};
    auto           result = src | encode(latin1_codec{}) | std::ranges::to<std::string>();
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("pluggable view|to: empty encode input yields empty output", "[pluggable_bulk]") {
    std::u32string src;
    auto           result = src | encode(latin1_codec{}) | std::ranges::to<std::string>();
    CHECK(result.empty());
}

TEST_CASE("pluggable view|to: vector<char> container type", "[pluggable_bulk]") {
    std::u32string src    = U"Hi";
    auto           result = src | encode(latin1_codec{}) | std::ranges::to<std::vector<char>>();
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 'H');
    CHECK(result[1] == 'i');
}

// ---------------------------------------------------------------------------
// ranges::copy — replaces decode_into / encode_into
// ---------------------------------------------------------------------------

TEST_CASE("pluggable ranges::copy: decode bytes to output iterator", "[pluggable_bulk]") {
    std::string           src{'\x41', '\xC0'};
    std::vector<char32_t> result;
    std::ranges::copy(std::span<const char>(src) | decode(latin1_codec{}), std::back_inserter(result));
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x00C0');
}

TEST_CASE("pluggable ranges::copy: empty input writes nothing", "[pluggable_bulk]") {
    std::string           src;
    std::vector<char32_t> result;
    std::ranges::copy(std::span<const char>(src) | decode(latin1_codec{}), std::back_inserter(result));
    CHECK(result.empty());
}

TEST_CASE("pluggable ranges::copy: encode codepoints to output iterator", "[pluggable_bulk]") {
    std::u32string src{U'A', U'\x00C0'};
    std::string    result;
    std::ranges::copy(src | encode(latin1_codec{}), std::back_inserter(result));
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 'A');
    CHECK(static_cast<unsigned char>(result[1]) == 0xC0);
}

TEST_CASE("pluggable ranges::copy: empty encode writes nothing", "[pluggable_bulk]") {
    std::u32string src;
    std::string    result;
    std::ranges::copy(src | encode(latin1_codec{}), std::back_inserter(result));
    CHECK(result.empty());
}

// ---------------------------------------------------------------------------
// Round-trip: decode | to then encode | to
// ---------------------------------------------------------------------------

TEST_CASE("pluggable view|to: round-trip decode then encode", "[pluggable_bulk]") {
    std::string src{'\x41', '\xC0', '\xC1'};
    auto        cps  = std::span<const char>(src) | decode(latin1_codec{}) | std::ranges::to<std::vector<char32_t>>();
    auto        back = cps | encode(latin1_codec{}) | std::ranges::to<std::string>();
    CHECK(back == src);
}

// ---------------------------------------------------------------------------
// Constexpr tests — ranges::copy with fixed-size output array
// ---------------------------------------------------------------------------

TEST_CASE("pluggable constexpr: decode via ranges::copy", "[pluggable_bulk]") {
    constexpr auto result = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xC1};
        std::array<char32_t, 3>      out{};
        std::ranges::copy(std::span(src) | decode(latin1_codec{}), out.begin());
        return out;
    }();
    CHECK(constify(result[0]) == U'A');
    CHECK(constify(result[1]) == U'\x00C0');
    CHECK(constify(result[2]) == U'\x00C1');
}

TEST_CASE("pluggable constexpr: encode via ranges::copy", "[pluggable_bulk]") {
    constexpr auto result = [] {
        std::array<char32_t, 2> src{U'\x00C0', U'\x00C1'};
        std::array<char, 2>     out{};
        std::ranges::copy(std::span(src) | encode(latin1_codec{}), out.begin());
        return out;
    }();
    CHECK(constify(static_cast<unsigned char>(result[0])) == 0xC0u);
    CHECK(constify(static_cast<unsigned char>(result[1])) == 0xC1u);
}
