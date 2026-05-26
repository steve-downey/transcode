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

// BLP-59001: Bloomberg Terminal character encoding.
// Full 256-entry table — NOT ASCII-compatible in the lower half.
// 0x00-0x1F: accented Latin (CP437-like), 0x20-0x7E: ASCII, 0x7F: Euro,
// 0x80-0xBF: vulgar fractions (64ths, PUA + standard Unicode),
// 0xC0-0xD3: index markers (PUA), 0xD4-0xDB: arrows,
// 0xDC-0xDF: finance symbols (PUA), 0xE0-0xFF: currency/math/Latin.
// clang-format off
inline constexpr std::array<char32_t, 256> blp_59001_table = {{
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5, // 0x00-0x0F
    0x00C9, 0x00C8, 0x00CC, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, // 0x10-0x1F
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, // 0x20-0x2F
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, // 0x30-0x3F
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, // 0x40-0x4F
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, // 0x50-0x5F
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, // 0x60-0x6F
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x20AC, // 0x70-0x7F
    0xE080, 0xE081, 0xE082, 0xE083, 0xE084, 0xE085, 0xE086, 0x215B, 0xE088, 0xE089, 0xE08A, 0xE08B, 0xE08C, 0xE08D, 0xE08E, 0x00BC, // 0x80-0x8F
    0xE090, 0xE091, 0xE092, 0xE093, 0xE094, 0xE095, 0xE096, 0x215C, 0xE098, 0xE099, 0xE09A, 0xE09B, 0xE09C, 0xE09D, 0xE09E, 0x00BD, // 0x90-0x9F
    0xE0A0, 0xE0A1, 0xE0A2, 0xE0A3, 0xE0A4, 0xE0A5, 0xE0A6, 0x215D, 0xE0A8, 0xE0A9, 0xE0AA, 0xE0AB, 0xE0AC, 0xE0AD, 0xE0AE, 0x00BE, // 0xA0-0xAF
    0xE0B0, 0xE0B1, 0xE0B2, 0xE0B3, 0xE0B4, 0xE0B5, 0xE0B6, 0x215E, 0xE0B8, 0xE0B9, 0xE0BA, 0xE0BB, 0xE0BC, 0xE0BD, 0xE0BE, 0x00D7, // 0xB0-0xBF
    0xE0C0, 0xE0C1, 0xE0C2, 0xE0C3, 0xE0C4, 0xE0C5, 0xE0C6, 0xE0C7, 0xE0C8, 0xE0C9, 0xE0CA, 0xE0CB, 0xE0CC, 0xE0CD, 0xE0CE, 0xE0CF, // 0xC0-0xCF
    0xE0D0, 0xE0D1, 0xE0D2, 0xE0D3, 0x2191, 0x2193, 0x2190, 0x2192, 0x2197, 0x2199, 0x2196, 0x2198, 0xE0DC, 0xE0DD, 0xE0DE, 0xE0DF, // 0xD0-0xDF
    0x00A3, 0x00A5, 0x20A3, 0x00D2, 0x00D9, 0x00B1, 0x2260, 0x2248, 0x2264, 0x2265, 0x00D5, 0x00C1, 0x00CD, 0x2122, 0x00A9, 0x00AE, // 0xE0-0xEF
    0x00D4, 0x2713, 0x00D3, 0x00DA, 0x00C2, 0x00CA, 0x00F5, 0x00C0, 0x00D1, 0x00BF, 0x00A1, 0x00AB, 0x00BB, 0x00E3, 0x00C3, 0x00DF, // 0xF0-0xFF
}};
// clang-format on

using blp_59001 = full_table_codec<blp_59001_table>;

static_assert(decode_codec<blp_59001>);
static_assert(random_access_decode_codec_type<blp_59001>);
static_assert(encode_codec<blp_59001>);

// ---------------------------------------------------------------------------
// Decode tests
// ---------------------------------------------------------------------------

TEST_CASE("blp59001: byte 0x00 is C-cedilla, not NUL", "[blp59001]") {
    std::array<unsigned char, 1> src{0x00};
    auto view = src | decode(blp_59001{});
    CHECK(*view.begin() == U'\x00C7'); // Ç
}

TEST_CASE("blp59001: ASCII range 0x20-0x7E is standard", "[blp59001]") {
    std::string src = "Hello, World!";
    std::u32string result;
    for (char32_t cp : std::span<const char>(src) | decode(blp_59001{}))
        result.push_back(cp);
    CHECK(result == U"Hello, World!");
}

TEST_CASE("blp59001: byte 0x7F is Euro sign", "[blp59001]") {
    std::array<unsigned char, 1> src{0x7F};
    auto view = src | decode(blp_59001{});
    CHECK(*view.begin() == U'\x20AC'); // €
}

TEST_CASE("blp59001: fraction 1/8 at 0x87", "[blp59001]") {
    std::array<unsigned char, 1> src{0x87};
    auto view = src | decode(blp_59001{});
    CHECK(*view.begin() == U'\x215B'); // ⅛
}

