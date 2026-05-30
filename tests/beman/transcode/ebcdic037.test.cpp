// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/decode_view.hpp>

#include <beman/transcode/detail/table_codec.hpp>
#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

// IBM Code Page 037 (US/Canada EBCDIC).
// Full 256-entry table — letters are at 0x81-0x89 (a-i), 0x91-0x99 (j-r),
// 0xA2-0xA9 (s-z), 0xC1-0xC9 (A-I), 0xD1-0xD9 (J-R), 0xE2-0xE9 (S-Z),
// digits at 0xF0-0xF9. Punctuation is scattered throughout.
// clang-format off
inline constexpr std::array<char32_t, 256> ebcdic_037_table = {{
    0x0000, 0x0001, 0x0002, 0x0003, 0x009C, 0x0009, 0x0086, 0x007F, 0x0097, 0x008D, 0x008E, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, // 0x00-0x0F
    0x0010, 0x0011, 0x0012, 0x0013, 0x009D, 0x0085, 0x0008, 0x0087, 0x0018, 0x0019, 0x0092, 0x008F, 0x001C, 0x001D, 0x001E, 0x001F, // 0x10-0x1F
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x000A, 0x0017, 0x001B, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x0005, 0x0006, 0x0007, // 0x20-0x2F
    0x0090, 0x0091, 0x0016, 0x0093, 0x0094, 0x0095, 0x0096, 0x0004, 0x0098, 0x0099, 0x009A, 0x009B, 0x0014, 0x0015, 0x009E, 0x001A, // 0x30-0x3F
    0x0020, 0x00A0, 0x00E2, 0x00E4, 0x00E0, 0x00E1, 0x00E3, 0x00E5, 0x00E7, 0x00F1, 0x00A2, 0x002E, 0x003C, 0x0028, 0x002B, 0x007C, // 0x40-0x4F
    0x0026, 0x00E9, 0x00EA, 0x00EB, 0x00E8, 0x00ED, 0x00EE, 0x00EF, 0x00EC, 0x00DF, 0x0021, 0x0024, 0x002A, 0x0029, 0x003B, 0x00AC, // 0x50-0x5F
    0x002D, 0x002F, 0x00C2, 0x00C4, 0x00C0, 0x00C1, 0x00C3, 0x00C5, 0x00C7, 0x00D1, 0x00A6, 0x002C, 0x0025, 0x005F, 0x003E, 0x003F, // 0x60-0x6F
    0x00F8, 0x00C9, 0x00CA, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x0060, 0x003A, 0x0023, 0x0040, 0x0027, 0x003D, 0x0022, // 0x70-0x7F
    0x00D8, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x00AB, 0x00BB, 0x00F0, 0x00FD, 0x00FE, 0x00B1, // 0x80-0x8F
    0x00B0, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x00AA, 0x00BA, 0x00E6, 0x00B8, 0x00C6, 0x00A4, // 0x90-0x9F
    0x00B5, 0x007E, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00A1, 0x00BF, 0x00D0, 0x00DD, 0x00DE, 0x00AE, // 0xA0-0xAF
    0x005E, 0x00A3, 0x00A5, 0x00B7, 0x00A9, 0x00A7, 0x00B6, 0x00BC, 0x00BD, 0x00BE, 0x005B, 0x005D, 0x00AF, 0x00A8, 0x00B4, 0x00D7, // 0xB0-0xBF
    0x007B, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00AD, 0x00F4, 0x00F6, 0x00F2, 0x00F3, 0x00F5, // 0xC0-0xCF
    0x007D, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x00B9, 0x00FB, 0x00FC, 0x00F9, 0x00FA, 0x00FF, // 0xD0-0xDF
    0x005C, 0x00F7, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00B2, 0x00D4, 0x00D6, 0x00D2, 0x00D3, 0x00D5, // 0xE0-0xEF
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x00B3, 0x00DB, 0x00DC, 0x00D9, 0x00DA, 0x009F, // 0xF0-0xFF
}};
// clang-format on

using ebcdic_037 = full_table_codec<ebcdic_037_table>;

static_assert(decode_codec<ebcdic_037>);
static_assert(random_access_decode_codec_type<ebcdic_037>);
static_assert(encode_codec<ebcdic_037>);

// ---------------------------------------------------------------------------
// Decode: EBCDIC letter positions
// ---------------------------------------------------------------------------

TEST_CASE("ebcdic037: lowercase letters a-i at 0x81-0x89", "[ebcdic037]") {
    std::array<unsigned char, 9> src{0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89};
    std::u32string               result;
    for (char32_t cp : src | decode(ebcdic_037{}))
        result.push_back(cp);
    CHECK(result == U"abcdefghi");
}

TEST_CASE("ebcdic037: lowercase letters j-r at 0x91-0x99", "[ebcdic037]") {
    std::array<unsigned char, 9> src{0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99};
    std::u32string               result;
    for (char32_t cp : src | decode(ebcdic_037{}))
        result.push_back(cp);
    CHECK(result == U"jklmnopqr");
}

