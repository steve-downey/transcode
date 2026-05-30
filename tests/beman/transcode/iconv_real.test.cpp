// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>
#include <catch2/catch_all.hpp>

#include <beman/transcode/iconv_transcode_or_error_view.hpp>

#include <array>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("real iconv UTF-8 to UTF-32LE ASCII", "[transcoding::iconv_real]") {
    // "Hi" in UTF-8 to UTF-32LE:
    // 'H' (U+0048) -> {0x48, 0x00, 0x00, 0x00}
    // 'i' (U+0069) -> {0x69, 0x00, 0x00, 0x00}
    std::vector<char>    input{'H', 'i'};
    std::array<char, 64> buf{};
    auto                 fns = make_real_iconv_fns();
    auto                 view =
        iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "UTF-8", "UTF-32LE", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    REQUIRE(output.size() == 8);
    CHECK(static_cast<unsigned char>(output[0]) == 0x48);
    CHECK(static_cast<unsigned char>(output[1]) == 0x00);
    CHECK(static_cast<unsigned char>(output[2]) == 0x00);
    CHECK(static_cast<unsigned char>(output[3]) == 0x00);
    CHECK(static_cast<unsigned char>(output[4]) == 0x69);
    CHECK(static_cast<unsigned char>(output[5]) == 0x00);
    CHECK(static_cast<unsigned char>(output[6]) == 0x00);
    CHECK(static_cast<unsigned char>(output[7]) == 0x00);
}

TEST_CASE("real iconv UTF-8 to UTF-8 identity", "[transcoding::iconv_real]") {
    std::string          input = "Hello";
    std::array<char, 64> buf{};
    auto                 fns = make_real_iconv_fns();
    auto view = iconv_transcode_view<iconv_functions, std::string>(input, fns, "UTF-8", "UTF-8", std::span(buf));
    std::string output;
    for (char c : view)
        output.push_back(c);
    CHECK(output == input);
}

TEST_CASE("iconv_transcode pipe syntax UTF-8 to UTF-32LE", "[transcoding::iconv_real]") {
    std::vector<char>    input{'H', 'i'};
    std::array<char, 64> buf{};
    std::vector<char>    output;
    for (char c : input | iconv_transcode("UTF-8", "UTF-32LE", std::span(buf)))
        output.push_back(c);
    REQUIRE(output.size() == 8);
    CHECK(static_cast<unsigned char>(output[0]) == 0x48);
    CHECK(static_cast<unsigned char>(output[1]) == 0x00);
    CHECK(static_cast<unsigned char>(output[2]) == 0x00);
    CHECK(static_cast<unsigned char>(output[3]) == 0x00);
    CHECK(static_cast<unsigned char>(output[4]) == 0x69);
    CHECK(static_cast<unsigned char>(output[5]) == 0x00);
    CHECK(static_cast<unsigned char>(output[6]) == 0x00);
    CHECK(static_cast<unsigned char>(output[7]) == 0x00);
}

TEST_CASE("iconv_transcode_or_error pipe syntax EILSEQ", "[transcoding::iconv_real]") {
    std::string          input = "Hi";
    std::array<char, 64> buf{};
    std::string          output;
    for (auto r : input | iconv_transcode_or_error("UTF-8", "UTF-8", std::span(buf))) {
        REQUIRE(r.has_value());
        output.push_back(r.value());
    }
    CHECK(output == input);
}

TEST_CASE("real iconv E2BIG: long input with small buffer", "[transcoding::iconv_real]") {
    // Scenario A from iconv-testing.md: 100 'A' chars → UTF-32LE with tiny buffer.
    // Forces repeated E2BIG as the 4-byte buffer can only hold one codepoint.
    std::string         input(100, 'A');
    std::array<char, 4> buf{};
    std::vector<char>   output;
    for (char c : input | iconv_transcode("UTF-8", "UTF-32LE", std::span(buf)))
        output.push_back(c);
    REQUIRE(output.size() == 400);
    for (int i = 0; i < 100; ++i) {
        CHECK(static_cast<unsigned char>(output[i * 4]) == 0x41);
        CHECK(static_cast<unsigned char>(output[i * 4 + 1]) == 0x00);
        CHECK(static_cast<unsigned char>(output[i * 4 + 2]) == 0x00);
        CHECK(static_cast<unsigned char>(output[i * 4 + 3]) == 0x00);
    }
}

