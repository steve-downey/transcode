// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>
#include <catch2/catch_all.hpp>

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
