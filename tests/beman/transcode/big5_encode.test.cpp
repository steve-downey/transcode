// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/detail/tables/big5.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
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
std::vector<std::expected<char, beman::transcoding::whatwg::error>> collect_or_error(View&& v) {
    std::vector<std::expected<char, beman::transcoding::whatwg::error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

// ---------------------------------------------------------------------------
// whatwg::encode<whatwg::encoding::big5>
// ---------------------------------------------------------------------------

TEST_CASE("big5 encode ASCII 'A' -> 0x41", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("big5 encode ASCII NUL -> 0x00", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'\0'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x00');
}

TEST_CASE("big5 encode U+4E00 -> 0xA4 0x40 (pointer 5495)", "[transcoding::big5_encode]") {
    // lowest pointer for U+4E00 is 5495: lead=0xA4, trail=0x40
    std::vector<char32_t> cps{U'\x4E00'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xA4);
    CHECK(static_cast<unsigned char>(result[1]) == 0x40);
}

TEST_CASE("big5 encode U+00CA (HKSCS only, pointer 1137) -> '?'", "[transcoding::big5_encode]") {
    // U+00CA is only in the Hong Kong Supplementary Character Set rows, which
    // the WHATWG Big5 encoder leaves out: pointers below 5024 are excluded.
    std::vector<char32_t> cps{U'\x00CA'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("big5 encode U+00EA (HKSCS only, pointer 1168) -> '?'", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'\x00EA'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("big5 encode U+4FBB (in HKSCS and Big5) -> 0xCD 0xE7, the Big5 pointer", "[transcoding::big5_encode]") {
    // U+4FBB is at pointer 4258 (HKSCS) and 12065 (Big5); the first pointer
    // not below 5024 wins.
    std::vector<char32_t> cps{U'\x4FBB'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xCD);
    CHECK(static_cast<unsigned char>(result[1]) == 0xE7);
}

TEST_CASE("big5 encode unmapped codepoint U+0041+extra -> '?' (non-error)", "[transcoding::big5_encode]") {
    // U+0304 (combining macron) is not individually in the Big5 table
    std::vector<char32_t> cps{U'\x0304'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("big5 encode mixed ASCII and 2-byte", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'A', U'\x4E00'};
    auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0xA4);
    CHECK(static_cast<unsigned char>(result[2]) == 0x40);
}

TEST_CASE("big5 encode pipe syntax", "[transcoding::big5_encode]") {
    std::vector<char32_t> cps{U'A'};
    std::vector<char>     result;
    for (char c : cps | whatwg::encode<whatwg::encoding::big5>)
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '\x41');
}

TEST_CASE("big5 encode consteval ASCII", "[transcoding::big5_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a = []() consteval {
        std::array<char32_t, 1> cps{U'A'};
        std::span<char32_t>     sp(cps);
        return *(sp | whatwg::encode<whatwg::encoding::big5>).begin();
    };
    CHECK(constify(encode_a()) == '\x41');
}

// ---------------------------------------------------------------------------
// whatwg::encode_or_error<whatwg::encoding::big5>
// ---------------------------------------------------------------------------

TEST_CASE("big5 or_error encode ASCII 'A'", "[transcoding::big5_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == '\x41');
}

TEST_CASE("big5 or_error encode U+4E00", "[transcoding::big5_encode_or_error]") {
    std::vector<char32_t> cps{U'\x4E00'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::big5>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0xA4);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0x40);
}

TEST_CASE("big5 or_error encode unmapped -> unmapped_codepoint error", "[transcoding::big5_encode_or_error]") {
    // U+0304 (combining macron) is not in Big5 table
    std::vector<char32_t> cps{U'\x0304'};
    auto                  result = collect_or_error(cps | whatwg::encode_or_error<whatwg::encoding::big5>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg::error::unmapped_codepoint);
}

// ---------------------------------------------------------------------------
// The index Big5 pointer: HKSCS excluded, six last-pointer code points
// ---------------------------------------------------------------------------

namespace {

std::array<unsigned char, 2> big5_bytes(int pointer) {
    int offset = pointer % 157;
    return {static_cast<unsigned char>((pointer / 157) + 0x81),
            static_cast<unsigned char>(offset < 63 ? offset + 0x40 : offset + 0x62)};
}

// WHATWG's "index Big5 pointer", spelled out as the scans it is defined by.
int reference_big5_pointer(char32_t cp) {
    bool last =
        cp == U'\x2550' || cp == U'\x255E' || cp == U'\x2561' || cp == U'\x256A' || cp == U'\x5341' || cp == U'\x5345';
    int found = -1;
    for (int pointer = 5024; pointer < 19782; ++pointer) {
        if (detail::tables::big5[pointer] != cp)
            continue;
        found = pointer;
        if (!last)
            break;
    }
    return found;
}

} // namespace

TEST_CASE("big5 encode takes the last pointer for the six WHATWG exceptions", "[transcoding::big5_encode]") {
    // Each of these appears twice in the Big5 rows; the encoder uses the later one.
    struct expectation {
        char32_t      cp;
        unsigned char lead;
        unsigned char trail;
    };
    for (auto [cp, lead, trail] : {expectation{U'\x2550', 0xF9, 0xF9},
                                   expectation{U'\x255E', 0xF9, 0xE9},
                                   expectation{U'\x2561', 0xF9, 0xEB},
                                   expectation{U'\x256A', 0xF9, 0xEA},
                                   expectation{U'\x5341', 0xA4, 0x51},
                                   expectation{U'\x5345', 0xA4, 0xCA}}) {
        std::vector<char32_t> cps{cp};
        auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
        REQUIRE(result.size() == 2);
        CHECK(static_cast<unsigned char>(result[0]) == lead);
        CHECK(static_cast<unsigned char>(result[1]) == trail);
    }
}

TEST_CASE("big5 encode never produces an HKSCS lead byte", "[transcoding::big5_encode]") {
    for (char32_t cp : detail::tables::big5) {
        if (cp == 0)
            continue;
        std::vector<char32_t> cps{cp};
        auto                  result = collect(cps | whatwg::encode<whatwg::encoding::big5>);
        if (result.size() == 2)
            CHECK(static_cast<unsigned char>(result[0]) >= 0xA1);
    }
}

TEST_CASE("big5 encode agrees with the WHATWG index Big5 pointer for every code point", "[transcoding::big5_encode]") {
    for (char32_t cp : detail::tables::big5) {
        if (cp == 0)
            continue;
        std::vector<char32_t> cps{cp};
        auto                  result  = collect(cps | whatwg::encode<whatwg::encoding::big5>);
        int                   pointer = reference_big5_pointer(cp);
        if (pointer < 0) {
            REQUIRE(result.size() == 1);
            CHECK(result[0] == '?');
        } else {
            auto expected = big5_bytes(pointer);
            REQUIRE(result.size() == 2);
            CHECK(static_cast<unsigned char>(result[0]) == expected[0]);
            CHECK(static_cast<unsigned char>(result[1]) == expected[1]);
        }
    }
}

TEST_CASE("big5 encode then decode round-trips every encodable code point", "[transcoding::big5_encode]") {
    for (char32_t cp : detail::tables::big5) {
        if (cp == 0 || reference_big5_pointer(cp) < 0)
            continue;
        std::vector<char32_t> cps{cp};
        auto                  bytes = collect(cps | whatwg::encode<whatwg::encoding::big5>);
        std::vector<char32_t> back;
        for (char32_t c : bytes | whatwg::decode<whatwg::encoding::big5>)
            back.push_back(c);
        REQUIRE(back.size() == 1);
        CHECK(back[0] == cp);
    }
}

TEST_CASE("big5 encode consteval two-byte", "[transcoding::big5_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode = [](char32_t cp) consteval {
        std::array<char32_t, 1> cps{cp};
        std::span<char32_t>     sp(cps);
        std::array<char, 2>     out{};
        int                     n = 0;
        for (char c : sp | whatwg::encode<whatwg::encoding::big5>)
            out[static_cast<std::size_t>(n++)] = c;
        return out;
    };
    CHECK(constify(encode(U'\x4E00')) == std::array<char, 2>{'\xA4', '\x40'});
    CHECK(constify(encode(U'\x5341')) == std::array<char, 2>{'\xA4', '\x51'});
}
