// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import beman.transcode;

#include <catch2/catch_all.hpp>

#include <string>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("module: whatwg_decode via import", "[transcoding::module]") {
    std::vector<char> utf8{'h', 'e', 'l', 'l', 'o'};
    std::u32string    result;
    for (char32_t cp : utf8 | whatwg_decode<codec::utf_8>)
        result.push_back(cp);
    CHECK(result == U"hello");
}

TEST_CASE("module: whatwg_encode via import", "[transcoding::module]") {
    std::u32string src = U"hello";
    std::string    result;
    for (char b : src | whatwg_encode<codec::utf_8>)
        result.push_back(b);
    CHECK(result == "hello");
}

TEST_CASE("module: get_encoding via import", "[transcoding::module]") {
    CHECK(get_encoding("utf-8") == codec::utf_8);
    CHECK(get_encoding("shift_jis") == codec::shift_jis);
}

TEST_CASE("module: sniff_encoding via import", "[transcoding::module]") {
    std::vector<unsigned char> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    CHECK(sniff_encoding(utf8_bom) == codec::utf_8);
}

TEST_CASE("module: iconv_functions type accessible", "[transcoding::module]") {
    static_assert(std::is_class_v<iconv_functions>);
}

TEST_CASE("module: iconv_transcode_view type accessible", "[transcoding::module]") {
    static_assert(std::is_class_v<iconv_transcode_view<iconv_functions, std::vector<char>>>);
}

TEST_CASE("module: iconv_transcode_or_error_view type accessible", "[transcoding::module]") {
    static_assert(std::is_class_v<iconv_transcode_or_error_view<iconv_functions, std::vector<char>>>);
}

TEST_CASE("module: make_real_iconv_fns accessible", "[transcoding::module]") {
    auto fns = make_real_iconv_fns();
    CHECK(fns.open != nullptr);
    CHECK(fns.convert != nullptr);
    CHECK(fns.close != nullptr);
}

TEST_CASE("module: iconv_transcode pipe via import", "[transcoding::module]") {
    std::vector<char> utf8{'h', 'i'};
    std::vector<char> buf(64);
    std::vector<char> result;

    for (char ch : utf8 | iconv_transcode("UTF-8", "UTF-32LE", buf)) {
        result.push_back(ch);
    }

    // UTF-32LE encoding of "hi" (2 chars × 4 bytes/char = 8 bytes)
    CHECK(result.size() == 8);
    CHECK(result[0] == 'h');
    CHECK(result[1] == 0x00);
}

TEST_CASE("module: round-trip UTF-8 decode/encode via import", "[transcoding::module]") {
    std::vector<char> utf8{'\xE4', '\xB8', '\xAD'}; // 中 in UTF-8

    std::u32string codepoints;
    for (char32_t cp : utf8 | whatwg_decode<codec::utf_8>)
        codepoints.push_back(cp);
    CHECK(codepoints == U"中");

    std::vector<char> result;
    for (char b : codepoints | whatwg_encode<codec::utf_8>)
        result.push_back(b);
    CHECK(result == utf8);
}
