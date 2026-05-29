// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/encode_view.hpp>

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

// Latin-1 table: bytes 0x80-0xFF identity-map to U+0080-U+00FF
inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = table_codec<latin1_upper>;

// Sparse table: only a few codepoints mapped, 0x80 unmapped
inline constexpr std::array<char32_t, 128> sparse_table = [] {
    std::array<char32_t, 128> t{};
    t[1] = U'\x00A1';
    t[2] = U'\x00A2';
    return t;
}();

using sparse_codec = table_codec<sparse_table>;

// ---------------------------------------------------------------------------
// encode_view basic tests
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: ASCII codepoints round-trip", "[encode_view]") {
    std::u32string src = U"ABC";
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == "ABC");
}

TEST_CASE("encode_view: upper-half codepoints encode correctly", "[encode_view]") {
    std::u32string src{U'\x00C0', U'\x00C1'};
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == '\xC0');
    CHECK(result[1] == '\xC1');
}

TEST_CASE("encode_view: unmapped codepoint yields replacement byte '?'", "[encode_view]") {
    std::u32string src{U'\x1F600'}; // emoji, not in Latin-1
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("encode_view: empty input produces empty output", "[encode_view]") {
    std::u32string src;
    std::string    result;
    for (char c : src | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result.empty());
}

TEST_CASE("encode_view: pipe with span of char32_t", "[encode_view]") {
    std::array<char32_t, 3> src{U'H', U'i', U'!'};
    std::string             result;
    for (char c : std::span<const char32_t>(src) | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == "Hi!");
}

// ---------------------------------------------------------------------------
// encode_or_error_view tests
// ---------------------------------------------------------------------------

TEST_CASE("encode_or_error_view: ASCII codepoints succeed", "[encode_view]") {
    std::u32string src  = U"Hi";
    auto           view = src | encode_or_error(latin1_codec{});
    auto           it   = view.begin();

    auto r1 = *it;
    CHECK(r1.has_value());
    CHECK(r1.value() == 'H');

    ++it;
    auto r2 = *it;
    CHECK(r2.has_value());
    CHECK(r2.value() == 'i');

    ++it;
    CHECK(it == std::default_sentinel);
}

TEST_CASE("encode_or_error_view: unmapped codepoint yields unexpected", "[encode_view]") {
    std::u32string src{U'\x1F600'}; // emoji, not in Latin-1
    auto           view = src | encode_or_error(latin1_codec{});
    auto           it   = view.begin();

    auto r = *it;
    CHECK(!r.has_value());
    CHECK(r.error() == decode_error::unmapped_codepoint);
}

TEST_CASE("encode_or_error_view: sparse table unmapped yields unexpected", "[encode_view]") {
    std::u32string src{U'\x0080'}; // index 0 is 0x80, which is unmapped in sparse_table
    auto           view = src | encode_or_error(sparse_codec{});
    auto           it   = view.begin();

    auto r = *it;
    CHECK(!r.has_value());
    CHECK(r.error() == decode_error::unmapped_codepoint);
}

TEST_CASE("encode_or_error_view: mixed success and error", "[encode_view]") {
    std::u32string                                 src{U'A', U'\x1F600', U'B'}; // A, emoji, B
    std::vector<std::expected<char, decode_error>> result;
    for (auto v : src | encode_or_error(latin1_codec{}))
        result.push_back(v);
    REQUIRE(result.size() == 3);
    CHECK(result[0].has_value());
    CHECK(result[0].value() == 'A');
    CHECK(!result[1].has_value());
    CHECK(result[1].error() == decode_error::unmapped_codepoint);
    CHECK(result[2].has_value());
    CHECK(result[2].value() == 'B');
}

// ---------------------------------------------------------------------------
// Round-trip: decode then encode
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: round-trip decode then encode with Latin-1", "[encode_view]") {
    std::string src{'\x41', '\xC0', '\xC1'}; // A, 0xC0, 0xC1 in Latin-1
    std::string result;
    for (char c : std::span<const char>(src) | decode(latin1_codec{}) | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == src);
}

TEST_CASE("encode_view: round-trip ASCII string", "[encode_view]") {
    std::string src = "hello world";
    std::string result;
    for (char c : std::span<const char>(src) | decode(latin1_codec{}) | encode(latin1_codec{}))
        result.push_back(c);
    CHECK(result == src);
}

// ---------------------------------------------------------------------------
// Constexpr tests
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: constexpr encode_one via table_codec", "[encode_view]") {
    CHECK(constify(latin1_codec{}.encode_one(U'A').bytes[0]) == 'A');
    CHECK(constify(latin1_codec{}.encode_one(U'A').count) == 1);
    CHECK(constify(latin1_codec{}.encode_one(U'\x00C0').bytes[0]) == static_cast<unsigned char>('\xC0'));
    CHECK(constify(latin1_codec{}.encode_one(U'\x1F600').is_error) == true);
}

TEST_CASE("encode_view: constexpr encode view iteration", "[encode_view]") {
    constexpr auto first_byte = [] {
        std::array<char32_t, 2> src{U'H', U'i'};
        auto                    view = src | encode(latin1_codec{});
        return *view.begin();
    }();
    CHECK(constify(first_byte) == 'H');
}

TEST_CASE("encode_view: constexpr round-trip", "[encode_view]") {
    constexpr auto result = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xC1};
        std::array<char, 3>          out{};
        int                          i = 0;
        for (char c : src | decode(latin1_codec{}) | encode(latin1_codec{}))
            out[static_cast<std::size_t>(i++)] = c;
        return out;
    }();
    CHECK(constify(result[0]) == '\x41');
    CHECK(constify(result[1]) == '\xC0');
    CHECK(constify(result[2]) == '\xC1');
}

// ---------------------------------------------------------------------------
// base() protocol
// ---------------------------------------------------------------------------

TEST_CASE("encode_view: view base() returns underlying range", "[encode_view]") {
    std::u32string src  = U"AB";
    auto           view = src | encode(latin1_codec{});
    CHECK(view.base().size() == 2);
    CHECK(view.base()[0] == U'A');
}
