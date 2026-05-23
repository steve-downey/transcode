// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_transcode_view.hpp>
#include <catch2/catch_all.hpp>

#include "tests/beman/transcode/iconv_mock.hpp"

#include <algorithm>
#include <concepts>
#include <cstring>
#include <ranges>
#include <vector>

using namespace beman::transcoding;
using namespace beman::transcoding::tests;

TEST_CASE("iconv_transcode_view identity conversion", "[transcoding::iconv_transcode]") {
    std::vector<char>    input{'H', 'e', 'l', 'l', 'o'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "ASCII", "ASCII", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    CHECK(output == input);
}

TEST_CASE("iconv_transcode_view destructor closes handle", "[transcoding::iconv_transcode]") {
    struct close_counting_fns {
        int* close_count;

        iconv_t open(const char*, const char*) const { return (iconv_t)1; }
        size_t  convert(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) const {
            size_t n = std::min(*inleft, *outleft);
            std::memcpy(*out, *in, n);
            *in += n;
            *inleft -= n;
            *out += n;
            *outleft -= n;
            return 0;
        }
        int close(iconv_t) const { return ++(*close_count), 0; }
    };

    int                  close_count = 0;
    std::vector<char>    input{'A'};
    std::array<char, 16> buf{};
    close_counting_fns   fns{&close_count};

    {
        auto view =
            iconv_transcode_view<close_counting_fns, std::vector<char>>(input, fns, "ASCII", "ASCII", std::span(buf));
        for ([[maybe_unused]] char c : view) {
        }
    } // iterator destroyed here
    CHECK(close_count == 1);
}

TEST_CASE("iconv_transcode_view satisfies input_range", "[transcoding::iconv_transcode]") {
    using view_t = iconv_transcode_view<iconv_functions, std::vector<char>>;
    static_assert(std::ranges::input_range<view_t>);
    static_assert(std::same_as<std::ranges::range_value_t<view_t>, char>);

    using iter_t = std::ranges::iterator_t<view_t>;
    static_assert(!std::copy_constructible<iter_t>);
    static_assert(std::movable<iter_t>);
}