TEST_CASE("blp59001: fraction 1/4 at 0x8F", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0x8F)) == U'\x00BC'); // ¼
}

TEST_CASE("blp59001: fraction 1/2 at 0x9F", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0x9F)) == U'\x00BD'); // ½
}

TEST_CASE("blp59001: fraction 3/4 at 0xAF", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0xAF)) == U'\x00BE'); // ¾
}

TEST_CASE("blp59001: PUA fractions in 64ths", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0x80)) == U'\xE080'); // 1/64 (PUA)
    CHECK(constify(blp_59001{}.decode_byte(0x81)) == U'\xE081'); // 1/32 (PUA)
    CHECK(constify(blp_59001{}.decode_byte(0xBE)) == U'\xE0BE'); // 63/64 (PUA)
}

TEST_CASE("blp59001: multiplication sign at 0xBF", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0xBF)) == U'\x00D7'); // ×
}

TEST_CASE("blp59001: arrows", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0xD4)) == U'\x2191'); // ↑
    CHECK(constify(blp_59001{}.decode_byte(0xD5)) == U'\x2193'); // ↓
    CHECK(constify(blp_59001{}.decode_byte(0xD6)) == U'\x2190'); // ←
    CHECK(constify(blp_59001{}.decode_byte(0xD7)) == U'\x2192'); // →
}

TEST_CASE("blp59001: currency symbols", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0xE0)) == U'\x00A3'); // £
    CHECK(constify(blp_59001{}.decode_byte(0xE1)) == U'\x00A5'); // ¥
    CHECK(constify(blp_59001{}.decode_byte(0xE2)) == U'\x20A3'); // ₣
}

TEST_CASE("blp59001: check mark at 0xF1", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0xF1)) == U'\x2713'); // ✓
}

TEST_CASE("blp59001: math operators", "[blp59001]") {
    CHECK(constify(blp_59001{}.decode_byte(0xE5)) == U'\x00B1'); // ±
    CHECK(constify(blp_59001{}.decode_byte(0xE6)) == U'\x2260'); // ≠
    CHECK(constify(blp_59001{}.decode_byte(0xE7)) == U'\x2248'); // ≈
    CHECK(constify(blp_59001{}.decode_byte(0xE8)) == U'\x2264'); // ≤
    CHECK(constify(blp_59001{}.decode_byte(0xE9)) == U'\x2265'); // ≥
}

// ---------------------------------------------------------------------------
// Encode round-trip
// ---------------------------------------------------------------------------

TEST_CASE("blp59001: encode ASCII round-trip", "[blp59001]") {
    blp_59001 codec;
    auto      r = codec.encode_one(U'A');
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x41);
}

TEST_CASE("blp59001: encode C-cedilla round-trips to 0x00", "[blp59001]") {
    blp_59001 codec;
    auto      r = codec.encode_one(U'\x00C7'); // Ç
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x00);
}

TEST_CASE("blp59001: encode Euro round-trips to 0x7F", "[blp59001]") {
    blp_59001 codec;
    auto      r = codec.encode_one(U'\x20AC'); // €
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x7F);
}

TEST_CASE("blp59001: encode PUA fraction round-trips", "[blp59001]") {
    blp_59001 codec;
    auto      r = codec.encode_one(U'\xE080'); // PUA 1/64
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x80);
}

TEST_CASE("blp59001: encode unmapped codepoint fails", "[blp59001]") {
    blp_59001 codec;
    auto      r = codec.encode_one(U'\x4E2D'); // 中 — not in BLP-59001
    CHECK(r.is_error);
}

// ---------------------------------------------------------------------------
// Pipe decode of a mixed BLP-59001 message
// ---------------------------------------------------------------------------

TEST_CASE("blp59001: decode mixed message via pipe", "[blp59001]") {
    // "IBM" then 0xD4 (↑) then "3.5" then 0x8F (¼)
    std::array<unsigned char, 8> src{0x49, 0x42, 0x4D, 0xD4, 0x33, 0x2E, 0x35, 0x8F};
    std::u32string result;
    for (char32_t cp : src | decode(blp_59001{}))
        result.push_back(cp);
    CHECK(result == U"IBM\x2191""3.5\x00BC");
}

// ---------------------------------------------------------------------------
// Random-access view
// ---------------------------------------------------------------------------

TEST_CASE("blp59001: random-access indexing", "[blp59001]") {
    std::array<unsigned char, 4> src{0x00, 0x41, 0x7F, 0xD4};
    auto                         view = src | decode(blp_59001{});
    auto                         it   = view.begin();
    CHECK(it[0] == U'\x00C7'); // Ç
    CHECK(it[1] == U'A');
    CHECK(it[2] == U'\x20AC'); // €
    CHECK(it[3] == U'\x2191'); // ↑
    CHECK(view.size() == 4);
}
