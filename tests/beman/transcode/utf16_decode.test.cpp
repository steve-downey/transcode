// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char32_t> collect(View&& v) {
    std::vector<char32_t> result;
    for (char32_t cp : v)
        result.push_back(cp);
    return result;
}

template <typename View>
std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg_decode<codec::utf_16be>
// ---------------------------------------------------------------------------

TEST_CASE("utf16be decode ASCII 'A'", "[transcoding::utf16be]") {
    // U+0041 in UTF-16BE: 0x00 0x41
    std::vector<char> bytes{'\x00', '\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("utf16be decode BMP non-ASCII (U+00E9)", "[transcoding::utf16be]") {
    // U+00E9 (e with acute) in UTF-16BE: 0x00 0xE9
    std::vector<char> bytes{'\x00', '\xE9'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x00E9');
}

TEST_CASE("utf16be decode surrogate pair (U+1F600)", "[transcoding::utf16be]") {
    // U+1F600 = 0x10000 + 0xF600
    // high = 0xD800 + ((0xF600) >> 10) = 0xD800 + 0x3D = 0xD83D
    // low  = 0xDC00 + ((0xF600) & 0x3FF) = 0xDC00 + 0x200 = 0xDE00
    // Wait, let me recalculate:
    // U+1F600 = 0x1F600
    // offset = 0x1F600 - 0x10000 = 0xF600
    // high = 0xD800 + (0xF600 >> 10) = 0xD800 + 0x3D = 0xD83D
    // low  = 0xDC00 + (0xF600 & 0x3FF) = 0xDC00 + 0x200 = 0xDE00
    // UTF-16BE: D8 3D DE 00
    std::vector<char> bytes{'\xD8', '\x3D', '\xDE', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x1F600');
}

TEST_CASE("utf16be decode lone high surrogate yields U+FFFD", "[transcoding::utf16be]") {
    // 0xD800 followed by non-surrogate 0x0041 -> U+FFFD, then U+0041
    std::vector<char> bytes{'\xD8', '\x00', '\x00', '\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\xFFFD');
    CHECK(result[1] == U'A');
}

TEST_CASE("utf16be decode lone low surrogate yields U+FFFD", "[transcoding::utf16be]") {
    // 0xDC00 (lone low surrogate) -> U+FFFD
    std::vector<char> bytes{'\xDC', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("utf16be decode truncated input yields U+FFFD", "[transcoding::utf16be]") {
    // Only 1 byte available (need 2)
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("utf16be decode multiple codepoints", "[transcoding::utf16be]") {
    // 'H' 'i' = 0x0048 0x0069 in UTF-16BE
    std::vector<char> bytes{'\x00', '\x48', '\x00', '\x69'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'H');
    CHECK(result[1] == U'i');
}

TEST_CASE("utf16be decode pipe syntax with span", "[transcoding::utf16be]") {
    std::vector<char>         bytes{'\x00', '\x41'};
    std::span<const char>     sp(bytes);
    std::vector<char32_t>     result;
    for (char32_t cp : sp | whatwg_decode<codec::utf_16be>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("utf16be decode consteval", "[transcoding::utf16be]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char            bytes[] = {'\x00', '\x41'};
        std::span<const char>     sp(bytes, 2);
        return *(sp | whatwg_decode<codec::utf_16be>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::utf_16be>
// ---------------------------------------------------------------------------

TEST_CASE("utf16be or_error: ASCII 'A'", "[transcoding::utf16be_or_error]") {
    std::vector<char> bytes{'\x00', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("utf16be or_error: lone high surrogate yields error", "[transcoding::utf16be_or_error]") {
    // 0xD800 followed by non-surrogate -> error
    std::vector<char> bytes{'\xD8', '\x00', '\x00', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'A');
}

TEST_CASE("utf16be or_error: lone low surrogate yields error", "[transcoding::utf16be_or_error]") {
    std::vector<char> bytes{'\xDC', '\x00'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
}

TEST_CASE("utf16be or_error: truncated input yields error", "[transcoding::utf16be_or_error]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("utf16be or_error: valid surrogate pair", "[transcoding::utf16be_or_error]") {
    // U+1F600: D8 3D DE 00
    std::vector<char> bytes{'\xD8', '\x3D', '\xDE', '\x00'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x1F600');
}

// ---------------------------------------------------------------------------
// whatwg_decode<codec::utf_16le>
// ---------------------------------------------------------------------------

TEST_CASE("utf16le decode ASCII 'A'", "[transcoding::utf16le]") {
    // U+0041 in UTF-16LE: 0x41 0x00
    std::vector<char> bytes{'\x41', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16le>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("utf16le decode BMP non-ASCII (U+00E9)", "[transcoding::utf16le]") {
    // U+00E9 in UTF-16LE: 0xE9 0x00
    std::vector<char> bytes{'\xE9', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16le>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x00E9');
}

TEST_CASE("utf16le decode surrogate pair (U+1F600)", "[transcoding::utf16le]") {
    // U+1F600: high=0xD83D, low=0xDE00
    // UTF-16LE: 3D D8 00 DE
    std::vector<char> bytes{'\x3D', '\xD8', '\x00', '\xDE'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16le>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x1F600');
}

TEST_CASE("utf16le decode lone high surrogate yields U+FFFD", "[transcoding::utf16le]") {
    // 0xD800 LE = 0x00 0xD8, then 0x0041 LE = 0x41 0x00
    std::vector<char> bytes{'\x00', '\xD8', '\x41', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16le>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\xFFFD');
    CHECK(result[1] == U'A');
}

TEST_CASE("utf16le decode lone low surrogate yields U+FFFD", "[transcoding::utf16le]") {
    // 0xDC00 LE = 0x00 0xDC
    std::vector<char> bytes{'\x00', '\xDC'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16le>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("utf16le decode multiple codepoints", "[transcoding::utf16le]") {
    // 'H' 'i' in UTF-16LE
    std::vector<char> bytes{'\x48', '\x00', '\x69', '\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::utf_16le>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'H');
    CHECK(result[1] == U'i');
}

TEST_CASE("utf16le or_error: lone surrogate yields error", "[transcoding::utf16le_or_error]") {
    // 0xDC00 LE = 0x00 0xDC
    std::vector<char> bytes{'\x00', '\xDC'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16le>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
}
