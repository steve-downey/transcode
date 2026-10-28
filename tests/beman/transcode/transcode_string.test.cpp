// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/transcode_string.hpp>
#include <beman/transcode/whatwg/transcode_string.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <concepts>
#include <optional>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("whatwg::transcode_string: empty input yields an engaged empty result", "[whatwg::transcode_string]") {
    std::span<const char> empty{};
    auto                  result = whatwg::transcode_string(empty, whatwg::encoding::utf_8, whatwg::encoding::utf_8);

    STATIC_REQUIRE(std::same_as<decltype(result), std::optional<std::string>>);
    REQUIRE(result.has_value());
    CHECK(result->empty());
}

TEST_CASE("whatwg::has_encoder identifies the decode-only codecs", "[whatwg::transcode_string]") {
    CHECK(constify(whatwg::has_encoder(whatwg::encoding::utf_8)));
    CHECK_FALSE(constify(whatwg::has_encoder(whatwg::encoding::utf_16be)));
    CHECK_FALSE(constify(whatwg::has_encoder(whatwg::encoding::utf_16le)));
    CHECK_FALSE(constify(whatwg::has_encoder(whatwg::encoding::replacement)));
    CHECK_FALSE(constify(whatwg::has_encoder(whatwg::encoding::x_user_defined)));
}

TEST_CASE("whatwg::transcode_string: UTF-8 identity round-trip", "[whatwg::transcode_string]") {
    std::string src = "hello";
    CHECK(whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::utf_8) ==
          "hello");
}

TEST_CASE("whatwg::transcode_string: UTF-8 multibyte identity", "[whatwg::transcode_string]") {
    // U+4E2D (中) in UTF-8: 0xE4 0xB8 0xAD
    std::string src{'\xE4', '\xB8', '\xAD'};
    CHECK(whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::utf_8) ==
          src);
}

TEST_CASE("whatwg::transcode_string: windows-1252 to UTF-8 (euro sign)", "[whatwg::transcode_string]") {
    // 0x80 in windows-1252 → U+20AC EURO SIGN → UTF-8: 0xE2 0x82 0xAC
    std::string src{'\x80'};
    std::string expected{'\xE2', '\x82', '\xAC'};
    CHECK(whatwg::transcode_string(
              std::span<const char>(src), whatwg::encoding::windows_1252, whatwg::encoding::utf_8) == expected);
}

TEST_CASE("whatwg::transcode_string: UTF-8 to windows-1252 (euro sign)", "[whatwg::transcode_string]") {
    // U+20AC in UTF-8: 0xE2 0x82 0xAC → windows-1252: 0x80
    std::string src{'\xE2', '\x82', '\xAC'};
    std::string expected{'\x80'};
    CHECK(whatwg::transcode_string(
              std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::windows_1252) == expected);
}

TEST_CASE("whatwg::transcode_string: invalid UTF-8 replaced with U+FFFD in UTF-8 output",
          "[whatwg::transcode_string]") {
    // 0xFF is invalid UTF-8; whatwg::decode yields U+FFFD; whatwg::encode<utf_8> yields 0xEF 0xBF 0xBD
    std::string src{'\xFF'};
    std::string expected{'\xEF', '\xBF', '\xBD'};
    CHECK(whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::utf_8) ==
          expected);
}

TEST_CASE("whatwg::transcode_string: ASCII preserved through shift_jis", "[whatwg::transcode_string]") {
    std::string src = "abc";
    CHECK(whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::shift_jis) ==
          "abc");
}

TEST_CASE("whatwg::transcode_string: UTF-8 to GBK to UTF-8 round-trip (中)", "[whatwg::transcode_string]") {
    // U+4E2D (中) in UTF-8: 0xE4 0xB8 0xAD; in GBK: 0xD6 0xD0
    std::string utf8{'\xE4', '\xB8', '\xAD'};
    std::string gbk =
        whatwg::transcode_string(std::span<const char>(utf8), whatwg::encoding::utf_8, whatwg::encoding::gbk).value();
    std::string back =
        whatwg::transcode_string(std::span<const char>(gbk), whatwg::encoding::gbk, whatwg::encoding::utf_8).value();
    CHECK(back == utf8);
}

TEST_CASE("whatwg::transcode_string: unmapped codepoint encoded as '?' in single-byte target",
          "[whatwg::transcode_string]") {
    // U+4E2D (中) cannot encode in windows-1252, yields '?'
    std::string src{'\xE4', '\xB8', '\xAD'};
    CHECK(whatwg::transcode_string(
              std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::windows_1252) == "?");
}

TEST_CASE("whatwg::transcode_string: replacement codec decodes to U+FFFD for non-empty input",
          "[whatwg::transcode_string]") {
    // replacement codec: any non-empty input → one U+FFFD; U+FFFD in UTF-8 = 0xEF 0xBF 0xBD
    std::string src = "hi";
    std::string expected{'\xEF', '\xBF', '\xBD'};
    CHECK(whatwg::transcode_string(
              std::span<const char>(src), whatwg::encoding::replacement, whatwg::encoding::utf_8) == expected);
}

// ---------------------------------------------------------------------------
// Coverage audit: all remaining decode switch arms — ASCII identity
// ---------------------------------------------------------------------------

