// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("whatwg::get_encoding: utf-8 labels", "[transcoding::labels]") {
    CHECK(whatwg::get_encoding("utf-8") == whatwg::encoding::utf_8);
    CHECK(whatwg::get_encoding("UTF-8") == whatwg::encoding::utf_8);
    CHECK(whatwg::get_encoding("utf8") == whatwg::encoding::utf_8);
    CHECK(whatwg::get_encoding("  UTF-8  ") == whatwg::encoding::utf_8);
    CHECK(whatwg::get_encoding("unicode-1-1-utf-8") == whatwg::encoding::utf_8);
}

TEST_CASE("whatwg::get_encoding: shift_jis labels", "[transcoding::labels]") {
    CHECK(whatwg::get_encoding("shift_jis") == whatwg::encoding::shift_jis);
    CHECK(whatwg::get_encoding("Shift_JIS") == whatwg::encoding::shift_jis);
    CHECK(whatwg::get_encoding("sjis") == whatwg::encoding::shift_jis);
    CHECK(whatwg::get_encoding("SJIS") == whatwg::encoding::shift_jis);
    CHECK(whatwg::get_encoding("x-sjis") == whatwg::encoding::shift_jis);
    CHECK(whatwg::get_encoding("ms932") == whatwg::encoding::shift_jis);
}

TEST_CASE("whatwg::get_encoding: windows-1252 labels", "[transcoding::labels]") {
    CHECK(whatwg::get_encoding("windows-1252") == whatwg::encoding::windows_1252);
    CHECK(whatwg::get_encoding("ascii") == whatwg::encoding::windows_1252);
    CHECK(whatwg::get_encoding("cp1252") == whatwg::encoding::windows_1252);
}

TEST_CASE("whatwg::get_encoding: unknown label", "[transcoding::labels]") {
    CHECK(whatwg::get_encoding("unknown") == std::nullopt);
    CHECK(whatwg::get_encoding("") == std::nullopt);
    CHECK(whatwg::get_encoding("not-a-codec") == std::nullopt);
}

TEST_CASE("whatwg::get_encoding: whitespace stripping", "[transcoding::labels]") {
    CHECK(whatwg::get_encoding("\tUTF-8\n") == whatwg::encoding::utf_8);
    CHECK(whatwg::get_encoding("  gb18030  ") == whatwg::encoding::gb18030);
}

TEST_CASE("whatwg::get_encoding: various codec labels", "[transcoding::labels]") {
    CHECK(whatwg::get_encoding("gb18030") == whatwg::encoding::gb18030);
    CHECK(whatwg::get_encoding("big5") == whatwg::encoding::big5);
    CHECK(whatwg::get_encoding("euc-jp") == whatwg::encoding::euc_jp);
    CHECK(whatwg::get_encoding("euc-kr") == whatwg::encoding::euc_kr);
    CHECK(whatwg::get_encoding("iso-2022-jp") == whatwg::encoding::iso_2022_jp);
    CHECK(whatwg::get_encoding("utf-16le") == whatwg::encoding::utf_16le);
    CHECK(whatwg::get_encoding("utf-16be") == whatwg::encoding::utf_16be);
    CHECK(whatwg::get_encoding("replacement") == whatwg::encoding::replacement);
    CHECK(whatwg::get_encoding("x-user-defined") == whatwg::encoding::x_user_defined);
}

TEST_CASE("whatwg::get_encoding: consteval", "[transcoding::labels]") {
    CHECK(constify(whatwg::get_encoding("utf-8")) == whatwg::encoding::utf_8);
    CHECK(constify(whatwg::get_encoding("unknown")) == std::nullopt);
    CHECK(constify(whatwg::get_encoding("shift_jis")) == whatwg::encoding::shift_jis);
}

TEST_CASE("whatwg::get_encoding: label >= 64 chars yields nullopt", "[transcoding::labels]") {
    // The buffer limit is 63 bytes; a label of 64+ chars cannot match any codec.
    std::string long_label(64, 'a');
    CHECK(whatwg::get_encoding(long_label) == std::nullopt);
}
