// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_bulk.hpp>
#include <beman/transcode/iconv_bulk.hpp>

#include <catch2/catch_all.hpp>

#include <tests/beman/transcode/iconv_mock.hpp>

#include <algorithm>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace beman::transcoding;
using namespace beman::transcoding::tests;

// ---------------------------------------------------------------------------
// iconv_transcode_to — identity mock (copies input to output)
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to identity transcode", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    std::string     input  = "Hello, world!";
    auto            result = iconv_transcode_to<std::string>(std::span<const char>(input), "ASCII", "ASCII", fns);
    CHECK(result == input);
}

TEST_CASE("iconv_transcode_to empty input", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    std::string     empty;
    auto            result = iconv_transcode_to<std::string>(std::span<const char>(empty), "ASCII", "ASCII", fns);
    CHECK(result.empty());
}

TEST_CASE("iconv_transcode_to vector<char> container", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    std::string     input = "ABCDE";
    auto result           = iconv_transcode_to<std::vector<char>>(std::span<const char>(input), "ASCII", "ASCII", fns);
    CHECK(result == std::vector<char>(input.begin(), input.end()));
}

// ---------------------------------------------------------------------------
// iconv_transcode_to — E2BIG recovery
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to recovers from E2BIG", "[transcoding::iconv_bulk]") {
    // mock_iconv_e2big writes 1 byte then returns E2BIG; forces repeated growth.
    iconv_functions fns{mock_iconv_open, mock_iconv_e2big, mock_iconv_close};
    std::string     input  = "Hello";
    auto            result = iconv_transcode_to<std::string>(std::span<const char>(input), "ASCII", "ASCII", fns);
    CHECK(result == input);
}

// ---------------------------------------------------------------------------
// iconv_transcode_to — EILSEQ handling (replaces with '?')
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to replaces EILSEQ bytes with '?'", "[transcoding::iconv_bulk]") {
    // mock_iconv_eilseq always signals EILSEQ; every input byte is replaced.
    iconv_functions fns{mock_iconv_open, mock_iconv_eilseq, mock_iconv_close};
    std::string     input  = "ABC";
    auto            result = iconv_transcode_to<std::string>(std::span<const char>(input), "X", "X", fns);
    CHECK(result == "???");
}

// ---------------------------------------------------------------------------
// iconv_transcode_to — EINVAL (incomplete sequence at end)
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to replaces trailing incomplete sequence with '?'", "[transcoding::iconv_bulk]") {
    // mock_iconv_pairwise: needs 2 bytes; returns EINVAL for a lone trailing byte.
    iconv_functions   fns{mock_iconv_open, mock_iconv_pairwise, mock_iconv_close};
    std::vector<char> input{0x41, 0x42, 0x43}; // one complete pair + one orphan
    auto              result = iconv_transcode_to<std::string>(std::span<const char>(input), "X", "X", fns);
    // Pair {0x41, 0x42} copied, orphan {0x43} replaced
    CHECK(result == std::string{0x41, 0x42, '?'});
}

// ---------------------------------------------------------------------------
// iconv_transcode_to — stateful encoding flush
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to flushes stateful encoding", "[transcoding::iconv_bulk]") {
    // mock_iconv_stateful writes a 0x0F reset byte on flush.
    iconv_functions fns{mock_iconv_open, mock_iconv_stateful, mock_iconv_close};
    std::string     input  = "AB";
    auto            result = iconv_transcode_to<std::string>(std::span<const char>(input), "X", "X", fns);
    // input bytes copied, then 0x0F appended by flush
    REQUIRE(result.size() == 3);
    CHECK(result[0] == 'A');
    CHECK(result[1] == 'B');
    CHECK(static_cast<unsigned char>(result[2]) == 0x0F);
}

// ---------------------------------------------------------------------------
// iconv_transcode_to — real iconv
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to with real iconv UTF-8 to UTF-32LE", "[transcoding::iconv_bulk]") {
    std::string_view input  = "Hi";
    auto             result = iconv_transcode_to<std::string>(input, "UTF-8", "UTF-32LE");
    // 'H' (0x48) and 'i' (0x69) as UTF-32LE (little-endian 4 bytes each)
    REQUIRE(result.size() == 8);
    CHECK(static_cast<unsigned char>(result[0]) == 0x48);
    CHECK(static_cast<unsigned char>(result[1]) == 0x00);
    CHECK(static_cast<unsigned char>(result[2]) == 0x00);
    CHECK(static_cast<unsigned char>(result[3]) == 0x00);
    CHECK(static_cast<unsigned char>(result[4]) == 0x69);
    CHECK(static_cast<unsigned char>(result[5]) == 0x00);
    CHECK(static_cast<unsigned char>(result[6]) == 0x00);
    CHECK(static_cast<unsigned char>(result[7]) == 0x00);
}

