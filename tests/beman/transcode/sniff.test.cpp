// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/sniff.hpp>
#include <beman/transcode/sniff.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <forward_list>
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

// A forward range that is not contiguous and not sized, to exercise the
// constraint on something that is not a span in disguise.
TEST_CASE("sniff_encoding: forward-only range with a BOM", "[transcoding::sniff]") {
    std::forward_list<unsigned char> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    CHECK(sniff_encoding(utf8_bom) == codec::utf_8);
}

TEST_CASE("sniff_encoding: forward-only range without a BOM", "[transcoding::sniff]") {
    std::forward_list<unsigned char> no_bom = {'h', 'i', '!'};
    CHECK(sniff_encoding(no_bom) == std::nullopt);
}

// The point of requiring forward_range.  The docblock promises the mark is not
// consumed and the range can be passed on unchanged; on a single-pass range
// that promise cannot hold, so this is what the constraint buys.  The non-BOM
// case matters more than the BOM case: it is the common one, and the bytes
// that would go missing are ordinary document content.
TEST_CASE("sniff_encoding: does not consume its argument", "[transcoding::sniff]") {
    const std::vector<unsigned char> before = {0xEF, 0xBB, 0xBF, 'h', 'i'};

    std::forward_list<unsigned char> bom(before.begin(), before.end());
    REQUIRE(sniff_encoding(bom) == codec::utf_8);
    CHECK(std::vector<unsigned char>(bom.begin(), bom.end()) == before);

    const std::vector<unsigned char> plain = {'h', 'i', '!'};
    std::forward_list<unsigned char> no_bom(plain.begin(), plain.end());
    REQUIRE(sniff_encoding(no_bom) == std::nullopt);
    CHECK(std::vector<unsigned char>(no_bom.begin(), no_bom.end()) == plain);
}

TEST_CASE("sniff_encoding: sniff then decode sees the whole document", "[transcoding::sniff]") {
    // The composition the docblock describes: decide the codec from the mark,
    // then hand the same range to a decode view, which strips the BOM itself.
    const std::vector<char> bytes = {'\xEF', '\xBB', '\xBF', 'h', 'i'};
    REQUIRE(sniff_encoding(bytes) == codec::utf_8);

    std::u32string decoded;
    for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
        decoded.push_back(cp);
    CHECK(decoded == U"hi");
}

TEST_CASE("sniff_encoding: consteval", "[transcoding::sniff]") {
    constexpr std::array<unsigned char, 5> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    CHECK(constify(sniff_encoding(utf8_bom)) == codec::utf_8);

    constexpr std::array<unsigned char, 2> no_bom = {'h', 'i'};
    CHECK(constify(sniff_encoding(no_bom)) == std::nullopt);

    constexpr std::array<unsigned char, 2> utf16be_bom = {0xFE, 0xFF};
    CHECK(constify(sniff_encoding(utf16be_bom)) == codec::utf_16be);

    constexpr std::array<unsigned char, 2> utf16le_bom = {0xFF, 0xFE};
    CHECK(constify(sniff_encoding(utf16le_bom)) == codec::utf_16le);
}

// null_term_view is a forward_range -- its iterator is a contiguous_iterator --
// so the composition in the paper's examples still compiles under the new
// constraint.  This is the case most at risk from it.
TEST_CASE("sniff_encoding: null_term is still accepted", "[transcoding::sniff]") {
    static_assert(std::ranges::forward_range<decltype(views::null_term(""))>);
    const char* bom = "\xEF\xBB\xBF"
                      "hi";
    CHECK(sniff_encoding(views::null_term(bom)) == codec::utf_8);
}
