// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <expected>
#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char> collect(View&& v) {
    std::vector<char> result;
    for (char b : v)
        result.push_back(b);
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
// whatwg_encode<codec::utf_16be>
// ---------------------------------------------------------------------------

TEST_CASE("utf16be encode BMP 'A' (2 bytes)", "[transcoding::utf16be_encode]") {
    // U+0041 -> 0x00 0x41 in UTF-16BE
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x00');
    CHECK(result[1] == '\x41');
}

TEST_CASE("utf16be encode BMP U+00E9 (2 bytes)", "[transcoding::utf16be_encode]") {
    // U+00E9 -> 0x00 0xE9 in UTF-16BE
    std::vector<char32_t> cps{U'\x00E9'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x00');
    CHECK(result[1] == '\xE9');
}

TEST_CASE("utf16be encode supplementary U+1F600 (4 bytes)", "[transcoding::utf16be_encode]") {
    // U+1F600: high=0xD83D, low=0xDE00
    // UTF-16BE: D8 3D DE 00
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16be>);
    REQUIRE(result.size() == 4);
    CHECK(static_cast<unsigned char>(result[0]) == 0xD8);
    CHECK(static_cast<unsigned char>(result[1]) == 0x3D);
    CHECK(static_cast<unsigned char>(result[2]) == 0xDE);
    CHECK(static_cast<unsigned char>(result[3]) == 0x00);
}

TEST_CASE("utf16be encode surrogate codepoint yields U+FFFD bytes", "[transcoding::utf16be_encode]") {
    // U+D800 (surrogate) -> U+FFFD in UTF-16BE: 0xFF 0xFD
    std::vector<char32_t> cps{U'\xD800'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xFF);
    CHECK(static_cast<unsigned char>(result[1]) == 0xFD);
}

TEST_CASE("utf16be encode multiple BMP codepoints", "[transcoding::utf16be_encode]") {
    // 'H' 'i' = U+0048 U+0069
    std::vector<char32_t> cps{U'H', U'i'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16be>);
    REQUIRE(result.size() == 4);
    CHECK(result[0] == '\x00');
    CHECK(result[1] == '\x48');
    CHECK(result[2] == '\x00');
    CHECK(result[3] == '\x69');
}

TEST_CASE("utf16be encode pipe syntax with span", "[transcoding::utf16be_encode]") {
    std::vector<char32_t>     cps{U'A'};
    std::span<const char32_t> sp(cps);
    auto                      result = collect(sp | whatwg_encode<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x00');
    CHECK(result[1] == '\x41');
}

TEST_CASE("utf16be encode consteval", "[transcoding::utf16be_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_a_first_byte = []() consteval {
        constexpr char32_t        cps[] = {U'A'};
        std::span<const char32_t> sp(cps, 1);
        return *(sp | whatwg_encode<codec::utf_16be>).begin();
    };
    CHECK(constify(encode_a_first_byte()) == '\x00');
}

// ---------------------------------------------------------------------------
// whatwg_encode_or_error<codec::utf_16be>
// ---------------------------------------------------------------------------

TEST_CASE("utf16be or_error: valid BMP 'A'", "[transcoding::utf16be_encode_or_error]") {
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    REQUIRE(result[1].has_value());
    CHECK(result[0].value() == '\x00');
    CHECK(result[1].value() == '\x41');
}

TEST_CASE("utf16be or_error: surrogate yields error", "[transcoding::utf16be_encode_or_error]") {
    std::vector<char32_t> cps{U'\xD800'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
}

TEST_CASE("utf16be or_error: supplementary U+1F600 (4 bytes)", "[transcoding::utf16be_encode_or_error]") {
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_16be>);
    REQUIRE(result.size() == 4);
    for (auto& r : result)
        REQUIRE(r.has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0xD8);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0x3D);
    CHECK(static_cast<unsigned char>(result[2].value()) == 0xDE);
    CHECK(static_cast<unsigned char>(result[3].value()) == 0x00);
}

// ---------------------------------------------------------------------------
// whatwg_encode<codec::utf_16le>
// ---------------------------------------------------------------------------

TEST_CASE("utf16le encode BMP 'A' (2 bytes)", "[transcoding::utf16le_encode]") {
    // U+0041 -> 0x41 0x00 in UTF-16LE
    std::vector<char32_t> cps{U'A'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16le>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\x41');
    CHECK(result[1] == '\x00');
}

TEST_CASE("utf16le encode BMP U+00E9 (2 bytes)", "[transcoding::utf16le_encode]") {
    // U+00E9 -> 0xE9 0x00 in UTF-16LE
    std::vector<char32_t> cps{U'\x00E9'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16le>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xE9');
    CHECK(result[1] == '\x00');
}

TEST_CASE("utf16le encode supplementary U+1F600 (4 bytes)", "[transcoding::utf16le_encode]") {
    // U+1F600: high=0xD83D, low=0xDE00
    // UTF-16LE: 3D D8 00 DE
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16le>);
    REQUIRE(result.size() == 4);
    CHECK(static_cast<unsigned char>(result[0]) == 0x3D);
    CHECK(static_cast<unsigned char>(result[1]) == 0xD8);
    CHECK(static_cast<unsigned char>(result[2]) == 0x00);
    CHECK(static_cast<unsigned char>(result[3]) == 0xDE);
}

TEST_CASE("utf16le encode surrogate codepoint yields U+FFFD bytes", "[transcoding::utf16le_encode]") {
    // U+FFFD in UTF-16LE: 0xFD 0xFF
    std::vector<char32_t> cps{U'\xD800'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16le>);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xFD);
    CHECK(static_cast<unsigned char>(result[1]) == 0xFF);
}

TEST_CASE("utf16le encode multiple BMP codepoints", "[transcoding::utf16le_encode]") {
    // 'H' 'i' in UTF-16LE
    std::vector<char32_t> cps{U'H', U'i'};
    auto                  result = collect(cps | whatwg_encode<codec::utf_16le>);
    REQUIRE(result.size() == 4);
    CHECK(result[0] == '\x48');
    CHECK(result[1] == '\x00');
    CHECK(result[2] == '\x69');
    CHECK(result[3] == '\x00');
}

TEST_CASE("utf16le or_error: surrogate yields error", "[transcoding::utf16le_encode_or_error]") {
    std::vector<char32_t> cps{U'\xDFFF'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_16le>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
}

TEST_CASE("utf16le or_error: supplementary U+1F600 (4 bytes)", "[transcoding::utf16le_encode_or_error]") {
    std::vector<char32_t> cps{U'\x1F600'};
    auto                  result = collect_or_error(cps | whatwg_encode_or_error<codec::utf_16le>);
    REQUIRE(result.size() == 4);
    for (auto& r : result)
        REQUIRE(r.has_value());
    CHECK(static_cast<unsigned char>(result[0].value()) == 0x3D);
    CHECK(static_cast<unsigned char>(result[1].value()) == 0xD8);
    CHECK(static_cast<unsigned char>(result[2].value()) == 0x00);
    CHECK(static_cast<unsigned char>(result[3].value()) == 0xDE);
}
