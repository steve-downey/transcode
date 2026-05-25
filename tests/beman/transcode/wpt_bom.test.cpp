// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_bom_vectors.hpp>

#include <cstdint>
#include <vector>

using namespace beman::transcoding;

namespace {

std::vector<char32_t> decode_utf8(const std::vector<uint8_t>& input) {
    std::vector<char>     bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
        result.push_back(cp);
    return result;
}

std::vector<char32_t> decode_utf16le(const std::vector<uint8_t>& input) {
    std::vector<char>     bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<codec::utf_16le>)
        result.push_back(cp);
    return result;
}

std::vector<char32_t> decode_utf16be(const std::vector<uint8_t>& input) {
    std::vector<char>     bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<codec::utf_16be>)
        result.push_back(cp);
    return result;
}

std::vector<uint8_t> concat(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    std::vector<uint8_t> r = a;
    r.insert(r.end(), b.begin(), b.end());
    return r;
}

} // namespace

TEST_CASE("WPT BOM: UTF-8 without BOM decodes correctly", "[wpt::bom]") {
    const auto& c = beman::transcoding::tests::wpt::wpt_bom_cases[0];
    CHECK(decode_utf8(c.bytes) == c.expected);
}

TEST_CASE("WPT BOM: UTF-8 with BOM strips the BOM", "[wpt::bom]") {
    const auto& c         = beman::transcoding::tests::wpt::wpt_bom_cases[0];
    auto        with_bom  = concat(c.bom, c.bytes);
    CHECK(decode_utf8(with_bom) == c.expected);
}

TEST_CASE("WPT BOM: UTF-16LE without BOM decodes correctly", "[wpt::bom]") {
    const auto& c = beman::transcoding::tests::wpt::wpt_bom_cases[1];
    CHECK(decode_utf16le(c.bytes) == c.expected);
}

TEST_CASE("WPT BOM: UTF-16LE with BOM strips the BOM", "[wpt::bom]") {
    const auto& c        = beman::transcoding::tests::wpt::wpt_bom_cases[1];
    auto        with_bom = concat(c.bom, c.bytes);
    CHECK(decode_utf16le(with_bom) == c.expected);
}

TEST_CASE("WPT BOM: UTF-16BE without BOM decodes correctly", "[wpt::bom]") {
    const auto& c = beman::transcoding::tests::wpt::wpt_bom_cases[2];
    CHECK(decode_utf16be(c.bytes) == c.expected);
}

TEST_CASE("WPT BOM: UTF-16BE with BOM strips the BOM", "[wpt::bom]") {
    const auto& c        = beman::transcoding::tests::wpt::wpt_bom_cases[2];
    auto        with_bom = concat(c.bom, c.bytes);
    CHECK(decode_utf16be(with_bom) == c.expected);
}

TEST_CASE("WPT BOM: Mismatching BOM is not stripped", "[wpt::bom]") {
    const auto& utf8    = beman::transcoding::tests::wpt::wpt_bom_cases[0];
    const auto& utf16le = beman::transcoding::tests::wpt::wpt_bom_cases[1];
    const auto& utf16be = beman::transcoding::tests::wpt::wpt_bom_cases[2];

    // UTF-8 decoder with UTF-16LE BOM prefix: not stripped, produces different output
    CHECK(decode_utf8(concat(utf16le.bom, utf8.bytes)) != utf8.expected);
    // UTF-8 decoder with UTF-16BE BOM prefix: not stripped
    CHECK(decode_utf8(concat(utf16be.bom, utf8.bytes)) != utf8.expected);

    // UTF-16LE decoder with UTF-8 BOM prefix: not stripped
    CHECK(decode_utf16le(concat(utf8.bom, utf16le.bytes)) != utf16le.expected);
    // UTF-16LE decoder with UTF-16BE BOM: 0xFE,0xFF decodes as LE to U+FFFE (not U+FEFF)
    CHECK(decode_utf16le(concat(utf16be.bom, utf16le.bytes)) != utf16le.expected);

    // UTF-16BE decoder with UTF-8 BOM prefix: not stripped
    CHECK(decode_utf16be(concat(utf8.bom, utf16be.bytes)) != utf16be.expected);
    // UTF-16BE decoder with UTF-16LE BOM: 0xFF,0xFE decodes as BE to U+FFFE (not U+FEFF)
    CHECK(decode_utf16be(concat(utf16le.bom, utf16be.bytes)) != utf16be.expected);
}
