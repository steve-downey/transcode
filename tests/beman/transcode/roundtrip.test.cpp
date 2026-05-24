// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>

#include <vector>

using namespace beman::transcoding;

namespace {

template <codec C>
std::vector<char32_t> encode_then_decode(std::vector<char32_t> cps) {
    std::vector<char> bytes;
    for (char b : cps | whatwg_encode<C>)
        bytes.push_back(b);
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<C>)
        result.push_back(cp);
    return result;
}

template <codec C>
std::vector<char> decode_then_encode(std::vector<char> bytes) {
    std::vector<char32_t> cps;
    for (char32_t cp : bytes | whatwg_decode<C>)
        cps.push_back(cp);
    std::vector<char> result;
    for (char b : cps | whatwg_encode<C>)
        result.push_back(b);
    return result;
}

} // namespace

// ---------------------------------------------------------------------------
// UTF-8
// ---------------------------------------------------------------------------

TEST_CASE("utf_8 encode-then-decode: ASCII, BMP, supplementary", "[roundtrip::utf_8]") {
    std::vector<char32_t> cps{U'A', U'\x00E9', U'\x20AC', U'\x1F600'};
    CHECK(encode_then_decode<codec::utf_8>(cps) == cps);
}

TEST_CASE("utf_8 decode-then-encode: multi-byte sequence", "[roundtrip::utf_8]") {
    // U+20AC (Euro) = E2 82 AC; U+1F600 (emoji) = F0 9F 98 80
    std::vector<char> bytes{'H', 'i', '\xE2', '\x82', '\xAC', '\xF0', '\x9F', '\x98', '\x80'};
    CHECK(decode_then_encode<codec::utf_8>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// UTF-16BE
// ---------------------------------------------------------------------------

TEST_CASE("utf_16be encode-then-decode: ASCII, BMP, supplementary", "[roundtrip::utf_16be]") {
    // U+1F600 encodes as surrogate pair D83D DE00
    std::vector<char32_t> cps{U'A', U'\x4E00', U'\x1F600'};
    CHECK(encode_then_decode<codec::utf_16be>(cps) == cps);
}

TEST_CASE("utf_16be decode-then-encode: known byte sequences", "[roundtrip::utf_16be]") {
    // U+4E00 in UTF-16BE: 0x4E 0x00; U+0041 in UTF-16BE: 0x00 0x41
    std::vector<char> bytes{'\x00', '\x41', '\x4E', '\x00'};
    CHECK(decode_then_encode<codec::utf_16be>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// UTF-16LE
// ---------------------------------------------------------------------------

TEST_CASE("utf_16le encode-then-decode: ASCII, BMP, supplementary", "[roundtrip::utf_16le]") {
    std::vector<char32_t> cps{U'A', U'\x4E00', U'\x1F600'};
    CHECK(encode_then_decode<codec::utf_16le>(cps) == cps);
}

TEST_CASE("utf_16le decode-then-encode: known byte sequences", "[roundtrip::utf_16le]") {
    // U+0041 in UTF-16LE: 0x41 0x00; U+4E00 in UTF-16LE: 0x00 0x4E
    std::vector<char> bytes{'\x41', '\x00', '\x00', '\x4E'};
    CHECK(decode_then_encode<codec::utf_16le>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// Single-byte: windows_1252
// ---------------------------------------------------------------------------

TEST_CASE("windows_1252 encode-then-decode: ASCII and non-ASCII", "[roundtrip::windows_1252]") {
    // U+20AC (Euro) -> 0x80, U+00E9 (e-acute) -> 0xE9
    std::vector<char32_t> cps{U'A', U'\x20AC', U'\x00E9'};
    CHECK(encode_then_decode<codec::windows_1252>(cps) == cps);
}

TEST_CASE("windows_1252 decode-then-encode: known byte values", "[roundtrip::windows_1252]") {
    std::vector<char> bytes{'A', '\x80', '\xE9'};
    CHECK(decode_then_encode<codec::windows_1252>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// Single-byte: koi8_r
// ---------------------------------------------------------------------------

TEST_CASE("koi8_r encode-then-decode: ASCII and Cyrillic", "[roundtrip::koi8_r]") {
    // U+0410 (Cyrillic A) -> 0xE1 in KOI8-R
    std::vector<char32_t> cps{U'A', U'\x0410'};
    CHECK(encode_then_decode<codec::koi8_r>(cps) == cps);
}

TEST_CASE("koi8_r decode-then-encode: Cyrillic byte", "[roundtrip::koi8_r]") {
    std::vector<char> bytes{'A', '\xE1'};
    CHECK(decode_then_encode<codec::koi8_r>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// Single-byte: iso_8859_2
// ---------------------------------------------------------------------------

TEST_CASE("iso_8859_2 encode-then-decode: ASCII and Latin Extended", "[roundtrip::iso_8859_2]") {
    // U+0104 (A with ogonek) -> 0xA1 in iso-8859-2
    std::vector<char32_t> cps{U'A', U'\x0104'};
    CHECK(encode_then_decode<codec::iso_8859_2>(cps) == cps);
}

TEST_CASE("iso_8859_2 decode-then-encode: known byte values", "[roundtrip::iso_8859_2]") {
    std::vector<char> bytes{'A', '\xA1'};
    CHECK(decode_then_encode<codec::iso_8859_2>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// Single-byte: windows_1251
// ---------------------------------------------------------------------------

TEST_CASE("windows_1251 encode-then-decode: ASCII and Cyrillic", "[roundtrip::windows_1251]") {
    // U+0410 (Cyrillic A) -> 0xC0 in windows-1251
    std::vector<char32_t> cps{U'A', U'\x0410'};
    CHECK(encode_then_decode<codec::windows_1251>(cps) == cps);
}

// ---------------------------------------------------------------------------
// GBK
// ---------------------------------------------------------------------------

TEST_CASE("gbk encode-then-decode: ASCII and CJK", "[roundtrip::gbk]") {
    // U+4E00 (一) is a well-known CJK character in GBK
    std::vector<char32_t> cps{U'A', U'\x4E00'};
    CHECK(encode_then_decode<codec::gbk>(cps) == cps);
}

TEST_CASE("gbk decode-then-encode: bytes from encoding", "[roundtrip::gbk]") {
    // derive canonical bytes by encoding, then verify decode->encode returns same bytes
    std::vector<char32_t> cps{U'A', U'\x4E00'};
    std::vector<char>     bytes;
    for (char b : cps | whatwg_encode<codec::gbk>)
        bytes.push_back(b);
    CHECK(decode_then_encode<codec::gbk>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// GB18030
// ---------------------------------------------------------------------------

TEST_CASE("gb18030 encode-then-decode: ASCII, BMP CJK, supplementary", "[roundtrip::gb18030]") {
    // GB18030 covers all of Unicode; U+1F600 encodes as 4 bytes
    std::vector<char32_t> cps{U'A', U'\x4E00', U'\x1F600'};
    CHECK(encode_then_decode<codec::gb18030>(cps) == cps);
}

TEST_CASE("gb18030 decode-then-encode: bytes from encoding", "[roundtrip::gb18030]") {
    std::vector<char32_t> cps{U'A', U'\x4E00', U'\x1F600'};
    std::vector<char>     bytes;
    for (char b : cps | whatwg_encode<codec::gb18030>)
        bytes.push_back(b);
    CHECK(decode_then_encode<codec::gb18030>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// Big5
// ---------------------------------------------------------------------------

TEST_CASE("big5 encode-then-decode: ASCII and Traditional Chinese", "[roundtrip::big5]") {
    // U+4E00 is in Big5 (pointer 5495: 0xA4 0x40)
    // U+00CA (E-circumflex) is a Big5 specific Latin char (pointer 1137: 0x88 0x66)
    std::vector<char32_t> cps{U'A', U'\x4E00', U'\x00CA'};
    CHECK(encode_then_decode<codec::big5>(cps) == cps);
}

TEST_CASE("big5 decode-then-encode: bytes from encoding", "[roundtrip::big5]") {
    std::vector<char32_t> cps{U'A', U'\x4E00'};
    std::vector<char>     bytes;
    for (char b : cps | whatwg_encode<codec::big5>)
        bytes.push_back(b);
    CHECK(decode_then_encode<codec::big5>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// Shift_JIS
// ---------------------------------------------------------------------------

TEST_CASE("shift_jis encode-then-decode: ASCII, katakana, CJK", "[roundtrip::shift_jis]") {
    // U+3000 (IDEOGRAPHIC SPACE) -> 0x81 0x40
    // U+FF61 (half-width katakana) -> single byte 0xA1
    std::vector<char32_t> cps{U'A', U'\x3000', U'\xFF61'};
    CHECK(encode_then_decode<codec::shift_jis>(cps) == cps);
}

TEST_CASE("shift_jis decode-then-encode: bytes from encoding", "[roundtrip::shift_jis]") {
    std::vector<char32_t> cps{U'A', U'\x3000', U'\xFF61'};
    std::vector<char>     bytes;
    for (char b : cps | whatwg_encode<codec::shift_jis>)
        bytes.push_back(b);
    CHECK(decode_then_encode<codec::shift_jis>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// EUC-JP
// ---------------------------------------------------------------------------

TEST_CASE("euc_jp encode-then-decode: ASCII, JIS X 0208, half-width katakana, JIS X 0212",
          "[roundtrip::euc_jp]") {
    // U+3000 -> 0xA1 0xA1 (JIS X 0208)
    // U+FF61 -> 0x8E 0xA1 (SS2 + half-width katakana)
    // U+02D8 -> 0x8F 0xA2 0xAF (SS3 + JIS X 0212)
    std::vector<char32_t> cps{U'A', U'\x3000', U'\xFF61', U'\x02D8'};
    CHECK(encode_then_decode<codec::euc_jp>(cps) == cps);
}

TEST_CASE("euc_jp decode-then-encode: bytes from encoding", "[roundtrip::euc_jp]") {
    std::vector<char32_t> cps{U'A', U'\x3000', U'\xFF61'};
    std::vector<char>     bytes;
    for (char b : cps | whatwg_encode<codec::euc_jp>)
        bytes.push_back(b);
    CHECK(decode_then_encode<codec::euc_jp>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// ISO-2022-JP (stateful encoder)
// ---------------------------------------------------------------------------

TEST_CASE("iso_2022_jp encode-then-decode: ASCII and JIS X 0208", "[roundtrip::iso_2022_jp]") {
    // Encoder emits full escape sequences; decoder consumes them and yields codepoints
    std::vector<char32_t> cps{U'A', U'\x3000'};
    CHECK(encode_then_decode<codec::iso_2022_jp>(cps) == cps);
}

TEST_CASE("iso_2022_jp decode-then-encode: full escape sequence for U+3000", "[roundtrip::iso_2022_jp]") {
    // Encoding U+3000 yields: ESC $ B 0x21 0x21 ESC ( B (8 bytes, already reset to ASCII)
    // Decoding those 8 bytes: {U+3000}
    // Re-encoding: same 8 bytes
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x21', '\x1B', '\x28', '\x42'};
    CHECK(decode_then_encode<codec::iso_2022_jp>(bytes) == bytes);
}

// ---------------------------------------------------------------------------
// EUC-KR
// ---------------------------------------------------------------------------

TEST_CASE("euc_kr encode-then-decode: ASCII and Korean Hangul", "[roundtrip::euc_kr]") {
    // U+AC00 (가) -> 0xB0 0xA2; U+AC02 -> 0x81 0x41 (pointer 0)
    std::vector<char32_t> cps{U'A', U'\xAC00', U'\xAC02'};
    CHECK(encode_then_decode<codec::euc_kr>(cps) == cps);
}

TEST_CASE("euc_kr decode-then-encode: known byte sequences", "[roundtrip::euc_kr]") {
    // 'A' (0x41), U+AC00 (0xB0 0xA2), U+AC02 (0x81 0x41)
    std::vector<char> bytes{'\x41', '\xB0', '\xA2', '\x81', '\x41'};
    CHECK(decode_then_encode<codec::euc_kr>(bytes) == bytes);
}
