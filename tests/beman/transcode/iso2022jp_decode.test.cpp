// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char32_t> collect(View&& v) {
    std::vector<char32_t> result;
    for (char32_t cp : v)
        result.push_back(cp);
    return result;
}

template <typename View>
std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg_decode<codec::iso_2022_jp>
// ---------------------------------------------------------------------------

TEST_CASE("iso_2022_jp decode ASCII passthrough 'A' (0x41)", "[transcoding::iso_2022_jp]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("iso_2022_jp decode ASCII NUL (0x00)", "[transcoding::iso_2022_jp]") {
    std::vector<char> bytes{'\x00'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\0');
}

TEST_CASE("iso_2022_jp decode JIS X 0208 ESC $B 0x21 0x21 -> U+3000 (pointer 0)", "[transcoding::iso_2022_jp]") {
    // ESC $ B = 0x1B 0x24 0x42 switches to Lead_Byte (JIS X 0208) state
    // Lead=0x21, Trail=0x21: pointer = (0x21-0x21)*94 + (0x21-0x21) = 0 -> U+3000
    // Per WHATWG: stream ending in Lead_Byte state is finished (no extra U+FFFD)
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x21'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x3000');
}

TEST_CASE("iso_2022_jp decode JIS X 0208 ESC $B -> U+4E00 (一)", "[transcoding::iso_2022_jp]") {
    // pointer 1485: lead_offset=15 -> 0x21+15=0x30, trail_offset=75 -> 0x21+75=0x6C
    // Per WHATWG: stream ending in Lead_Byte state is finished (no extra U+FFFD)
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x30', '\x6C'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x4E00');
}

