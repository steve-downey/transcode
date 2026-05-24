// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <array>
#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char> collect(View&& v) {
    std::vector<char> result;
    for (char c : v)
        result.push_back(c);
    return result;
}

template <typename View>
std::vector<std::expected<char, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg_encode<codec::iso_2022_jp>
// ---------------------------------------------------------------------------

TEST_CASE("iso_2022_jp encode ASCII 'A' -> 0x41 (no escape)", "[transcoding::iso_2022_jp_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("iso_2022_jp encode ASCII NUL -> 0x00", "[transcoding::iso_2022_jp_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("iso_2022_jp encode U+00A5 (YEN SIGN) -> ESC ( J 0x5C",
          "[transcoding::iso_2022_jp_encode]") {
    // U+00A5 in Roman state is encoded as 0x5C; switching to Roman: ESC ( J = 1B 28 4A
    std::vector<char32_t> cps{U'\x00A5'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 4);
    CHECK(result[0] == '\x1B');
    CHECK(result[1] == '\x28');
    CHECK(result[2] == '\x4A');
    CHECK(result[3] == '\x5C');
}

TEST_CASE("iso_2022_jp encode U+203E (OVERLINE) -> ESC ( J 0x7E",
          "[transcoding::iso_2022_jp_encode]") {
    std::vector<char32_t> cps{U'\x203E'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 4);
    CHECK(result[0] == '\x1B');
    CHECK(result[1] == '\x28');
    CHECK(result[2] == '\x4A');
    CHECK(result[3] == '\x7E');
}

TEST_CASE("iso_2022_jp encode U+3000 (JIS X 0208 pointer 0) -> ESC $ B 0x21 0x21",
          "[transcoding::iso_2022_jp_encode]") {
    // First time in JIS0208: ESC $ B = 1B 24 42, then lead=0x21 trail=0x21
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 5);
    CHECK(result[0] == '\x1B');
    CHECK(result[1] == '\x24');
    CHECK(result[2] == '\x42');
    CHECK(result[3] == '\x21');
    CHECK(result[4] == '\x21');
}

TEST_CASE("iso_2022_jp encode U+4E00 (一) after U+3000: 0x30 0x6C (already in JIS0208)",
          "[transcoding::iso_2022_jp_encode]") {
    // U+3000 switches to JIS0208, U+4E00 (pointer 1485) stays in JIS0208 -> 0x30 0x6C
    std::vector<char32_t> cps{U'\x3000', U'\x4E00'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    // U+3000: ESC $ B 0x21 0x21 = 5 bytes; U+4E00: 0x30 0x6C = 2 bytes; total 7
    REQUIRE(result.size() == 7);
    CHECK(result[0] == '\x1B');  // ESC $ B for U+3000
    CHECK(result[1] == '\x24');
    CHECK(result[2] == '\x42');
    CHECK(result[3] == '\x21');
    CHECK(result[4] == '\x21');
    CHECK(result[5] == '\x30');  // U+4E00 lead
    CHECK(result[6] == '\x6C');  // U+4E00 trail
}

TEST_CASE("iso_2022_jp encode unmapped codepoint U+1F600 -> '?'",
          "[transcoding::iso_2022_jp_encode]") {
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("iso_2022_jp encode: end of stream in JIS0208 state emits ESC ( B",
          "[transcoding::iso_2022_jp_encode]") {
    // U+3000 switches to JIS0208; end of stream must emit ESC ( B = 1B 28 42
    // Total: ESC $ B 0x21 0x21 ESC ( B = 8 bytes
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    // The final ESC ( B comes at end-of-stream when still in JIS0208 state
    // But wait: U+3000 alone already produces ESC$B+lead+trail. Does the stream
    // also emit the return-to-ASCII? Yes — the encoder must reset to ASCII on stream end.
    // Actually re-reading the spec: the WHATWG encoder emits return-to-ASCII at end.
    // So encoding {U+3000} should produce: ESC$B 0x21 0x21 ESC(B = 8 bytes
    REQUIRE(result.size() == 8);
    CHECK(result[0] == '\x1B');
    CHECK(result[1] == '\x24');
    CHECK(result[2] == '\x42');
    CHECK(result[3] == '\x21');
    CHECK(result[4] == '\x21');
    CHECK(result[5] == '\x1B');
    CHECK(result[6] == '\x28');
    CHECK(result[7] == '\x42');
}

TEST_CASE("iso_2022_jp encode mixed: 'A' + U+3000 + 'B'",
          "[transcoding::iso_2022_jp_encode]") {
    // 'A' (ASCII, no escape) + U+3000 (ESC$B 0x21 0x21) + 'B' (needs ESC(B first)
    // Result: 0x41 1B 24 42 0x21 0x21 1B 28 42 0x42 + trailing ESC(B for reset
    // Wait: after 'B' we return to ASCII, and 'B' needs ESC(B to switch to ASCII
    // So: 0x41 [ESC$B 0x21 0x21] [ESC(B 0x42] = 1 + 5 + 4 = 10 bytes
    // No trailing ESC(B because we're already in ASCII after emitting 'B'
    std::vector<char32_t> cps{U'A', U'\x3000', U'B'};
    auto                  result = collect(cps | whatwg_encode<codec::iso_2022_jp>);
    REQUIRE(result.size() == 10);
    CHECK(result[0] == '\x41');   // 'A'
    CHECK(result[1] == '\x1B');   // ESC $ B for U+3000
    CHECK(result[2] == '\x24');
    CHECK(result[3] == '\x42');
    CHECK(result[4] == '\x21');
    CHECK(result[5] == '\x21');
    CHECK(result[6] == '\x1B');   // ESC ( B for 'B'
    CHECK(result[7] == '\x28');
    CHECK(result[8] == '\x42');
    CHECK(result[9] == '\x42');   // 'B'
}

TEST_CASE("iso_2022_jp encode pipe syntax", "[transcoding::iso_2022_jp_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg_encode<codec::iso_2022_jp>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("iso_2022_jp encode consteval ASCII", "[transcoding::iso_2022_jp_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg_encode<codec::iso_2022_jp>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::iso_2022_jp>
// ---------------------------------------------------------------------------

TEST_CASE("iso_2022_jp or_error encode ASCII 'A'", "[transcoding::iso_2022_jp_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("iso_2022_jp or_error encode U+3000 -> ESC $ B 0x21 0x21 [+ ESC ( B at end]",
          "[transcoding::iso_2022_jp_encode_or_error]") {
    std::vector<char32_t> cps{U'\x3000'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::iso_2022_jp>);
    // ESC $ B 0x21 0x21 ESC ( B = 8 bytes
    REQUIRE(result.size() == 8);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x1B');
    REQUIRE(result[4].has_value());
    CHECK(result[4].value() == '\x21');
}

TEST_CASE("iso_2022_jp or_error encode unmapped yields error",
          "[transcoding::iso_2022_jp_encode_or_error]") {
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::iso_2022_jp>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::unmapped_codepoint);
}
