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
// whatwg_decode<codec::gbk>
// ---------------------------------------------------------------------------

TEST_CASE("gbk decode ASCII passthrough 'A'", "[transcoding::gbk]") {
    // ASCII bytes pass through unchanged
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("gbk decode ASCII passthrough NUL", "[transcoding::gbk]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("gbk decode multibyte 0x81 0x40 -> U+4E02", "[transcoding::gbk]") {
    // index = (0x81 - 0x81) * 190 + (0x40 - 0x40) - 0 = 0 -> U+4E02
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E02');
}

TEST_CASE("gbk decode multibyte lead 0x81 trail 0x41 -> U+4E04", "[transcoding::gbk]") {
    // index = 0 * 190 + (0x41 - 0x40) - 0 = 1 -> U+4E04
    std::vector<char> bytes{'\x81', '\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E04');
}

TEST_CASE("gbk decode trail byte 0x7F is invalid", "[transcoding::gbk]") {
    // Trail byte 0x7F is excluded; error emitted, 0x7F re-processed as ASCII.
    std::vector<char> bytes{'\x81', '\x7F'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\xFFFD');
    CHECK(result[1] == U'\x7F');
}

TEST_CASE("gbk decode byte 0x80 -> U+20AC (euro)", "[transcoding::gbk]") {
    // WHATWG: byte 0x80 is a special case mapping to U+20AC.
    std::vector<char> bytes{'\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x20AC');
}

TEST_CASE("gbk decode truncated multibyte -> U+FFFD", "[transcoding::gbk]") {
    // Lead byte with no trail
    std::vector<char> bytes{'\x81'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("gbk decode mixed ASCII and multibyte", "[transcoding::gbk]") {
    // 'A' then 0x81 0x40 (U+4E02)
    std::vector<char> bytes{'\x41', '\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x4E02');
}

TEST_CASE("gbk decode pipe syntax with span", "[transcoding::gbk]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::gbk>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("gbk decode consteval ASCII", "[transcoding::gbk]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::gbk>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

TEST_CASE("gbk decode invalid lead byte 0xFF -> U+FFFD", "[transcoding::gbk]") {
    // 0xFF > 0xFE: invalid lead, only the lead byte is consumed
    std::vector<char> bytes{'\xFF'};
    auto              result = collect(bytes | whatwg_decode<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::gbk>
// ---------------------------------------------------------------------------

TEST_CASE("gbk or_error: ASCII passthrough", "[transcoding::gbk_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gbk>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("gbk or_error: valid multibyte 0x81 0x40", "[transcoding::gbk_or_error]") {
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gbk>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x4E02');
}

TEST_CASE("gbk or_error: byte 0x80 yields U+20AC (euro)", "[transcoding::gbk_or_error]") {
    // WHATWG: 0x80 is a special case mapping to U+20AC.
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(result[0].has_value());
    CHECK(result[0].value() == char32_t(0x20AC));
}

TEST_CASE("gbk or_error: truncated multibyte yields error", "[transcoding::gbk_or_error]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gbk>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("gbk or_error: trail byte 0x7F yields error then re-processes", "[transcoding::gbk_or_error]") {
    // 0x7F is not a valid trail; error emitted, 0x7F re-processed as ASCII.
    std::vector<char> bytes{'\x81', '\x7F'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gbk>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
    CHECK(result[1].has_value());
    CHECK(result[1].value() == char32_t(0x7F));
}
