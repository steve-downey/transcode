// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/transcode_string.hpp>
#include <beman/transcode/detail/transcode_string.hpp>

#include <catch2/catch_all.hpp>

#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("transcode_string: empty input yields empty output", "[transcode_string]") {
    std::span<const char> empty{};
    CHECK(transcode_string(empty, codec::utf_8, codec::utf_8).empty());
}

TEST_CASE("transcode_string: UTF-8 identity round-trip", "[transcode_string]") {
    std::string src = "hello";
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::utf_8) == "hello");
}

TEST_CASE("transcode_string: UTF-8 multibyte identity", "[transcode_string]") {
    // U+4E2D (中) in UTF-8: 0xE4 0xB8 0xAD
    std::string src{'\xE4', '\xB8', '\xAD'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::utf_8) == src);
}

TEST_CASE("transcode_string: windows-1252 to UTF-8 (euro sign)", "[transcode_string]") {
    // 0x80 in windows-1252 → U+20AC EURO SIGN → UTF-8: 0xE2 0x82 0xAC
    std::string src{'\x80'};
    std::string expected{'\xE2', '\x82', '\xAC'};
    CHECK(transcode_string(std::span<const char>(src), codec::windows_1252, codec::utf_8) == expected);
}

TEST_CASE("transcode_string: UTF-8 to windows-1252 (euro sign)", "[transcode_string]") {
    // U+20AC in UTF-8: 0xE2 0x82 0xAC → windows-1252: 0x80
    std::string src{'\xE2', '\x82', '\xAC'};
    std::string expected{'\x80'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::windows_1252) == expected);
}

TEST_CASE("transcode_string: invalid UTF-8 replaced with U+FFFD in UTF-8 output", "[transcode_string]") {
    // 0xFF is invalid UTF-8; whatwg_decode yields U+FFFD; whatwg_encode<utf_8> yields 0xEF 0xBF 0xBD
    std::string src{'\xFF'};
    std::string expected{'\xEF', '\xBF', '\xBD'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::utf_8) == expected);
}

TEST_CASE("transcode_string: ASCII preserved through shift_jis", "[transcode_string]") {
    std::string src = "abc";
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::shift_jis) == "abc");
}

TEST_CASE("transcode_string: UTF-8 to GBK to UTF-8 round-trip (中)", "[transcode_string]") {
    // U+4E2D (中) in UTF-8: 0xE4 0xB8 0xAD; in GBK: 0xD6 0xD0
    std::string utf8{'\xE4', '\xB8', '\xAD'};
    std::string gbk = transcode_string(std::span<const char>(utf8), codec::utf_8, codec::gbk);
    std::string back = transcode_string(std::span<const char>(gbk), codec::gbk, codec::utf_8);
    CHECK(back == utf8);
}

TEST_CASE("transcode_string: unmapped codepoint encoded as '?' in single-byte target", "[transcode_string]") {
    // U+4E2D (中) cannot encode in windows-1252, yields '?'
    std::string src{'\xE4', '\xB8', '\xAD'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::windows_1252) == "?");
}

TEST_CASE("transcode_string: replacement codec decodes to U+FFFD for non-empty input", "[transcode_string]") {
    // replacement codec: any non-empty input → one U+FFFD; U+FFFD in UTF-8 = 0xEF 0xBF 0xBD
    std::string src = "hi";
    std::string expected{'\xEF', '\xBF', '\xBD'};
    CHECK(transcode_string(std::span<const char>(src), codec::replacement, codec::utf_8) == expected);
}