TEST_CASE("real iconv split multi-byte: U+1D11E (4-byte UTF-8)", "[transcoding::iconv_real]") {
    // Scenario B from iconv-testing.md: 𝄞 (U+1D11E) = F0 9D 84 9E in UTF-8.
    // The view must accumulate all 4 bytes before iconv can decode them.
    std::vector<char>   input{'\xF0', '\x9D', '\x84', '\x9E'};
    std::array<char, 4> buf{};
    std::vector<char>   output;
    for (char c : input | iconv_transcode("UTF-8", "UTF-32LE", std::span(buf)))
        output.push_back(c);
    REQUIRE(output.size() == 4);
    // U+1D11E in UTF-32LE: 1E D1 01 00
    CHECK(static_cast<unsigned char>(output[0]) == 0x1E);
    CHECK(static_cast<unsigned char>(output[1]) == 0xD1);
    CHECK(static_cast<unsigned char>(output[2]) == 0x01);
    CHECK(static_cast<unsigned char>(output[3]) == 0x00);
}

TEST_CASE("real iconv ISO-2022-JP flush: stateful encoding produces trailing escape", "[transcoding::iconv_real]") {
    // あ (U+3042) in UTF-8: E3 81 82
    // Expected ISO-2022-JP output:
    //   1B 24 42  — ESC $ B (switch to JIS X 0208)
    //   24 22     — encoded あ
    //   1B 28 42  — ESC ( B (flush: switch back to ASCII)
    std::vector<char>    input{'\xE3', '\x81', '\x82'};
    std::array<char, 64> buf{};
    std::vector<char>    output;
    for (char c : input | iconv_transcode("UTF-8", "ISO-2022-JP", std::span(buf)))
        output.push_back(c);
    std::vector<unsigned char> expected{0x1B, 0x24, 0x42, 0x24, 0x22, 0x1B, 0x28, 0x42};
    REQUIRE(output.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        INFO("byte " << i);
        CHECK(static_cast<unsigned char>(output[i]) == expected[i]);
    }
}

TEST_CASE("real iconv ISO-2022-JP flush: mixed ASCII and Japanese", "[transcoding::iconv_real]") {
    // "Aあ" in UTF-8: 41 E3 81 82
    // Expected ISO-2022-JP: 41 1B 24 42 24 22 1B 28 42
    std::vector<char>    input{'\x41', '\xE3', '\x81', '\x82'};
    std::array<char, 64> buf{};
    std::vector<char>    output;
    for (char c : input | iconv_transcode("UTF-8", "ISO-2022-JP", std::span(buf)))
        output.push_back(c);
    std::vector<unsigned char> expected{0x41, 0x1B, 0x24, 0x42, 0x24, 0x22, 0x1B, 0x28, 0x42};
    REQUIRE(output.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        INFO("byte " << i);
        CHECK(static_cast<unsigned char>(output[i]) == expected[i]);
    }
}

TEST_CASE("real iconv ISO-2022-JP flush with or_error view", "[transcoding::iconv_real]") {
    // Same as above but via the or_error view — all values should succeed.
    std::vector<char>    input{'\xE3', '\x81', '\x82'};
    std::array<char, 64> buf{};
    std::vector<char>    output;
    for (auto r : input | iconv_transcode_or_error("UTF-8", "ISO-2022-JP", std::span(buf))) {
        REQUIRE(r.has_value());
        output.push_back(r.value());
    }
    std::vector<unsigned char> expected{0x1B, 0x24, 0x42, 0x24, 0x22, 0x1B, 0x28, 0x42};
    REQUIRE(output.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        INFO("byte " << i);
        CHECK(static_cast<unsigned char>(output[i]) == expected[i]);
    }
}

TEST_CASE("real iconv ISO-2022-JP flush with small buffer", "[transcoding::iconv_real]") {
    // Force E2BIG by using a tiny buffer, verifying flush still works.
    std::vector<char>   input{'\xE3', '\x81', '\x82'};
    std::array<char, 3> buf{};
    std::vector<char>   output;
    for (char c : input | iconv_transcode("UTF-8", "ISO-2022-JP", std::span(buf)))
        output.push_back(c);
    std::vector<unsigned char> expected{0x1B, 0x24, 0x42, 0x24, 0x22, 0x1B, 0x28, 0x42};
    REQUIRE(output.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        INFO("byte " << i);
        CHECK(static_cast<unsigned char>(output[i]) == expected[i]);
    }
}