TEST_CASE("whatwg::transcode_string: ASCII identity through all uncovered single-byte decode arms",
          "[whatwg::transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    // x_user_defined: 0x00-0x7F map to identity codepoints
    CHECK(whatwg::transcode_string(src, whatwg::encoding::x_user_defined, whatwg::encoding::utf_8) == "abc");
    // IBM866 (Cyrillic): ASCII passes through
    CHECK(whatwg::transcode_string(src, whatwg::encoding::ibm866, whatwg::encoding::utf_8) == "abc");
    // ISO-8859 family
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_2, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_3, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_4, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_5, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_6, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_7, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_8, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_8_i, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_10, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_13, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_14, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_15, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_8859_16, whatwg::encoding::utf_8) == "abc");
    // KOI8
    CHECK(whatwg::transcode_string(src, whatwg::encoding::koi8_r, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::koi8_u, whatwg::encoding::utf_8) == "abc");
    // Mac / Windows single-byte
    CHECK(whatwg::transcode_string(src, whatwg::encoding::macintosh, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_874, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1250, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1251, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1253, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1254, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1255, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1256, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1257, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::windows_1258, whatwg::encoding::utf_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::x_mac_cyrillic, whatwg::encoding::utf_8) == "abc");
    // Shift_JIS decode (encode was already tested; ASCII passes through)
    CHECK(whatwg::transcode_string(src, whatwg::encoding::shift_jis, whatwg::encoding::utf_8) == "abc");
}

// ---------------------------------------------------------------------------
// Coverage audit: all remaining encode switch arms — ASCII identity
// ---------------------------------------------------------------------------

TEST_CASE("whatwg::transcode_string: ASCII identity through all uncovered single-byte encode arms",
          "[whatwg::transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::ibm866) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_2) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_3) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_4) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_5) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_6) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_7) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_8) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_8_i) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_10) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_13) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_14) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_15) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_8859_16) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::koi8_r) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::koi8_u) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::macintosh) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_874) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1250) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1251) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1253) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1254) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1255) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1256) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1257) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::windows_1258) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::x_mac_cyrillic) == "abc");
}

// ---------------------------------------------------------------------------
// Coverage audit: UTF-16 decode arms
// ---------------------------------------------------------------------------

TEST_CASE("whatwg::transcode_string: UTF-16LE decode ASCII", "[whatwg::transcode_string][coverage]") {
    // "hi" in UTF-16LE: 0x68,0x00, 0x69,0x00
    std::string src{'\x68', '\x00', '\x69', '\x00'};
    CHECK(whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_16le, whatwg::encoding::utf_8) ==
          "hi");
}

TEST_CASE("whatwg::transcode_string: UTF-16BE decode ASCII", "[whatwg::transcode_string][coverage]") {
    // "hi" in UTF-16BE: 0x00,0x68, 0x00,0x69
    std::string src{'\x00', '\x68', '\x00', '\x69'};
    CHECK(whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_16be, whatwg::encoding::utf_8) ==
          "hi");
}

TEST_CASE("whatwg::transcode_string: decode-only targets return nullopt", "[whatwg::transcode_string][coverage]") {
    std::string src = "hi";

    CHECK_FALSE(
        whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::utf_16be)
            .has_value());
    CHECK_FALSE(
        whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::utf_16le)
            .has_value());
    CHECK_FALSE(
        whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::replacement)
            .has_value());
    CHECK_FALSE(
        whatwg::transcode_string(std::span<const char>(src), whatwg::encoding::utf_8, whatwg::encoding::x_user_defined)
            .has_value());
}

// ---------------------------------------------------------------------------
// Coverage audit: CJK decode + encode arms — ASCII identity
// ---------------------------------------------------------------------------

TEST_CASE("whatwg::transcode_string: ASCII identity through all CJK decode arms",
          "[whatwg::transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    // gb18030: ASCII bytes map to same codepoints
    CHECK(whatwg::transcode_string(src, whatwg::encoding::gb18030, whatwg::encoding::utf_8) == "abc");
    // big5: ASCII bytes pass through unchanged
    CHECK(whatwg::transcode_string(src, whatwg::encoding::big5, whatwg::encoding::utf_8) == "abc");
    // euc_jp: 0x00-0x7F are ASCII
    CHECK(whatwg::transcode_string(src, whatwg::encoding::euc_jp, whatwg::encoding::utf_8) == "abc");
    // iso_2022_jp: initial state is ASCII; bytes 0x00-0x7E pass directly
    CHECK(whatwg::transcode_string(src, whatwg::encoding::iso_2022_jp, whatwg::encoding::utf_8) == "abc");
    // euc_kr: 0x00-0x7F are ASCII
    CHECK(whatwg::transcode_string(src, whatwg::encoding::euc_kr, whatwg::encoding::utf_8) == "abc");
}

TEST_CASE("whatwg::transcode_string: ASCII identity through all CJK encode arms",
          "[whatwg::transcode_string][coverage]") {
    std::string           ascii = "abc";
    std::span<const char> src(ascii);
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::gb18030) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::big5) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::euc_jp) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::iso_2022_jp) == "abc");
    CHECK(whatwg::transcode_string(src, whatwg::encoding::utf_8, whatwg::encoding::euc_kr) == "abc");
}
