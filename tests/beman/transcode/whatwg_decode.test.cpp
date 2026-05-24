// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <vector>

namespace {
// Helper: collect all code points from a whatwg_decode view into a vector.
template <typename View>
std::vector<char32_t> collect(View&& v) {
    std::vector<char32_t> result;
    for (char32_t cp : v)
        result.push_back(cp);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_decode_view satisfies input_range", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    auto              view = bytes | whatwg_decode<codec::utf_8>;
    static_assert(std::ranges::input_range<decltype(view)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(view)>, char32_t>);
}

// All WHATWG codecs share the 7-bit ASCII base (U+0000–U+007F).
TEST_CASE("whatwg_decode 7-bit ASCII shared base", "[transcoding::whatwg_decode]") {
    std::vector<char> ascii{'H', 'e', 'l', 'l', 'o'};
    CHECK(collect(ascii | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
}

// Step 5: valid multi-byte UTF-8 sequences

TEST_CASE("whatwg_decode 2-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+00E9 é = 0xC3 0xA9
    std::vector<char> bytes{'\xC3', '\xA9'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'é'});
}

TEST_CASE("whatwg_decode 3-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+20AC € = 0xE2 0x82 0xAC
    std::vector<char> bytes{'\xE2', '\x82', '\xAC'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'€'});
}

TEST_CASE("whatwg_decode 4-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+1F600 😀 = 0xF0 0x9F 0x98 0x80
    std::vector<char> bytes{'\xF0', '\x9F', '\x98', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'\U0001F600'});
}

TEST_CASE("whatwg_decode mixed ASCII and multi-byte", "[transcoding::whatwg_decode]") {
    // "Hi" + é (0xC3 0xA9) + "!"
    std::vector<char> bytes{'H', 'i', '\xC3', '\xA9', '!'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'H', U'i', U'é', U'!'});
}

// Step 6: WHATWG-specific error replacement behavior

TEST_CASE("whatwg_decode invalid lead byte 0xFF", "[transcoding::whatwg_decode]") {
    // 0xFF is never valid in UTF-8; followed by ASCII 'A' which is re-processed.
    std::vector<char> bytes{'\xFF', 'A'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�', U'A'});
}

TEST_CASE("whatwg_decode 0xC0 pre-rejected continuation re-processed", "[transcoding::whatwg_decode]") {
    // WHATWG pre-rejects 0xC0 immediately (→ U+FFFD); 0x80 is then an
    // unexpected continuation byte → second U+FFFD.
    std::vector<char> bytes{'\xC0', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�', U'�'});
}

TEST_CASE("whatwg_decode bad continuation causes re-processing", "[transcoding::whatwg_decode]") {
    // 0xC3 starts a 2-byte sequence; '(' (0x28) is not a continuation byte.
    // Result: U+FFFD for the incomplete sequence, then '(' re-processed as ASCII.
    std::vector<char> bytes{'\xC3', '\x28'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�', U'('});
}

TEST_CASE("whatwg_decode surrogate codepoint rejected", "[transcoding::whatwg_decode]") {
    // U+D800 encoded as UTF-8: 0xED 0xA0 0x80 — continuation bytes are valid,
    // but the assembled codepoint is a surrogate.
    std::vector<char> bytes{'\xED', '\xA0', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�'});
}

TEST_CASE("whatwg_decode truncated sequence at end of input", "[transcoding::whatwg_decode]") {
    // 0xC3 is a 2-byte lead; no continuation follows → one U+FFFD.
    std::vector<char> bytes{'\xC3'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'�'});
}

TEST_CASE("whatwg_decode consteval 2-byte", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_two_byte = []() consteval {
        constexpr char        bytes[] = {'\xC3', '\xA9'};
        std::span<const char> sp(bytes, 2);
        return *(sp | whatwg_decode<codec::utf_8>).begin();
    };
    CHECK(constify(decode_two_byte()) == U'é');
}

// Step 14: codec::replacement tests

TEST_CASE("whatwg_decode replacement empty input", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{};
    CHECK(collect(bytes | whatwg_decode<codec::replacement>).empty());
}

TEST_CASE("whatwg_decode replacement single byte", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'X'};
    CHECK(collect(bytes | whatwg_decode<codec::replacement>) == std::vector<char32_t>{U'\xFFFD'});
}

TEST_CASE("whatwg_decode replacement multiple bytes yields one U+FFFD", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'H', 'e', 'l', 'l', 'o'};
    auto              result = collect(bytes | whatwg_decode<codec::replacement>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("whatwg_decode replacement consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_replacement = []() consteval {
        constexpr char        bytes[] = {'a', 'b', 'c'};
        std::span<const char> sp(bytes, 3);
        return *(sp | whatwg_decode<codec::replacement>).begin();
    };
    CHECK(constify(decode_replacement()) == U'\xFFFD');
}
