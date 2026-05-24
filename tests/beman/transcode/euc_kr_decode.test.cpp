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
// whatwg_decode<codec::euc_kr>
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr decode ASCII passthrough 'A' (0x41)", "[transcoding::euc_kr]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("euc_kr decode ASCII NUL (0x00)", "[transcoding::euc_kr]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("euc_kr decode pointer 0: 0x81 0x41 -> U+AC02", "[transcoding::euc_kr]") {
    // pointer = (0x81 - 0x81) * 190 + (0x41 - 0x41) = 0 -> U+AC02
    std::vector<char> bytes{'\x81', '\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xAC02');
}

TEST_CASE("euc_kr decode U+AC00 (가): 0xB0 0xA2", "[transcoding::euc_kr]") {
    // pointer = (0xB0 - 0x81) * 190 + (0xA2 - 0x41) - 1 = 47*190 + 96 = 9026 -> U+AC00
    // (trail 0xA2 > 0x7F, so offset = 0xA2 - 0x41 - 1 = 96)
    std::vector<char> bytes{'\xB0', '\xA2'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xAC00');
}

TEST_CASE("euc_kr decode invalid lead byte 0x80 -> U+FFFD", "[transcoding::euc_kr]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_kr decode invalid lead byte 0xFF -> U+FFFD", "[transcoding::euc_kr]") {
    std::vector<char> bytes{'\xFF'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_kr decode invalid trail byte 0x7F -> U+FFFD", "[transcoding::euc_kr]") {
    // trail 0x7F is DEL, excluded from valid trail bytes
    std::vector<char> bytes{'\x81', '\x7F'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_kr decode invalid trail byte 0x40 -> U+FFFD", "[transcoding::euc_kr]") {
    // trail must be >= 0x41
    std::vector<char> bytes{'\x81', '\x40'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_kr decode unmapped pointer (pointer 26): 0x81 0x5B -> U+FFFD", "[transcoding::euc_kr]") {
    // pointer = (0x81-0x81)*190 + (0x5B-0x41) = 20 -> unmapped (table[20]==0 but actually..
    // trail=0x5B, offset = 0x5B-0x41=26, 0x5B < 0x7F -> no skip adjustment
    // pointer = 0*190 + 26 = 26, euc_kr[26] == 0 -> U+FFFD
    std::vector<char> bytes{'\x81', '\x5B'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_kr decode truncated sequence: lone lead 0x81 -> U+FFFD", "[transcoding::euc_kr]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("euc_kr decode mixed ASCII + Korean", "[transcoding::euc_kr]") {
    // 'A' (0x41) + U+AC00 (0xB0 0xA2)
    std::vector<char> bytes{'\x41', '\xB0', '\xA2'};
    auto              result = collect(bytes | whatwg_decode<codec::euc_kr>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\xAC00');
}

TEST_CASE("euc_kr decode pipe syntax with span", "[transcoding::euc_kr]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::euc_kr>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("euc_kr decode consteval ASCII", "[transcoding::euc_kr]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::euc_kr>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::euc_kr>
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr or_error: ASCII passthrough", "[transcoding::euc_kr_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("euc_kr or_error: pointer 0 -> U+AC02", "[transcoding::euc_kr_or_error]") {
    std::vector<char> bytes{'\x81', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\xAC02');
}

TEST_CASE("euc_kr or_error: U+AC00 (가) -> success", "[transcoding::euc_kr_or_error]") {
    std::vector<char> bytes{'\xB0', '\xA2'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\xAC00');
}

TEST_CASE("euc_kr or_error: invalid lead byte yields error", "[transcoding::euc_kr_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("euc_kr or_error: truncated sequence yields error", "[transcoding::euc_kr_or_error]") {
    std::vector<char> bytes{'\x81'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("euc_kr or_error: invalid trail byte yields error", "[transcoding::euc_kr_or_error]") {
    std::vector<char> bytes{'\x81', '\x7F'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::euc_kr>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}
