// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("transcode.hpp: views::null_term accessible", "[transcoding::umbrella]") {
    const char* s = "hello";
    auto        r = views::null_term(s);
    CHECK(std::ranges::distance(r) == 5);
}

TEST_CASE("transcode.hpp: whatwg_decode_view accessible", "[transcoding::umbrella]") {
    std::vector<unsigned char> utf8 = {0x68, 0x65, 0x6C, 0x6C, 0x6F};
    auto                       v    = whatwg_decode_view<codec::utf_8>(utf8);
    std::u32string             result(v.begin(), v.end());
    CHECK(result == U"hello");
}

TEST_CASE("transcode.hpp: whatwg_encode_view accessible", "[transcoding::umbrella]") {
    std::u32string src = U"hello";
    auto           v   = whatwg_encode_view<codec::utf_8>(src);
    std::string    result(v.begin(), v.end());
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
    std::vector<unsigned char> utf8 = {0xE4, 0xB8, 0xAD};
    auto                       decoded = whatwg_decode_view<codec::utf_8>(utf8);
    std::u32string             codepoints(decoded.begin(), decoded.end());
    CHECK(codepoints == U"中");

    auto                       encoded = whatwg_encode_view<codec::utf_8>(codepoints);
    std::vector<unsigned char> result(encoded.begin(), encoded.end());
    CHECK(result == utf8);
}

TEST_CASE("transcode.hpp: consteval sniff_encoding via umbrella", "[transcoding::umbrella]") {
    constexpr std::array<unsigned char, 3> utf8_bom = {0xEF, 0xBB, 0xBF};
    CHECK(constify(sniff_encoding(utf8_bom)) == codec::utf_8);
}
