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
// whatwg_decode<codec::gb18030>
// ---------------------------------------------------------------------------

TEST_CASE("gb18030 decode ASCII passthrough 'A'", "[transcoding::gb18030]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("gb18030 decode ASCII passthrough NUL", "[transcoding::gb18030]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("gb18030 decode 0x80 -> U+20AC (EURO SIGN)", "[transcoding::gb18030]") {
    // Special case: 0x80 maps to U+20AC per WHATWG spec
    std::vector<char> bytes{'\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'€');
}

TEST_CASE("gb18030 decode 2-byte GBK sequence 0x81 0x40 -> U+4E02", "[transcoding::gb18030]") {
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E02');
}

TEST_CASE("gb18030 decode 4-byte U+10000: 0x90 0x30 0x81 0x30", "[transcoding::gb18030]") {
    // U+10000 is encoded as 0x90 0x30 0x81 0x30 in GB18030
    std::vector<char> bytes{'\x90', '\x30', '\x81', '\x30'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\U00010000');
}

TEST_CASE("gb18030 decode invalid lead byte -> U+FFFD", "[transcoding::gb18030]") {
    // 0xFF > 0xFE: invalid lead
    std::vector<char> bytes{'\xFF'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("gb18030 decode truncated 2-byte -> U+FFFD", "[transcoding::gb18030]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("gb18030 decode truncated 4-byte sequence -> U+FFFD", "[transcoding::gb18030]") {
    // Lead + second digit byte but no third/fourth
    std::vector<char> bytes{'\x81', '\x30'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("gb18030 decode mixed ASCII and 2-byte", "[transcoding::gb18030]") {
    std::vector<char> bytes{'\x41', '\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::gb18030>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x4E02');
}

TEST_CASE("gb18030 decode pipe syntax with span", "[transcoding::gb18030]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::gb18030>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("gb18030 decode consteval ASCII", "[transcoding::gb18030]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::gb18030>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::gb18030>
// ---------------------------------------------------------------------------

TEST_CASE("gb18030 or_error: ASCII passthrough", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("gb18030 or_error: 0x80 -> U+20AC", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'€');
}

TEST_CASE("gb18030 or_error: valid 2-byte GBK", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x4E02');
}

TEST_CASE("gb18030 or_error: valid 4-byte U+10000", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\x90', '\x30', '\x81', '\x30'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\U00010000');
}

TEST_CASE("gb18030 or_error: invalid lead byte yields error", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\xFF'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("gb18030 or_error: truncated 2-byte yields error", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("gb18030 or_error: truncated 4-byte yields error", "[transcoding::gb18030_or_error]") {
    std::vector<char> bytes{'\x81', '\x30'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::gb18030>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}
