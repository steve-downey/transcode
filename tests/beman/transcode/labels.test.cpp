// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/labels.hpp>
#include <beman/transcode/detail/labels.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("get_encoding: utf-8 labels", "[transcoding::labels]") {
    CHECK(get_encoding("utf-8") == codec::utf_8);
    CHECK(get_encoding("UTF-8") == codec::utf_8);
    CHECK(get_encoding("utf8") == codec::utf_8);
    CHECK(get_encoding("  UTF-8  ") == codec::utf_8);
    CHECK(get_encoding("unicode-1-1-utf-8") == codec::utf_8);
}

TEST_CASE("get_encoding: shift_jis labels", "[transcoding::labels]") {
    CHECK(get_encoding("shift_jis") == codec::shift_jis);
    CHECK(get_encoding("Shift_JIS") == codec::shift_jis);
    CHECK(get_encoding("sjis") == codec::shift_jis);
    CHECK(get_encoding("SJIS") == codec::shift_jis);
    CHECK(get_encoding("x-sjis") == codec::shift_jis);
    CHECK(get_encoding("ms932") == codec::shift_jis);
}

TEST_CASE("get_encoding: windows-1252 labels", "[transcoding::labels]") {
    CHECK(get_encoding("windows-1252") == codec::windows_1252);
    CHECK(get_encoding("ascii") == codec::windows_1252);
    CHECK(get_encoding("cp1252") == codec::windows_1252);
}

TEST_CASE("get_encoding: unknown label", "[transcoding::labels]") {
    CHECK(get_encoding("unknown") == std::nullopt);
    CHECK(get_encoding("") == std::nullopt);
    CHECK(get_encoding("not-a-codec") == std::nullopt);
}

TEST_CASE("get_encoding: whitespace stripping", "[transcoding::labels]") {
    CHECK(get_encoding("\tUTF-8\n") == codec::utf_8);
    CHECK(get_encoding("  gb18030  ") == codec::gb18030);
}

TEST_CASE("get_encoding: various codec labels", "[transcoding::labels]") {
    CHECK(get_encoding("gb18030") == codec::gb18030);
    CHECK(get_encoding("big5") == codec::big5);
    CHECK(get_encoding("euc-jp") == codec::euc_jp);
    CHECK(get_encoding("euc-kr") == codec::euc_kr);
    CHECK(get_encoding("iso-2022-jp") == codec::iso_2022_jp);
    CHECK(get_encoding("utf-16le") == codec::utf_16le);
    CHECK(get_encoding("utf-16be") == codec::utf_16be);
    CHECK(get_encoding("replacement") == codec::replacement);
    CHECK(get_encoding("x-user-defined") == codec::x_user_defined);
}

TEST_CASE("get_encoding: consteval", "[transcoding::labels]") {
    CHECK(constify(get_encoding("utf-8")) == codec::utf_8);
    CHECK(constify(get_encoding("unknown")) == std::nullopt);
    CHECK(constify(get_encoding("shift_jis")) == codec::shift_jis);
}
