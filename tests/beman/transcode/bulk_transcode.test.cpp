// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Tests that ranges::to and ranges::copy compose naturally with the
// decode/encode views, making dedicated bulk helpers unnecessary.

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <array>
#include <ranges>
#include <span>
#include <string>
#include <vector>

using beman::transcoding::codec;
using beman::transcoding::tests::constify;

namespace {

constexpr auto decode_ascii_constexpr() {
    constexpr std::array<char, 2> source{'A', 'B'};
    std::array<char32_t, 2>       output{};
    std::ranges::copy(source | beman::transcoding::whatwg_decode<codec::utf_8>, output.begin());
    return output;
}

constexpr auto encode_ascii_constexpr() {
    constexpr std::array<char32_t, 2> source{U'A', U'B'};
    std::array<char, 2>               output{};
    std::ranges::copy(source | beman::transcoding::whatwg_encode<codec::utf_8>, output.begin());
    return output;
}

} // namespace

TEST_CASE("view|to: decode UTF-8 to vector", "[bulk]") {
    const std::string input = "Hello";
    auto result = input | beman::transcoding::whatwg_decode<codec::utf_8> | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result == std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
}

TEST_CASE("view|to: decode single-byte iso-8859-15", "[bulk]") {
    const std::string input{'A', static_cast<char>(0xA4)};
    auto              result =
        input | beman::transcoding::whatwg_decode<codec::iso_8859_15> | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result == std::vector<char32_t>{U'A', U'€'});
}

TEST_CASE("view|to: encode UTF-8 to string", "[bulk]") {
    const std::u32string input = U"Hello";
    auto result = input | beman::transcoding::whatwg_encode<codec::utf_8> | std::ranges::to<std::string>();
    CHECK(result == "Hello");
}

TEST_CASE("view|to: encode single-byte iso-8859-15", "[bulk]") {
    const std::u32string input{U'A', U'€'};
    const std::string    expected{'A', static_cast<char>(0xA4)};
    auto result = input | beman::transcoding::whatwg_encode<codec::iso_8859_15> | std::ranges::to<std::string>();
    CHECK(result == expected);
}

TEST_CASE("view|to: encode to alternate container (vector<char>)", "[bulk]") {
    const std::u32string input = U"Hi";
    auto bytes = input | beman::transcoding::whatwg_encode<codec::utf_8> | std::ranges::to<std::vector<char>>();
    REQUIRE(bytes == std::vector<char>{'H', 'i'});
}

TEST_CASE("ranges::copy: decode iso-8859-15 to output iterator", "[bulk]") {
    const std::string     input{'A', static_cast<char>(0xA4)};
    std::vector<char32_t> output;
    std::ranges::copy(input | beman::transcoding::whatwg_decode<codec::iso_8859_15>, std::back_inserter(output));
    REQUIRE(output == std::vector<char32_t>{U'A', U'€'});
}

TEST_CASE("ranges::copy: encode iso-8859-15 to output iterator", "[bulk]") {
    const std::u32string input{U'A', U'€'};
    std::string          output;
    std::ranges::copy(input | beman::transcoding::whatwg_encode<codec::iso_8859_15>, std::back_inserter(output));
    REQUIRE(output == std::string{'A', static_cast<char>(0xA4)});
}

TEST_CASE("view|to: round-trip UTF-8", "[bulk]") {
    using namespace beman::transcoding;
    const std::string original = "Hello, World!";
    auto              cps      = original | whatwg_decode<codec::utf_8> | std::ranges::to<std::vector<char32_t>>();
    auto              result   = cps | whatwg_encode<codec::utf_8> | std::ranges::to<std::string>();
    CHECK(result == original);
}

TEST_CASE("view|to: round-trip ISO-8859-15", "[bulk]") {
    using namespace beman::transcoding;
    const std::string original{'A', static_cast<char>(0xA4)};
    auto              cps    = original | whatwg_decode<codec::iso_8859_15> | std::ranges::to<std::vector<char32_t>>();
    auto              result = cps | whatwg_encode<codec::iso_8859_15> | std::ranges::to<std::string>();
    CHECK(result == original);
}

TEST_CASE("view|to: std::byte input is accepted", "[bulk]") {
    constexpr std::array<std::byte, 2> input{std::byte{0x41}, std::byte{0xA4}};
    auto                               result =
        input | beman::transcoding::whatwg_decode<codec::iso_8859_15> | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result == std::vector<char32_t>{U'A', U'€'});
}

TEST_CASE("view|to: empty input yields empty output", "[bulk]") {
    using namespace beman::transcoding;
    const std::span<const char>     empty_bytes{};
    const std::span<const char32_t> empty_code_points{};

    CHECK((empty_bytes | whatwg_decode<codec::utf_8> | std::ranges::to<std::vector<char32_t>>()).empty());
    CHECK((empty_code_points | whatwg_encode<codec::utf_8> | std::ranges::to<std::string>()).empty());
    CHECK((empty_code_points | whatwg_encode<codec::utf_8> | std::ranges::to<std::vector<char>>()).empty());
}

TEST_CASE("ranges::copy: constexpr decode UTF-8 ASCII", "[bulk][constexpr]") {
    CHECK(constify(decode_ascii_constexpr()) == std::array<char32_t, 2>{U'A', U'B'});
}

TEST_CASE("ranges::copy: constexpr encode UTF-8 ASCII", "[bulk][constexpr]") {
    CHECK(constify(encode_ascii_constexpr()) == std::array<char, 2>{'A', 'B'});
}

// ---------------------------------------------------------------------------
// Single-byte codecs with unmapped bytes/codepoints: verify the views
// handle replacement correctly even without a dedicated bulk path.
// ---------------------------------------------------------------------------

TEST_CASE("view|to: iso_8859_6 unmapped byte yields U+FFFD", "[bulk]") {
    const std::string input{static_cast<char>(0xA1)};
    auto              result =
        input | beman::transcoding::whatwg_decode<codec::iso_8859_6> | std::ranges::to<std::vector<char32_t>>();
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("view|to: iso_8859_6 unmapped codepoint yields '?'", "[bulk]") {
    const std::u32string input{U'Ā'};
    auto result = input | beman::transcoding::whatwg_encode<codec::iso_8859_6> | std::ranges::to<std::string>();
    CHECK(result == "?");
}
