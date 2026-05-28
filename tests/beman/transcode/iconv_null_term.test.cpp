// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_transcode_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <beman/transcode/detail/null_term.hpp>
#include <beman/transcode/iconv_bulk.hpp>
#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_transcode_or_error_view.hpp>

#include <catch2/catch_all.hpp>

#include <tests/beman/transcode/iconv_mock.hpp>

#include <array>
#include <ranges>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;
using namespace beman::transcoding::tests;

// ---------------------------------------------------------------------------
// Streaming view: views::null_term | iconv_transcode_closure (mock)
// ---------------------------------------------------------------------------

TEST_CASE("null_term string literal piped to iconv_transcode_view mock identity",
          "[transcoding::iconv_null_term]") {
    std::array<char, 16>                     buf{};
    iconv_functions                          fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    iconv_transcode_closure<iconv_functions> closure{fns, "ASCII", "ASCII", std::span(buf)};

    auto        v = views::null_term("Hello") | closure;
    std::string result;
    for (char c : v)
        result.push_back(c);
    CHECK(result == "Hello");
}

TEST_CASE("null_term char pointer piped to iconv_transcode_view mock identity",
          "[transcoding::iconv_null_term]") {
    std::array<char, 16>                     buf{};
    iconv_functions                          fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    iconv_transcode_closure<iconv_functions> closure{fns, "ASCII", "ASCII", std::span(buf)};

    const char* s = "world";
    auto        v = views::null_term(s) | closure;
    std::string result;
    for (char c : v)
        result.push_back(c);
    CHECK(result == "world");
}

TEST_CASE("null_term empty string piped to iconv_transcode_view mock produces empty output",
          "[transcoding::iconv_null_term]") {
    std::array<char, 16>                     buf{};
    iconv_functions                          fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    iconv_transcode_closure<iconv_functions> closure{fns, "ASCII", "ASCII", std::span(buf)};

    auto        v = views::null_term("") | closure;
    std::string result;
    for (char c : v)
        result.push_back(c);
    CHECK(result.empty());
}

// ---------------------------------------------------------------------------
// Streaming or_error view: views::null_term | iconv_transcode_or_error (mock)
// ---------------------------------------------------------------------------

TEST_CASE("null_term piped to iconv_transcode_or_error_view mock identity",
          "[transcoding::iconv_null_term]") {
    std::array<char, 16>                              buf{};
    iconv_functions                                   fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    iconv_transcode_or_error_closure<iconv_functions> closure{fns, "ASCII", "ASCII", std::span(buf)};

    auto        v = views::null_term("Hello") | closure;
    std::string result;
    for (auto r : v) {
        REQUIRE(r.has_value());
        result.push_back(*r);
    }
    CHECK(result == "Hello");
}

// ---------------------------------------------------------------------------
// Bulk: iconv_transcode_to with views::null_term source (mock)
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to with null_term string literal source (mock identity)",
          "[transcoding::iconv_null_term]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    auto result = iconv_transcode_to<std::string>(views::null_term("Hello"), "ASCII", "ASCII", fns);
    CHECK(result == "Hello");
}

TEST_CASE("iconv_transcode_to with null_term char pointer source (mock identity)",
          "[transcoding::iconv_null_term]") {
    iconv_functions fns{mock_iconv_open, mock_iconv, mock_iconv_close};
    const char* s  = "world";
    auto        result = iconv_transcode_to<std::string>(views::null_term(s), "ASCII", "ASCII", fns);
    CHECK(result == "world");
}

// ---------------------------------------------------------------------------
// Real iconv: views::null_term | iconv_transcode
// ---------------------------------------------------------------------------

TEST_CASE("null_term piped to iconv_transcode UTF-8 identity (real iconv)",
          "[transcoding::iconv_null_term]") {
    std::array<char, 64> buf{};
    auto v = views::null_term("Hello") | iconv_transcode("UTF-8", "UTF-8", std::span(buf));
    std::string result;
    for (char c : v)
        result.push_back(c);
    CHECK(result == "Hello");
}

TEST_CASE("null_term piped to iconv_transcode UTF-8 to UTF-32LE (real iconv)",
          "[transcoding::iconv_null_term]") {
    // "A" in UTF-32LE is four bytes: 0x41, 0x00, 0x00, 0x00
    std::array<char, 64> buf{};
    auto v = views::null_term("A") | iconv_transcode("UTF-8", "UTF-32LE", std::span(buf));
    std::vector<unsigned char> result;
    for (char c : v)
        result.push_back(static_cast<unsigned char>(c));
    REQUIRE(result.size() == 4);
    CHECK(result[0] == 0x41);
    CHECK(result[1] == 0x00);
    CHECK(result[2] == 0x00);
    CHECK(result[3] == 0x00);
}

// ---------------------------------------------------------------------------
// Real iconv: iconv_transcode_to with views::null_term source
// ---------------------------------------------------------------------------

TEST_CASE("iconv_transcode_to with null_term source UTF-8 identity (real iconv)",
          "[transcoding::iconv_null_term]") {
    const char* s      = "Hello";
    auto        result = iconv_transcode_to<std::string>(views::null_term(s), "UTF-8", "UTF-8");
    CHECK(result == "Hello");
}
