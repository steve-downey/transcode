// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/sniff.hpp>
#include <beman/transcode/detail/sniff.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("sniff_encoding: UTF-8 BOM (EF BB BF)", "[transcoding::sniff]") {
    std::vector<unsigned char> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    CHECK(sniff_encoding(utf8_bom) == codec::utf_8);
}

TEST_CASE("sniff_encoding: UTF-8 BOM only", "[transcoding::sniff]") {
    std::vector<unsigned char> utf8_bom = {0xEF, 0xBB, 0xBF};
    CHECK(sniff_encoding(utf8_bom) == codec::utf_8);
}

TEST_CASE("sniff_encoding: UTF-16BE BOM (FE FF)", "[transcoding::sniff]") {
    std::vector<unsigned char> utf16be_bom = {0xFE, 0xFF, 0x00, 0x68};
    CHECK(sniff_encoding(utf16be_bom) == codec::utf_16be);
}

TEST_CASE("sniff_encoding: UTF-16LE BOM (FF FE)", "[transcoding::sniff]") {
    std::vector<unsigned char> utf16le_bom = {0xFF, 0xFE, 0x68, 0x00};
    CHECK(sniff_encoding(utf16le_bom) == codec::utf_16le);
}

TEST_CASE("sniff_encoding: no BOM — ASCII", "[transcoding::sniff]") {
    std::vector<unsigned char> no_bom = {'h', 'i'};
    CHECK(sniff_encoding(no_bom) == std::nullopt);
}

TEST_CASE("sniff_encoding: no BOM — empty range", "[transcoding::sniff]") {
    std::vector<unsigned char> empty = {};
    CHECK(sniff_encoding(empty) == std::nullopt);
}

TEST_CASE("sniff_encoding: no BOM — one byte", "[transcoding::sniff]") {
    std::vector<unsigned char> one = {0xEF};
    CHECK(sniff_encoding(one) == std::nullopt);
}

TEST_CASE("sniff_encoding: no BOM — two bytes (not a BOM)", "[transcoding::sniff]") {
    std::vector<unsigned char> two = {0xEF, 0xBB};
    CHECK(sniff_encoding(two) == std::nullopt);
}

TEST_CASE("sniff_encoding: char range", "[transcoding::sniff]") {
    std::vector<char> utf8_bom = {'\xEF', '\xBB', '\xBF', 'h', 'i'};
    CHECK(sniff_encoding(utf8_bom) == codec::utf_8);
}

TEST_CASE("sniff_encoding: with null_term (no BOM)", "[transcoding::sniff]") {
    const char* s = "hello";
    CHECK(sniff_encoding(beman::transcoding::views::null_term(s)) == std::nullopt);
}

TEST_CASE("sniff_encoding: consteval", "[transcoding::sniff]") {
    constexpr std::array<unsigned char, 5> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    CHECK(constify(sniff_encoding(utf8_bom)) == codec::utf_8);

    constexpr std::array<unsigned char, 2> no_bom = {'h', 'i'};
    CHECK(constify(sniff_encoding(no_bom)) == std::nullopt);
}
