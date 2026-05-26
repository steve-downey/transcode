// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/bulk_transcode.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <span>
#include <string>
#include <vector>

using beman::transcoding::codec;
using beman::transcoding::decode_into;
using beman::transcoding::decode_to;
using beman::transcoding::encode_into;
using beman::transcoding::encode_to;
using beman::transcoding::tests::constify;

namespace {

constexpr auto decode_ascii_constexpr() {
    constexpr std::array<char, 2> source{'A', 'B'};
    std::array<char32_t, 2>        output{};
    decode_into<codec::utf_8>(source, output.begin());
    return output;
}

constexpr auto encode_ascii_constexpr() {
    constexpr std::array<char32_t, 2> source{U'A', U'B'};
    std::array<char, 2>               output{};
    encode_into<codec::utf_8>(source, output.begin());
    return output;
}

} // namespace

TEST_CASE("bulk_transcode: decode_to UTF-8", "[bulk]") {
    const std::string input = "Hello";
    auto              result = decode_to<codec::utf_8>(input);
    REQUIRE(result == std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
}

TEST_CASE("bulk_transcode: decode_to single-byte fast path", "[bulk]") {
    const std::string input{'A', static_cast<char>(0xA4)};
    auto              result = decode_to<codec::iso_8859_15>(input);
    REQUIRE(result == std::vector<char32_t>{U'A', U'\u20AC'});
}

TEST_CASE("bulk_transcode: encode_to UTF-8", "[bulk]") {
    const std::u32string input = U"Hello";
    CHECK(encode_to<codec::utf_8>(input) == "Hello");
}

TEST_CASE("bulk_transcode: encode_to single-byte fast path", "[bulk]") {
    const std::u32string input{U'A', U'\u20AC'};
    const std::string    expected{'A', static_cast<char>(0xA4)};
    CHECK(encode_to<codec::iso_8859_15>(input) == expected);
}

TEST_CASE("bulk_transcode: encode_to supports alternate container", "[bulk]") {
    const std::u32string input = U"Hi";
    auto                 bytes = encode_to<codec::utf_8, std::vector<char>>(input);
    REQUIRE(bytes == std::vector<char>{'H', 'i'});
}

TEST_CASE("bulk_transcode: decode_into appends to output iterator", "[bulk]") {
    const std::string      input{'A', static_cast<char>(0xA4)};
    std::vector<char32_t>  output;
    decode_into<codec::iso_8859_15>(input, std::back_inserter(output));
    REQUIRE(output == std::vector<char32_t>{U'A', U'\u20AC'});
}

TEST_CASE("bulk_transcode: encode_into appends to output iterator", "[bulk]") {
    const std::u32string input{U'A', U'\u20AC'};
    std::string          output;
    encode_into<codec::iso_8859_15>(input, std::back_inserter(output));
    REQUIRE(output == std::string{'A', static_cast<char>(0xA4)});
}

TEST_CASE("bulk_transcode: round-trip UTF-8", "[bulk]") {
    const std::string original = "Hello, World!";
    CHECK(encode_to<codec::utf_8>(decode_to<codec::utf_8>(original)) == original);
}

TEST_CASE("bulk_transcode: round-trip ISO-8859-15 fast path", "[bulk]") {
    const std::string original{'A', static_cast<char>(0xA4)};
    CHECK(encode_to<codec::iso_8859_15>(decode_to<codec::iso_8859_15>(original)) == original);
}

TEST_CASE("bulk_transcode: std::byte input is accepted", "[bulk]") {
    constexpr std::array<std::byte, 2> input{std::byte{0x41}, std::byte{0xA4}};
    auto                               result = decode_to<codec::iso_8859_15>(input);
    REQUIRE(result == std::vector<char32_t>{U'A', U'\u20AC'});
}

TEST_CASE("bulk_transcode: empty input yields empty output", "[bulk]") {
    const std::span<const char>     empty_bytes{};
    const std::span<const char32_t> empty_code_points{};

    CHECK(decode_to<codec::utf_8>(empty_bytes).empty());
    CHECK(encode_to<codec::utf_8>(empty_code_points).empty());
    CHECK((encode_to<codec::utf_8, std::vector<char>>(empty_code_points)).empty());
}

TEST_CASE("bulk_transcode: decode_into is constexpr for UTF-8 ASCII", "[bulk][constexpr]") {
    CHECK(constify(decode_ascii_constexpr()) == std::array<char32_t, 2>{U'A', U'B'});
}

TEST_CASE("bulk_transcode: encode_into is constexpr for UTF-8 ASCII", "[bulk][constexpr]") {
    CHECK(constify(encode_ascii_constexpr()) == std::array<char, 2>{'A', 'B'});
}
