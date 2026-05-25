// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/transcode_string.hpp>
#include <beman/transcode/detail/transcode_string.hpp>

#include <catch2/catch_all.hpp>

#include <optional>
#include <span>
#include <string>

using namespace beman::transcoding;

TEST_CASE("transcode_string label: windows-1252 to UTF-8 by label", "[transcode_string][label]") {
    // 0x80 in windows-1252 → U+20AC EURO SIGN → UTF-8: 0xE2 0x82 0xAC
    std::string src{'\x80'};
    std::string expected{'\xE2', '\x82', '\xAC'};
    auto        result = transcode_string(std::span<const char>(src), "windows-1252", "utf-8");
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

TEST_CASE("transcode_string label: UTF-8 to windows-1252 by label", "[transcode_string][label]") {
    // U+20AC in UTF-8: 0xE2 0x82 0xAC → windows-1252: 0x80
    std::string src{'\xE2', '\x82', '\xAC'};
    std::string expected{'\x80'};
    auto        result = transcode_string(std::span<const char>(src), "utf-8", "windows-1252");
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

TEST_CASE("transcode_string label: case-insensitive label lookup", "[transcode_string][label]") {
    std::string src = "hello";
    auto        r1  = transcode_string(std::span<const char>(src), "UTF-8", "UTF-8");
    auto        r2  = transcode_string(std::span<const char>(src), "utf-8", "utf-8");
    auto        r3  = transcode_string(std::span<const char>(src), "Utf-8", "Utf-8");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    REQUIRE(r3.has_value());
    CHECK(*r1 == "hello");
    CHECK(*r2 == "hello");
    CHECK(*r3 == "hello");
}

TEST_CASE("transcode_string label: whitespace-stripped label", "[transcode_string][label]") {
    std::string src = "hello";
    auto        result = transcode_string(std::span<const char>(src), "  utf-8  ", "  utf-8  ");
    REQUIRE(result.has_value());
    CHECK(*result == "hello");
}

TEST_CASE("transcode_string label: unknown from_label returns nullopt", "[transcode_string][label]") {
    std::string src = "hello";
    auto        result = transcode_string(std::span<const char>(src), "not-a-codec", "utf-8");
    CHECK(!result.has_value());
}

TEST_CASE("transcode_string label: unknown to_label returns nullopt", "[transcode_string][label]") {
    std::string src = "hello";
    auto        result = transcode_string(std::span<const char>(src), "utf-8", "not-a-codec");
    CHECK(!result.has_value());
}

TEST_CASE("transcode_string label: both labels unknown returns nullopt", "[transcode_string][label]") {
    std::string src = "hello";
    auto        result = transcode_string(std::span<const char>(src), "bogus", "garbage");
    CHECK(!result.has_value());
}

TEST_CASE("transcode_string label: alias labels resolve correctly", "[transcode_string][label]") {
    // "sjis", "x-sjis", "shift_jis" all map to codec::shift_jis
    std::string src = "abc";
    auto        r1  = transcode_string(std::span<const char>(src), "utf-8", "sjis");
    auto        r2  = transcode_string(std::span<const char>(src), "utf-8", "shift_jis");
    auto        r3  = transcode_string(std::span<const char>(src), "utf-8", "x-sjis");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    REQUIRE(r3.has_value());
    CHECK(*r1 == "abc");
    CHECK(*r2 == "abc");
    CHECK(*r3 == "abc");
}

TEST_CASE("transcode_string label: empty label returns nullopt", "[transcode_string][label]") {
    std::string src = "hello";
    CHECK(!transcode_string(std::span<const char>(src), "", "utf-8").has_value());
    CHECK(!transcode_string(std::span<const char>(src), "utf-8", "").has_value());
}
