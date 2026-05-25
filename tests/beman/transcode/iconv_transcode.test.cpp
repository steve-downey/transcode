// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_transcode_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>
#include <catch2/catch_all.hpp>

#include <tests/beman/transcode/iconv_mock.hpp>

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
            if (in == nullptr || *in == nullptr)
                return 0;
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
    }
    CHECK(close_count == 1);
}

TEST_CASE("iconv_transcode_view handles EINVAL by accumulating pairs", "[transcoding::iconv_transcode]") {
    // mock_iconv_pairwise returns EINVAL for < 2 input bytes; the iterator must
    // accumulate bytes in a staging buffer until a complete pair is available.
    std::vector<char>    input{0x41, 0x42, 0x43, 0x44};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_pairwise, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    CHECK(output == input);
}

TEST_CASE("iconv_transcode_view discards incomplete trailing sequence on EINVAL", "[transcoding::iconv_transcode]") {
    // 3-byte input: one complete pair {0x41,0x42} plus one incomplete byte {0x43}.
    // The complete pair must be converted; the trailing byte must be discarded
    // (EINVAL with no more input → done).
    std::vector<char>    input{0x41, 0x42, 0x43};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_pairwise, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    std::vector<char> expected{0x41, 0x42};
    CHECK(output == expected);
}

TEST_CASE("iconv_transcode_view yields all bytes despite repeated E2BIG", "[transcoding::iconv_transcode]") {
    // mock_iconv_e2big writes 1 byte then always returns E2BIG; the iterator must
    // yield that byte and continue rather than stalling or dropping input.
    std::vector<char>    input{'H', 'i', '!'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_e2big, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    CHECK(output == input);
}

TEST_CASE("iconv_transcode_view satisfies input_range", "[transcoding::iconv_transcode]") {
    using view_t = iconv_transcode_view<iconv_functions, std::vector<char>>;
    static_assert(std::ranges::input_range<view_t>);
    static_assert(std::same_as<std::ranges::range_value_t<view_t>, char>);

    using iter_t = std::ranges::iterator_t<view_t>;
    static_assert(!std::copy_constructible<iter_t>);
    static_assert(std::movable<iter_t>);
}

TEST_CASE("iconv_transcode_closure pipe syntax with mock fns", "[transcoding::iconv_transcode]") {
    std::vector<char>    input{'H', 'e', 'l', 'l', 'o'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    auto                 closure = iconv_transcode_closure<iconv_functions>{fns, "ASCII", "ASCII", std::span(buf)};
    std::vector<char>    output;
    for (char c : input | closure)
        output.push_back(c);
    CHECK(output == input);
}

TEST_CASE("iconv_transcode_view EILSEQ skips bad bytes and terminates", "[transcoding::iconv_transcode]") {
    std::vector<char>    input{'\xFF', '\xFE'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_eilseq, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    CHECK(output.empty());
}

TEST_CASE("iconv_transcode_view stateful flush produces trailing bytes", "[transcoding::iconv_transcode]") {
    std::vector<char>    input{'A', 'B'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_stateful, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    std::vector<char> expected{'A', 'B', 0x0F};
    CHECK(output == expected);
}

TEST_CASE("iconv_transcode_view partial staging consume shifts correctly", "[transcoding::iconv_transcode]") {
    // mock_iconv_partial_consume consumes 1 byte, writes 1, returns EINVAL.
    // With 4-byte input, each pair of bytes produces output after accumulating.
    std::vector<char>    input{0x41, 0x42, 0x43, 0x44};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_partial_consume, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    CHECK(output.size() >= 2);
}

TEST_CASE("iconv_transcode_view output before EILSEQ error", "[transcoding::iconv_transcode]") {
    // mock_iconv_output_then_eilseq writes 1 byte then returns EILSEQ.
    // Tests the path where output is yielded before the error (line 210-211 return).
    std::vector<char>    input{'A', 'B', 'C'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_output_then_eilseq, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    // First byte should be output before EILSEQ silently skips remaining
    CHECK(!output.empty());
}

TEST_CASE("iconv_transcode_view output before E2BIG error", "[transcoding::iconv_transcode]") {
    // mock_iconv_output_then_e2big writes 1 byte then returns E2BIG.
    // Tests the path where output is yielded before E2BIG error (line 210-211 return).
    std::vector<char>    input{'X', 'Y', 'Z'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_output_then_e2big, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    // Should output bytes despite repeated E2BIG errors
    CHECK(!output.empty());
}

TEST_CASE("iconv_transcode_view EILSEQ multi-byte shift", "[transcoding::iconv_transcode]") {
    // mock_iconv_eilseq_multi_byte always returns EILSEQ with no output.
    // With 3+ staging bytes, triggers the byte-shifting loop (line 214-216).
    std::vector<char>    input{'A', 'B', 'C'};
    std::array<char, 16> buf{};
    iconv_functions      fns{mock_iconv_open, mock_iconv_eilseq_multi_byte, mock_iconv_close};
    auto view = iconv_transcode_view<iconv_functions, std::vector<char>>(input, fns, "X", "X", std::span(buf));
    std::vector<char> output;
    for (char c : view)
        output.push_back(c);
    // EILSEQ should be silently skipped
    CHECK(output.empty());
}
