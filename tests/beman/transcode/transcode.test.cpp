// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <ranges>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("transcode.hpp: views::null_term accessible", "[transcoding::umbrella]") {
    const char* s = "hello";
    auto        r = views::null_term(s);
    CHECK(std::ranges::distance(r) == 5);
}

TEST_CASE("transcode.hpp: whatwg_decode accessible via pipe", "[transcoding::umbrella]") {
    std::vector<char> utf8{'h', 'e', 'l', 'l', 'o'};
    std::u32string    result;
    for (char32_t cp : utf8 | whatwg_decode<codec::utf_8>)
        result.push_back(cp);
    CHECK(result == U"hello");
}

TEST_CASE("transcode.hpp: whatwg_encode accessible via pipe", "[transcoding::umbrella]") {
    std::u32string src = U"hello";
    std::string    result;
    for (char b : src | whatwg_encode<codec::utf_8>)
        result.push_back(b);
    CHECK(result == "hello");
}

TEST_CASE("transcode.hpp: get_encoding accessible", "[transcoding::umbrella]") {
    CHECK(get_encoding("utf-8") == codec::utf_8);
    CHECK(get_encoding("shift_jis") == codec::shift_jis);
    CHECK(get_encoding("unknown-xyz") == std::nullopt);
}

TEST_CASE("transcode.hpp: sniff_encoding accessible", "[transcoding::umbrella]") {
    std::vector<unsigned char> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    CHECK(sniff_encoding(utf8_bom) == codec::utf_8);

    std::vector<unsigned char> no_bom = {'h', 'i'};
    CHECK(sniff_encoding(no_bom) == std::nullopt);
}

TEST_CASE("transcode.hpp: decode+encode round-trip via umbrella header", "[transcoding::umbrella]") {
    std::vector<char> utf8{'\xE4', '\xB8', '\xAD'};

    std::u32string codepoints;
    for (char32_t cp : utf8 | whatwg_decode<codec::utf_8>)
        codepoints.push_back(cp);
    CHECK(codepoints == U"中");

    std::vector<char> result;
    for (char b : codepoints | whatwg_encode<codec::utf_8>)
        result.push_back(b);
    CHECK(result == utf8);
}

TEST_CASE("transcode.hpp: consteval sniff_encoding via umbrella", "[transcoding::umbrella]") {
    constexpr std::array<unsigned char, 3> utf8_bom = {0xEF, 0xBB, 0xBF};
    CHECK(constify(sniff_encoding(utf8_bom)) == codec::utf_8);
}

TEST_CASE("transcode.hpp: null_term + decode round-trip", "[transcoding::umbrella]") {
    const char*    s = "hi";
    std::u32string result;
    for (char32_t cp : views::null_term(s) | whatwg_decode<codec::utf_8>)
        result.push_back(cp);
    CHECK(result == U"hi");
}
