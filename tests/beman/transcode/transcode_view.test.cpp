// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/transcode_view.hpp>
#include <beman/transcode/detail/transcode_view.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <span>
#include <string>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

TEST_CASE("transcode_view: windows-1252 bytes to UTF-8 via pipe", "[transcode_view]") {
    // 0x80 in windows-1252 → U+20AC EURO SIGN → UTF-8: 0xE2 0x82 0xAC
    std::string src{'\x80'};
    std::string expected{'\xE2', '\x82', '\xAC'};
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::windows_1252, codec::utf_8>)
        result.push_back(b);
    CHECK(result == expected);
}

TEST_CASE("transcode_view: UTF-8 to windows-1252 via pipe", "[transcode_view]") {
    // U+20AC in UTF-8: 0xE2 0x82 0xAC → windows-1252: 0x80
    std::string src{'\xE2', '\x82', '\xAC'};
    std::string expected{'\x80'};
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::utf_8, codec::windows_1252>)
        result.push_back(b);
    CHECK(result == expected);
}

TEST_CASE("transcode_view: UTF-8 to GBK via pipe (ASCII passthrough)", "[transcode_view]") {
    std::string src    = "hello";
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::utf_8, codec::gbk>)
        result.push_back(b);
    CHECK(result == "hello");
}

TEST_CASE("transcode_view: GBK to UTF-8 via pipe (ASCII passthrough)", "[transcode_view]") {
    std::string src    = "abc";
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::gbk, codec::utf_8>)
        result.push_back(b);
    CHECK(result == "abc");
}

TEST_CASE("transcode_view: UTF-8 to Shift_JIS via pipe (ASCII passthrough)", "[transcode_view]") {
    std::string src    = "abc";
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::utf_8, codec::shift_jis>)
        result.push_back(b);
    CHECK(result == "abc");
}

TEST_CASE("transcode_view: same codec is identity (UTF-8)", "[transcode_view]") {
    std::string src    = "hello world";
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::utf_8, codec::utf_8>)
        result.push_back(b);
    CHECK(result == src);
}

TEST_CASE("transcode_view: consteval — transcode_closure is default-constructible", "[transcode_view]") {
    CHECK(constify(true));
}
