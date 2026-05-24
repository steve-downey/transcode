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
// whatwg_decode<codec::big5>
// ---------------------------------------------------------------------------

TEST_CASE("big5 decode ASCII passthrough 'A'", "[transcoding::big5]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("big5 decode ASCII NUL", "[transcoding::big5]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("big5 decode 0xA4 0x40 -> U+4E00 (pointer 5495)", "[transcoding::big5]") {
    // lead=0xA4, trail=0x40 -> offset=0 -> pointer=(0xA4-0x81)*157+0=35*157=5495 -> U+4E00
    std::vector<char> bytes{'\xA4', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E00');
}

TEST_CASE("big5 decode 2-codepoint pointer 1133: 0x88 0x62 -> U+00CA U+0304", "[transcoding::big5]") {
    // lead=0x88, trail=0x62 -> offset=0x62-0x40=34 -> pointer=7*157+34=1133
    std::vector<char> bytes{'\x88', '\x62'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\x00CA');
    CHECK(result[1] == U'\x0304');
}

TEST_CASE("big5 decode 2-codepoint pointer 1135: 0x88 0x64 -> U+00CA U+030C", "[transcoding::big5]") {
    // lead=0x88, trail=0x64 -> offset=36 -> pointer=7*157+36=1135
    std::vector<char> bytes{'\x88', '\x64'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\x00CA');
    CHECK(result[1] == U'\x030C');
}

TEST_CASE("big5 decode 2-codepoint pointer 1164: 0x88 0xA3 -> U+00EA U+0304", "[transcoding::big5]") {
    // lead=0x88, trail=0xA3 -> offset=0xA3-0x62=65 -> pointer=7*157+65=1164
    std::vector<char> bytes{'\x88', '\xA3'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\x00EA');
    CHECK(result[1] == U'\x0304');
}

TEST_CASE("big5 decode 2-codepoint pointer 1165: 0x88 0xA4 -> U+00EA U+030C", "[transcoding::big5]") {
    // lead=0x88, trail=0xA4 -> offset=66 -> pointer=7*157+66=1165
    // Special case overrides table entry U+1EBF
    std::vector<char> bytes{'\x88', '\xA4'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\x00EA');
    CHECK(result[1] == U'\x030C');
}

TEST_CASE("big5 decode null table entry 0x81 0x40 -> U+FFFD", "[transcoding::big5]") {
    // pointer 0 is unmapped in the Big5 table
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("big5 decode bad trail byte 0x81 0x80 -> U+FFFD", "[transcoding::big5]") {
    // trail 0x80 is not in valid range 0x40-0x7E or 0xA1-0xFE
    std::vector<char> bytes{'\x81', '\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("big5 decode truncated lead byte -> U+FFFD", "[transcoding::big5]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("big5 decode mixed ASCII and 2-byte", "[transcoding::big5]") {
    // 'A' then U+4E00 (0xA4 0x40)
    std::vector<char> bytes{'\x41', '\xA4', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::big5>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x4E00');
}

TEST_CASE("big5 decode pipe syntax with span", "[transcoding::big5]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::big5>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("big5 decode consteval ASCII", "[transcoding::big5]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::big5>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::big5>
// ---------------------------------------------------------------------------

TEST_CASE("big5 or_error: ASCII passthrough", "[transcoding::big5_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("big5 or_error: valid 2-byte 0xA4 0x40 -> U+4E00", "[transcoding::big5_or_error]") {
    std::vector<char> bytes{'\xA4', '\x40'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x4E00');
}

TEST_CASE("big5 or_error: null table entry -> invalid_byte error", "[transcoding::big5_or_error]") {
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("big5 or_error: bad trail byte -> invalid_byte error", "[transcoding::big5_or_error]") {
    std::vector<char> bytes{'\x81', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("big5 or_error: truncated -> truncated_sequence error", "[transcoding::big5_or_error]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::big5>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("big5 or_error: 2-codepoint pointer 1133 -> two successive values", "[transcoding::big5_or_error]") {
    std::vector<char> bytes{'\x88', '\x62'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::big5>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == U'\x00CA');
    CHECK(result[1].value() == U'\x0304');
}
