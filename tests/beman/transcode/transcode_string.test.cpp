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
    std::string gbk  = transcode_string(std::span<const char>(utf8), codec::utf_8, codec::gbk);
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

// ---------------------------------------------------------------------------
// Coverage audit: all remaining decode switch arms — ASCII identity
// ---------------------------------------------------------------------------

TEST_CASE("transcode_string: ASCII identity through all uncovered single-byte decode arms",
          "[transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    // x_user_defined: 0x00-0x7F map to identity codepoints
    CHECK(transcode_string(src, codec::x_user_defined, codec::utf_8) == "abc");
    // IBM866 (Cyrillic): ASCII passes through
    CHECK(transcode_string(src, codec::ibm866, codec::utf_8) == "abc");
    // ISO-8859 family
    CHECK(transcode_string(src, codec::iso_8859_2, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_3, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_4, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_5, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_6, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_7, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_8, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_8_i, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_10, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_13, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_14, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_15, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::iso_8859_16, codec::utf_8) == "abc");
    // KOI8
    CHECK(transcode_string(src, codec::koi8_r, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::koi8_u, codec::utf_8) == "abc");
    // Mac / Windows single-byte
    CHECK(transcode_string(src, codec::macintosh, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_874, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1250, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1251, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1253, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1254, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1255, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1256, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1257, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::windows_1258, codec::utf_8) == "abc");
    CHECK(transcode_string(src, codec::x_mac_cyrillic, codec::utf_8) == "abc");
    // Shift_JIS decode (encode was already tested; ASCII passes through)
    CHECK(transcode_string(src, codec::shift_jis, codec::utf_8) == "abc");
}

// ---------------------------------------------------------------------------
// Coverage audit: all remaining encode switch arms — ASCII identity
// ---------------------------------------------------------------------------

TEST_CASE("transcode_string: ASCII identity through all uncovered single-byte encode arms",
          "[transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    CHECK(transcode_string(src, codec::utf_8, codec::ibm866) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_2) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_3) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_4) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_5) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_6) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_7) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_8) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_8_i) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_10) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_13) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_14) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_15) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_8859_16) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::koi8_r) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::koi8_u) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::macintosh) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_874) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1250) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1251) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1253) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1254) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1255) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1256) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1257) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::windows_1258) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::x_mac_cyrillic) == "abc");
}

// ---------------------------------------------------------------------------
// Coverage audit: UTF-16 decode + encode arms
// ---------------------------------------------------------------------------

TEST_CASE("transcode_string: UTF-16LE decode ASCII", "[transcode_string][coverage]") {
    // "hi" in UTF-16LE: 0x68,0x00, 0x69,0x00
    std::string src{'\x68', '\x00', '\x69', '\x00'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_16le, codec::utf_8) == "hi");
}

TEST_CASE("transcode_string: UTF-16BE decode ASCII", "[transcode_string][coverage]") {
    // "hi" in UTF-16BE: 0x00,0x68, 0x00,0x69
    std::string src{'\x00', '\x68', '\x00', '\x69'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_16be, codec::utf_8) == "hi");
}

TEST_CASE("transcode_string: UTF-16LE encode ASCII", "[transcode_string][coverage]") {
    // 'h'=U+0068 → 0x68,0x00; 'i'=U+0069 → 0x69,0x00
    std::string src = "hi";
    std::string expected{'\x68', '\x00', '\x69', '\x00'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::utf_16le) == expected);
}

TEST_CASE("transcode_string: UTF-16BE encode ASCII", "[transcode_string][coverage]") {
    // 'h'=U+0068 → 0x00,0x68; 'i'=U+0069 → 0x00,0x69
    std::string src = "hi";
    std::string expected{'\x00', '\x68', '\x00', '\x69'};
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::utf_16be) == expected);
}

// ---------------------------------------------------------------------------
// Coverage audit: CJK decode + encode arms — ASCII identity
// ---------------------------------------------------------------------------

TEST_CASE("transcode_string: ASCII identity through all CJK decode arms", "[transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    // gb18030: ASCII bytes map to same codepoints
    CHECK(transcode_string(src, codec::gb18030, codec::utf_8) == "abc");
    // big5: ASCII bytes pass through unchanged
    CHECK(transcode_string(src, codec::big5, codec::utf_8) == "abc");
    // euc_jp: 0x00-0x7F are ASCII
    CHECK(transcode_string(src, codec::euc_jp, codec::utf_8) == "abc");
    // iso_2022_jp: initial state is ASCII; bytes 0x00-0x7E pass directly
    CHECK(transcode_string(src, codec::iso_2022_jp, codec::utf_8) == "abc");
    // euc_kr: 0x00-0x7F are ASCII
    CHECK(transcode_string(src, codec::euc_kr, codec::utf_8) == "abc");
}

TEST_CASE("transcode_string: ASCII identity through all CJK encode arms", "[transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    CHECK(transcode_string(src, codec::utf_8, codec::gb18030) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::big5) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::euc_jp) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::iso_2022_jp) == "abc");
    CHECK(transcode_string(src, codec::utf_8, codec::euc_kr) == "abc");
}

// ---------------------------------------------------------------------------
// Coverage audit: default encode arm — replacement and x_user_defined have no encoder
// ---------------------------------------------------------------------------

TEST_CASE("transcode_string: to=replacement yields empty (no encoder defined)", "[transcode_string][coverage]") {
    std::string src = "abc";
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::replacement).empty());
}

TEST_CASE("transcode_string: to=x_user_defined yields empty (no encoder defined)", "[transcode_string][coverage]") {
    std::string src = "abc";
    CHECK(transcode_string(std::span<const char>(src), codec::utf_8, codec::x_user_defined).empty());
}
