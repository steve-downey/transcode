// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/transcode_view.hpp>
#include <beman/transcode/detail/transcode_view.hpp>

#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

// Latin-1 table: bytes 0x80-0xFF identity-map to U+0080-U+00FF
inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = table_codec<latin1_upper>;

// ---------------------------------------------------------------------------
// Basic transcode via pipe adaptor
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: ASCII identity round-trip via pipe", "[pluggable_transcode]") {
    std::string src = "Hello";
    std::string result;
    for (char c : src | pluggable_transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    CHECK(result == "Hello");
}

TEST_CASE("pluggable transcode: upper-half identity round-trip via pipe", "[pluggable_transcode]") {
    std::string src{'\xC0', '\xC1', '\xC2'};
    std::string result;
    for (char c : src | pluggable_transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    CHECK(result == src);
}

TEST_CASE("pluggable transcode: empty input via pipe", "[pluggable_transcode]") {
    std::string src;
    std::string result;
    for (char c : src | pluggable_transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    CHECK(result.empty());
}

TEST_CASE("pluggable transcode: mixed ASCII and upper-half bytes", "[pluggable_transcode]") {
    std::string src{'\x41', '\xC0', '\x42', '\xC1'};
    std::string result;
    for (char c : src | pluggable_transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    REQUIRE(result.size() == 4);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0xC0u);
    CHECK(result[2] == '\x42');
    CHECK(static_cast<unsigned char>(result[3]) == 0xC1u);
}

// ---------------------------------------------------------------------------
// pluggable_transcode closure called directly (not pipe)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: operator() called directly", "[pluggable_transcode]") {
    std::string src = "Hi";
    auto        closure = pluggable_transcode(latin1_codec{}, latin1_codec{});
    std::string result;
    for (char c : closure(src))
        result += c;
    CHECK(result == "Hi");
}

// ---------------------------------------------------------------------------
// span input
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: span<const char> input", "[pluggable_transcode]") {
    std::string           src{'\x41', '\xC0', '\xC1'};
    std::span<const char> sp(src);
    std::string           result;
    for (char c : sp | pluggable_transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0xC0u);
    CHECK(static_cast<unsigned char>(result[2]) == 0xC1u);
}

// ---------------------------------------------------------------------------
// Consteval tests
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: constexpr ASCII round-trip", "[pluggable_transcode]") {
    constexpr auto result = [] {
        std::array<unsigned char, 3> src{0x41, 0x42, 0x43};
        std::array<char, 3>          out{};
        int                          idx = 0;
        for (char c : src | pluggable_transcode(latin1_codec{}, latin1_codec{}))
            out[static_cast<std::size_t>(idx++)] = c;
        return out;
    }();
    CHECK(constify(result[0]) == 'A');
    CHECK(constify(result[1]) == 'B');
    CHECK(constify(result[2]) == 'C');
}

TEST_CASE("pluggable transcode: constexpr upper-half round-trip", "[pluggable_transcode]") {
    constexpr auto result = [] {
        std::array<unsigned char, 2> src{0xC0, 0xC1};
        std::array<unsigned char, 2> out{};
        int                          idx = 0;
        for (char c : src | pluggable_transcode(latin1_codec{}, latin1_codec{}))
            out[static_cast<std::size_t>(idx++)] = static_cast<unsigned char>(c);
        return out;
    }();
    CHECK(constify(result[0]) == 0xC0u);
    CHECK(constify(result[1]) == 0xC1u);
}
