// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Tests for the bulk helpers decode_to / encode_to / decode_into /
// encode_into, and for the ranges::to and ranges::copy pipelines each one
// names. Both spellings are proposed, so both are covered here.

#include <beman/transcode/detail/bulk_transcode.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
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
// Single-byte codecs with unmapped bytes/codepoints: verify the view pipelines
// handle replacement correctly before checking the named wrappers below.
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

TEST_CASE("decode_to: UTF-8 to vector", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string input = "Hello";
    CHECK(decode_to<codec::utf_8>(input) == std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
    CHECK(decode_to<codec::utf_8>(input) ==
          (input | whatwg_decode<codec::utf_8> | std::ranges::to<std::vector<char32_t>>()));
}

TEST_CASE("decode_to: single-byte iso-8859-15", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string input{'A', static_cast<char>(0xA4)};
    CHECK(decode_to<codec::iso_8859_15>(input) == std::vector<char32_t>{U'A', U'\u20AC'});
}

TEST_CASE("decode_to: std::byte input is accepted", "[bulk][named]") {
    using namespace beman::transcoding;
    constexpr std::array<std::byte, 2> input{std::byte{0x41}, std::byte{0xA4}};
    CHECK(decode_to<codec::iso_8859_15>(input) == std::vector<char32_t>{U'A', U'\u20AC'});
}

TEST_CASE("decode_to: empty input yields empty output", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string input;
    CHECK(decode_to<codec::utf_8>(input).empty());
}

TEST_CASE("decode_to: unmapped byte yields U+FFFD", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string input{static_cast<char>(0xA1)};
    CHECK(decode_to<codec::iso_8859_6>(input) == std::vector<char32_t>{U'\uFFFD'});
}

TEST_CASE("encode_to: UTF-8 to string", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::u32string input = U"Hello";
    CHECK(encode_to<codec::utf_8>(input) == "Hello");
    CHECK(encode_to<codec::utf_8>(input) == (input | whatwg_encode<codec::utf_8> | std::ranges::to<std::string>()));
}

TEST_CASE("encode_to: single-byte iso-8859-15", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::u32string input{U'A', U'\u20AC'};
    CHECK(encode_to<codec::iso_8859_15>(input) == std::string{'A', static_cast<char>(0xA4)});
}

TEST_CASE("encode_to: alternate container", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::u32string input = U"Hi";
    CHECK(encode_to<codec::utf_8, std::vector<char>>(input) == std::vector<char>{'H', 'i'});
}

TEST_CASE("encode_to: unmapped codepoint yields '?'", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::u32string input{U'\u0100'};
    CHECK(encode_to<codec::iso_8859_6>(input) == "?");
}

TEST_CASE("decode_into: writes through an output iterator", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string     input{'A', static_cast<char>(0xA4)};
    std::vector<char32_t> output;
    decode_into<codec::iso_8859_15>(input, std::back_inserter(output));
    CHECK(output == std::vector<char32_t>{U'A', U'\u20AC'});
}

TEST_CASE("decode_into: empty input writes nothing", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string     input;
    std::vector<char32_t> output;
    decode_into<codec::utf_8>(input, std::back_inserter(output));
    CHECK(output.empty());
}

TEST_CASE("encode_into: writes through an output iterator", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::u32string input{U'A', U'\u20AC'};
    std::string          output;
    encode_into<codec::iso_8859_15>(input, std::back_inserter(output));
    CHECK(output == std::string{'A', static_cast<char>(0xA4)});
}

TEST_CASE("encode_into: empty input writes nothing", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::u32string input;
    std::string          output;
    encode_into<codec::utf_8>(input, std::back_inserter(output));
    CHECK(output.empty());
}

TEST_CASE("named helpers: round-trip UTF-8", "[bulk][named]") {
    using namespace beman::transcoding;
    const std::string original = "Hello, World!";
    CHECK(encode_to<codec::utf_8>(decode_to<codec::utf_8>(original)) == original);
}

namespace {

consteval std::size_t decode_to_size_consteval() {
    constexpr std::array<char, 2> source{'A', 'B'};
    return beman::transcoding::decode_to<codec::utf_8>(std::span<const char>(source)).size();
}

consteval char32_t decode_to_first_consteval() {
    constexpr std::array<char, 2> source{'A', 'B'};
    return beman::transcoding::decode_to<codec::utf_8>(std::span<const char>(source)).front();
}

consteval char encode_to_first_consteval() {
    constexpr std::array<char32_t, 2> source{U'A', U'B'};
    return beman::transcoding::encode_to<codec::utf_8>(std::span<const char32_t>(source)).front();
}

consteval auto decode_into_consteval() {
    constexpr std::array<char, 2> source{'A', 'B'};
    std::array<char32_t, 2>       output{};
    beman::transcoding::decode_into<codec::utf_8>(std::span<const char>(source), output.begin());
    return output;
}

consteval auto encode_into_consteval() {
    constexpr std::array<char32_t, 2> source{U'A', U'B'};
    std::array<char, 2>               output{};
    beman::transcoding::encode_into<codec::utf_8>(std::span<const char32_t>(source), output.begin());
    return output;
}

} // namespace

TEST_CASE("constexpr: decode_to is usable in a constant expression", "[bulk][named][constexpr]") {
    CHECK(constify(decode_to_size_consteval()) == 2u);
    CHECK(constify(decode_to_first_consteval()) == U'A');
}

TEST_CASE("constexpr: encode_to is usable in a constant expression", "[bulk][named][constexpr]") {
    CHECK(constify(encode_to_first_consteval()) == 'A');
}

TEST_CASE("constexpr: decode_into is usable in a constant expression", "[bulk][named][constexpr]") {
    constexpr auto decoded = decode_into_consteval();
    STATIC_REQUIRE(decoded[0] == U'A');
    STATIC_REQUIRE(decoded[1] == U'B');
}

TEST_CASE("constexpr: encode_into is usable in a constant expression", "[bulk][named][constexpr]") {
    constexpr auto encoded = encode_into_consteval();
    STATIC_REQUIRE(encoded[0] == 'A');
    STATIC_REQUIRE(encoded[1] == 'B');
}