// ---------------------------------------------------------------------------
// iconv_transcode_into
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_into identity transcode", "[transcoding::iconv_bulk]") {
    iconv_functions   fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    std::string       input = "Hello";
    std::vector<char> output;
    iconv_transcode_into(std::span<const char>(input), "ASCII", "ASCII", std::back_inserter(output), fns);
    CHECK(output == std::vector<char>(input.begin(), input.end()));
}

TEST_CASE("iconv_transcode_into replaces EILSEQ with '?'", "[transcoding::iconv_bulk]") {
    iconv_functions   fns{mock_iconv_open, mock_iconv_eilseq, mock_iconv_close};
    std::string       input = "AB";
    std::vector<char> output;
    iconv_transcode_into(std::span<const char>(input), "X", "X", std::back_inserter(output), fns);
    CHECK(output == std::vector<char>{'?', '?'});
}

TEST_CASE("iconv_transcode_into with E2BIG recovery", "[transcoding::iconv_bulk]") {
    iconv_functions   fns{mock_iconv_open, mock_iconv_e2big, mock_iconv_close};
    std::string       input = "Test";
    std::vector<char> output;
    iconv_transcode_into(std::span<const char>(input), "ASCII", "ASCII", std::back_inserter(output), fns);
    CHECK(output == std::vector<char>(input.begin(), input.end()));
}

TEST_CASE("iconv_transcode_into returns advanced output iterator", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    std::string     input = "XY";
    std::string     output(4, '\0'); // pre-allocated 4 chars
    auto            it = iconv_transcode_into(std::span<const char>(input), "ASCII", "ASCII", output.begin(), fns);
    // Iterator should have advanced by 2 (input length)
    CHECK(std::distance(output.begin(), it) == 2);
    CHECK(output[0] == 'X');
    CHECK(output[1] == 'Y');
}

// ---------------------------------------------------------------------------
// iconv_transcode_to_or_error
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to_or_error succeeds on valid input", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    std::string     input = "Hello";
    auto result = iconv_transcode_to_or_error<std::string>(std::span<const char>(input), "ASCII", "ASCII", fns);
    REQUIRE(result.has_value());
    CHECK(*result == input);
}

TEST_CASE("iconv_transcode_to_or_error returns invalid_sequence on EILSEQ", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv_eilseq, mock_iconv_close};
    std::string     input  = "AB";
    auto            result = iconv_transcode_to_or_error<std::string>(std::span<const char>(input), "X", "X", fns);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == iconv_error::invalid_sequence);
}

TEST_CASE("iconv_transcode_to_or_error returns incomplete_sequence on EINVAL", "[transcoding::iconv_bulk]") {
    iconv_functions   fns{mock_iconv_open, mock_iconv_pairwise, mock_iconv_close};
    std::vector<char> input{0x41, 0x42, 0x43}; // one pair + orphan
    auto              result = iconv_transcode_to_or_error<std::string>(std::span<const char>(input), "X", "X", fns);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == iconv_error::incomplete_sequence);
}

TEST_CASE("iconv_transcode_to_or_error with E2BIG recovery succeeds", "[transcoding::iconv_bulk]") {
    iconv_functions fns{mock_iconv_open, mock_iconv_e2big, mock_iconv_close};
    std::string     input = "AB";
    auto result = iconv_transcode_to_or_error<std::string>(std::span<const char>(input), "ASCII", "ASCII", fns);
    REQUIRE(result.has_value());
    CHECK(*result == input);
}

TEST_CASE("iconv_transcode_to_or_error with real iconv succeeds", "[transcoding::iconv_bulk]") {
    std::string_view input  = "ok";
    auto             result = iconv_transcode_to_or_error<std::string>(input, "UTF-8", "UTF-8");
    REQUIRE(result.has_value());
    CHECK(*result == "ok");
}
