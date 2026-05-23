// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_transcode_or_error_view.hpp>
#include <beman/transcode/iconv_transcode_or_error_view.hpp>

#include <catch2/catch_all.hpp>

#include <tests/beman/transcode/iconv_mock.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstring>
#include <expected>
#include <ranges>
#include <vector>

using namespace beman::transcoding;
using namespace beman::transcoding::tests;

namespace {
using result_t = std::expected<char, iconv_error>;

template <typename View>
std::vector<result_t> collect(View&& v) {
    std::vector<result_t> out;
    for (auto r : v)
        out.push_back(r);
    return out;
}
} // namespace

TEST_CASE("iconv_transcode_or_error_view satisfies input_range", "[transcoding::iconv_transcode_or_error]") {
    using view_t = iconv_transcode_or_error_view<iconv_functions, std::vector<char>>;
    static_assert(std::ranges::input_range<view_t>);
    static_assert(std::same_as<std::ranges::range_value_t<view_t>, result_t>);

    using iter_t = std::ranges::iterator_t<view_t>;
    static_assert(!std::copy_constructible<iter_t>);
    static_assert(std::movable<iter_t>);
}

TEST_CASE("iconv_transcode_or_error_view identity yields expected values",
          "[transcoding::iconv_transcode_or_error]") {
    std::vector<char>    input{'H', 'i'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    auto                 view = iconv_transcode_or_error_view<iconv_functions, std::vector<char>>(
        input, fns, "ASCII", "ASCII", std::span(buf));
    auto result = collect(view);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == 'H');
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == 'i');
}

TEST_CASE("iconv_transcode_or_error_view EILSEQ yields unexpected invalid_sequence",
          "[transcoding::iconv_transcode_or_error]") {
    std::vector<char>    input{'\xFF', '\xFE'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_eilseq, mock_iconv_close};
    auto                 view = iconv_transcode_or_error_view<iconv_functions, std::vector<char>>(
        input, fns, "X", "X", std::span(buf));
    auto result = collect(view);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == iconv_error::invalid_sequence);
    CHECK(!result[1].has_value());
    CHECK(result[1].error() == iconv_error::invalid_sequence);
}

TEST_CASE("iconv_transcode_or_error_view EINVAL at end yields unexpected incomplete_sequence",
          "[transcoding::iconv_transcode_or_error]") {
    // pairwise mock needs 2 bytes; odd input means a trailing incomplete sequence
    std::vector<char>    input{0x41, 0x42, 0x43};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_pairwise, mock_iconv_close};
    auto                 view = iconv_transcode_or_error_view<iconv_functions, std::vector<char>>(
        input, fns, "X", "X", std::span(buf));
    auto result = collect(view);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == 0x41);
    REQUIRE(result[1].has_value());
    CHECK(result[1].value() == 0x42);
    CHECK(!result[2].has_value());
    CHECK(result[2].error() == iconv_error::incomplete_sequence);
}

TEST_CASE("iconv_transcode_or_error_view destructor closes handle",
          "[transcoding::iconv_transcode_or_error]") {
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
        auto view = iconv_transcode_or_error_view<close_counting_fns, std::vector<char>>(
            input, fns, "ASCII", "ASCII", std::span(buf));
        for ([[maybe_unused]] auto r : view) {
        }
    }
    CHECK(close_count == 1);
}