TEST_CASE("ebcdic037: lowercase letters s-z at 0xA2-0xA9", "[ebcdic037]") {
    std::array<unsigned char, 8> src{0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
    std::u32string               result;
    for (char32_t cp : src | decode(ebcdic_037{}))
        result.push_back(cp);
    CHECK(result == U"stuvwxyz");
}

TEST_CASE("ebcdic037: uppercase letters A-I at 0xC1-0xC9", "[ebcdic037]") {
    std::array<unsigned char, 9> src{0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9};
    std::u32string               result;
    for (char32_t cp : src | decode(ebcdic_037{}))
        result.push_back(cp);
    CHECK(result == U"ABCDEFGHI");
}

TEST_CASE("ebcdic037: digits 0-9 at 0xF0-0xF9", "[ebcdic037]") {
    std::array<unsigned char, 10> src{0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9};
    std::u32string                result;
    for (char32_t cp : src | decode(ebcdic_037{}))
        result.push_back(cp);
    CHECK(result == U"0123456789");
}

TEST_CASE("ebcdic037: space at 0x40", "[ebcdic037]") { CHECK(constify(ebcdic_037{}.decode_byte(0x40)) == U' '); }

TEST_CASE("ebcdic037: punctuation positions", "[ebcdic037]") {
    CHECK(constify(ebcdic_037{}.decode_byte(0x4B)) == U'.');
    CHECK(constify(ebcdic_037{}.decode_byte(0x6B)) == U',');
    CHECK(constify(ebcdic_037{}.decode_byte(0x7A)) == U':');
    CHECK(constify(ebcdic_037{}.decode_byte(0x5A)) == U'!');
    CHECK(constify(ebcdic_037{}.decode_byte(0x6F)) == U'?');
}

// ---------------------------------------------------------------------------
// Decode a full EBCDIC "HELLO WORLD" message
// ---------------------------------------------------------------------------

TEST_CASE("ebcdic037: decode HELLO WORLD", "[ebcdic037]") {
    // H=0xC8, E=0xC5, L=0xD3, L=0xD3, O=0xD6, SP=0x40,
    // W=0xE6, O=0xD6, R=0xD9, L=0xD3, D=0xC4
    std::array<unsigned char, 11> src{0xC8, 0xC5, 0xD3, 0xD3, 0xD6, 0x40, 0xE6, 0xD6, 0xD9, 0xD3, 0xC4};
    std::u32string                result;
    for (char32_t cp : src | decode(ebcdic_037{}))
        result.push_back(cp);
    CHECK(result == U"HELLO WORLD");
}

// ---------------------------------------------------------------------------
// Encode round-trip
// ---------------------------------------------------------------------------

TEST_CASE("ebcdic037: encode 'A' -> 0xC1", "[ebcdic037]") {
    ebcdic_037 codec;
    auto       r = codec.encode_one(U'A');
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0xC1);
}

TEST_CASE("ebcdic037: encode 'a' -> 0x81", "[ebcdic037]") {
    ebcdic_037 codec;
    auto       r = codec.encode_one(U'a');
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x81);
}

TEST_CASE("ebcdic037: encode '0' -> 0xF0", "[ebcdic037]") {
    ebcdic_037 codec;
    auto       r = codec.encode_one(U'0');
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0xF0);
}

TEST_CASE("ebcdic037: encode space -> 0x40", "[ebcdic037]") {
    ebcdic_037 codec;
    auto       r = codec.encode_one(U' ');
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x40);
}

// ---------------------------------------------------------------------------
// Random-access indexing
// ---------------------------------------------------------------------------

TEST_CASE("ebcdic037: random-access view", "[ebcdic037]") {
    std::array<unsigned char, 5> src{0xC8, 0xC5, 0xD3, 0xD3, 0xD6}; // HELLO
    auto                         view = src | decode(ebcdic_037{});
    CHECK(view.size() == 5);
    auto it = view.begin();
    CHECK(it[0] == U'H');
    CHECK(it[1] == U'E');
    CHECK(it[2] == U'L');
    CHECK(it[3] == U'L');
    CHECK(it[4] == U'O');
}

// ---------------------------------------------------------------------------
// NUL handling: EBCDIC 0x00 maps to U+0000 (NUL)
// Note: full_table_codec treats table entry 0 as "unmapped" (error).
// For EBCDIC, byte 0x00 legitimately maps to NUL — this is a known
// limitation of the zero-means-unmapped convention. The decode_byte
// path returns U+FFFD for it, while decode_one returns is_error=true.
// ---------------------------------------------------------------------------

TEST_CASE("ebcdic037: byte 0x00 is NUL (table limitation)", "[ebcdic037]") {
    // 0x0000 in the table is indistinguishable from "unmapped" with the
    // zero-means-error convention. This documents the known edge case.
    CHECK(ebcdic_037{}.decode_byte(0x00) == U'\xFFFD');
}