TEST_CASE("iso_2022_jp decode ESC (J 0x5C -> U+00A5 (YEN SIGN)", "[transcoding::iso_2022_jp]") {
    // ESC ( J = 0x1B 0x28 0x4A switches to Roman state
    // In Roman state 0x5C -> U+00A5
    std::vector<char> bytes{'\x1B', '\x28', '\x4A', '\x5C'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x00A5');
}

TEST_CASE("iso_2022_jp decode ESC (J 0x7E -> U+203E (OVERLINE)", "[transcoding::iso_2022_jp]") {
    // In Roman state 0x7E -> U+203E
    std::vector<char> bytes{'\x1B', '\x28', '\x4A', '\x7E'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\x203E');
}

TEST_CASE("iso_2022_jp decode ESC (B 0x41 -> U+0041 (ASCII state)", "[transcoding::iso_2022_jp]") {
    // ESC ( B = 0x1B 0x28 0x42 switches back to ASCII state
    std::vector<char> bytes{'\x1B', '\x28', '\x42', '\x41'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("iso_2022_jp decode ESC (I 0x21 -> U+FF61 (half-width katakana)", "[transcoding::iso_2022_jp]") {
    // ESC ( I = 0x1B 0x28 0x49 switches to Katakana state
    // WHATWG Katakana: bytes 0x21-0x5F map to U+FF61-U+FF9F (7-bit range)
    // 0x21 -> U+FF61 + (0x21 - 0x21) = U+FF61
    std::vector<char> bytes{'\x1B', '\x28', '\x49', '\x21'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFF61');
}

TEST_CASE("iso_2022_jp decode invalid JIS0208 lead byte (0x80) -> U+FFFD", "[transcoding::iso_2022_jp]") {
    // After ESC $ B, 0x80 is not in 0x21-0x7E -> error
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("iso_2022_jp decode invalid JIS0208 trail byte -> U+FFFD", "[transcoding::iso_2022_jp]") {
    // After ESC $ B 0x21 (valid lead), 0x80 is not in 0x21-0x7E -> error
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x80'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("iso_2022_jp decode end-of-stream in Lead_Byte state -> U+FFFD", "[transcoding::iso_2022_jp]") {
    // ESC $ B then 0x21 (stored as lead) then end -> truncated
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("iso_2022_jp decode escape at end-of-stream -> U+FFFD", "[transcoding::iso_2022_jp]") {
    // Lone ESC at end of input -> U+FFFD
    std::vector<char> bytes{'\x1B'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("iso_2022_jp decode mixed: ASCII then JIS0208 then ASCII", "[transcoding::iso_2022_jp]") {
    // 'A' + ESC $ B 0x21 0x21 (U+3000) + ESC ( B 'B'
    // Per WHATWG: ESC in Lead_Byte state silently transitions to escape_start (no U+FFFD)
    std::vector<char> bytes{'\x41', '\x1B', '\x24', '\x42', '\x21', '\x21', '\x1B', '\x28', '\x42', '\x42'};
    auto              result = collect(bytes | whatwg_decode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x3000');
    CHECK(result[2] == U'B');
}

TEST_CASE("iso_2022_jp decode pipe syntax with span", "[transcoding::iso_2022_jp]") {
    std::vector<char>     bytes{'\x41'};
    std::span<const char> sp(bytes);
    std::vector<char32_t> result;
    for (char32_t cp : sp | whatwg_decode<codec::iso_2022_jp>)
        result.push_back(cp);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'A');
}

TEST_CASE("iso_2022_jp decode consteval ASCII", "[transcoding::iso_2022_jp]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_a = []() consteval {
        constexpr char        bytes[] = {'\x41'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::iso_2022_jp>).begin();
    };
    CHECK(constify(decode_a()) == U'A');
}

// ---------------------------------------------------------------------------
// whatwg_decode_or_error<codec::iso_2022_jp>
// ---------------------------------------------------------------------------

TEST_CASE("iso_2022_jp or_error: ASCII passthrough", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("iso_2022_jp or_error: JIS X 0208 ESC $B 0x21 0x21 -> U+3000", "[transcoding::iso_2022_jp_or_error]") {
    // Per WHATWG: stream ending in Lead_Byte state is finished (no extra error)
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x21'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x3000');
}

TEST_CASE("iso_2022_jp or_error: invalid lead byte yields error", "[transcoding::iso_2022_jp_or_error]") {
    // After ESC $ B, 0x80 is invalid -> invalid_byte error
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("iso_2022_jp or_error: invalid trail byte yields error", "[transcoding::iso_2022_jp_or_error]") {
    // After ESC $ B 0x21, 0x80 is invalid trail -> invalid_byte error
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("iso_2022_jp or_error: truncated in Lead_Byte state yields error", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

// Coverage: EOS while in Escape_Start state (state=5)
TEST_CASE("iso_2022_jp or_error: EOS in Escape_Start state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

// Coverage: EOS while in Escape state (state=6) — pending lead byte
// After truncated escape, the pending byte (0x24='$') is re-processed as ASCII.
TEST_CASE("iso_2022_jp or_error: EOS in Escape state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x24'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'$');
}

// Coverage: EOS in Lead_Byte state via default case (just ESC $B, no character)
TEST_CASE("iso_2022_jp or_error: EOS after entering Lead_Byte state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x24', '\x42'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    CHECK(result.empty());
}

// Coverage: invalid byte in Escape_Start state (not 0x24 or 0x28)
// The invalid byte is pushed to pending and re-processed as ASCII.
TEST_CASE("iso_2022_jp or_error: invalid Escape_Start byte", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x30'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'0');
}

// Coverage: invalid second escape byte in Escape state (state=6, lead=0x28, byte=0x30)
// Both bytes (lead=0x28 and byte=0x30) are pushed to pending and re-processed.
TEST_CASE("iso_2022_jp or_error: invalid Escape byte", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x28', '\x30'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 3);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'(');
    REQUIRE(result[2].has_value());
    CHECK(result[2].value() == U'0');
}

// Coverage: SO (0x0E) in ASCII state
TEST_CASE("iso_2022_jp or_error: SO byte yields error", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x0E'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// Coverage: Roman state (state=1) — 0x5C → U+00A5, 0x7E → U+203E, 'A' → U+0041
TEST_CASE("iso_2022_jp or_error: Roman state special chars", "[transcoding::iso_2022_jp_or_error]") {
    // ESC ( J → Roman state, then 0x5C, 0x7E, and 'A'
    std::vector<char> bytes{'\x1B', '\x28', '\x4A', '\x5C', '\x7E', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x00A5');
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'\x203E');
    REQUIRE(result[2].has_value());
    CHECK(result[2].value() == U'A');
}

// Coverage: Katakana state (state=2) — ESC ( I then 0x21 → U+FF61
TEST_CASE("iso_2022_jp or_error: Katakana state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x28', '\x49', '\x21'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\xFF61');
}

// Coverage: high byte (>= 0x80) in ASCII state yields error
TEST_CASE("iso_2022_jp or_error: high byte in ASCII state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// Coverage: high byte in Roman state yields error
TEST_CASE("iso_2022_jp or_error: high byte in Roman state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x28', '\x4A', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// Coverage: Katakana byte out of range yields error
TEST_CASE("iso_2022_jp or_error: Katakana out-of-range byte", "[transcoding::iso_2022_jp_or_error]") {
    // ESC ( I then 0x60 — outside [0x21, 0x5F]
    std::vector<char> bytes{'\x1B', '\x28', '\x49', '\x60'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// Coverage: escape in Lead_Byte state (state=3, byte=0x1B → state=5)
// The first ESC $B sets output_flag=true. The second ESC (B triggers
// output_flag error before switching back to ASCII.
TEST_CASE("iso_2022_jp or_error: escape in Lead_Byte state", "[transcoding::iso_2022_jp_or_error]") {
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x1B', '\x28', '\x42', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'A');
}

// Coverage: escape in Trail_Byte state (state=4, byte=0x1B → state=5)
TEST_CASE("iso_2022_jp or_error: escape in Trail_Byte state", "[transcoding::iso_2022_jp_or_error]") {
    // ESC $B 0x21 → state=4 (trail byte), then ESC (B → back to ASCII, then 'A'
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x1B', '\x28', '\x42', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'A');
}

// Coverage: output_flag is reset after decoding a character, so a subsequent
// escape does NOT trigger output_flag error.
TEST_CASE("iso_2022_jp or_error: escape after decode has no output_flag error",
          "[transcoding::iso_2022_jp_or_error]") {
    // ESC $B 0x21 0x21 → U+3000 (trail byte resets output_flag=false)
    // Then ESC (B → no error (output_flag was false), then 'A'
    std::vector<char> bytes{'\x1B', '\x24', '\x42', '\x21', '\x21', '\x1B', '\x28', '\x42', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'\x3000');
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == U'A');
}

// Coverage: _or_error BOM stripping for UTF-16BE
TEST_CASE("utf16be or_error: BOM is stripped", "[transcoding::utf16be_or_error]") {
    std::vector<char> bytes{'\xFE', '\xFF', '\x00', '\x41'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}
