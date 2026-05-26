// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/decode_view.hpp>

#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <optional>
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

// Sparse table with a hole at index 0
inline constexpr std::array<char32_t, 128> sparse_table = [] {
    std::array<char32_t, 128> t{};
    t[1] = U'\x00A1';
    t[2] = U'\x00A2';
    return t;
}();

using sparse_codec = table_codec<sparse_table>;

// A stateful codec: XORs each byte with incrementing state
struct xor_codec {
    int counter = 0;

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, [[maybe_unused]] S end) {
        auto byte = static_cast<unsigned char>(*current);
        ++current;
        ++counter;
        return {static_cast<char32_t>(byte ^ counter), {}, false};
    }
};

static_assert(decode_codec<xor_codec>);
static_assert(!random_access_decode_codec_type<xor_codec>);

// ---------------------------------------------------------------------------
// decode_view with table_codec via pipe
// ---------------------------------------------------------------------------

TEST_CASE("decode_view: table_codec pipe decode ASCII", "[decode_view]") {
    std::string src = "Hello";
    std::u32string result;
    for (char32_t cp : std::span<const char>(src) | decode(latin1_codec{}))
        result.push_back(cp);
    CHECK(result == U"Hello");
}

TEST_CASE("decode_view: table_codec pipe decode upper-half", "[decode_view]") {
    std::string src{'\xC0', '\xC1'}; // Latin-1: U+00C0, U+00C1
    std::u32string result;
    for (char32_t cp : std::span<const char>(src) | decode(latin1_codec{}))
        result.push_back(cp);
    CHECK(result.size() == 2);
    CHECK(result[0] == U'\x00C0');
    CHECK(result[1] == U'\x00C1');
}

TEST_CASE("decode_view: sparse table replaces unmapped with U+FFFD", "[decode_view]") {
    std::string src{'\x80'}; // index 0 is unmapped in sparse_table
    std::u32string result;
    for (char32_t cp : std::span<const char>(src) | decode(sparse_codec{}))
        result.push_back(cp);
    CHECK(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("decode_view: empty input produces empty output", "[decode_view]") {
    std::string src;
    std::u32string result;
    for (char32_t cp : std::span<const char>(src) | decode(latin1_codec{}))
        result.push_back(cp);
    CHECK(result.empty());
}

// ---------------------------------------------------------------------------
// decode_or_error_view with table_codec
// ---------------------------------------------------------------------------

TEST_CASE("decode_or_error_view: table_codec success", "[decode_view]") {
    std::string src{'\x41', '\xC0'}; // A, then Latin-1 0xC0
    auto view = std::span<const char>(src) | decode_or_error(latin1_codec{});
    auto it   = view.begin();
    auto end  = view.end();

    auto r1 = *it;
    CHECK(r1.has_value());
    CHECK(r1.value() == U'A');

    ++it;
    auto r2 = *it;
    CHECK(r2.has_value());
    CHECK(r2.value() == U'\x00C0');

    ++it;
    CHECK(it == end);
}

TEST_CASE("decode_or_error_view: reports errors", "[decode_view]") {
    std::string src{'\x80'}; // unmapped in sparse
    auto view = std::span<const char>(src) | decode_or_error(sparse_codec{});
    auto it   = view.begin();

    auto r = *it;
    CHECK(!r.has_value());
    CHECK(r.error() == decode_error::invalid_byte);
}

// ---------------------------------------------------------------------------
// decode_view with stateful codec
// ---------------------------------------------------------------------------

TEST_CASE("decode_view: stateful codec via pipe", "[decode_view]") {
    std::string src{'\x10', '\x10', '\x10'};
    std::vector<char32_t> result;
    for (char32_t cp : std::span<const char>(src) | decode(xor_codec{}))
        result.push_back(cp);
    CHECK(result.size() == 3);
    CHECK(result[0] == char32_t(0x10 ^ 1));
    CHECK(result[1] == char32_t(0x10 ^ 2));
    CHECK(result[2] == char32_t(0x10 ^ 3));
}

// ---------------------------------------------------------------------------
// Random-access path: table_codec with random_access_range
// ---------------------------------------------------------------------------

TEST_CASE("decode_view: random-access path for table_codec", "[decode_view]") {
    std::vector<unsigned char> src{0x41, 0x42, 0xC0};
    auto view = src | decode(latin1_codec{});

    CHECK(view.size() == 3);
    auto it = view.begin();
    CHECK(it[0] == U'A');
    CHECK(it[1] == U'B');
    CHECK(it[2] == U'\x00C0');
}

TEST_CASE("decode_view: random-access iterator arithmetic", "[decode_view]") {
    std::vector<unsigned char> src{0x41, 0x42, 0x43, 0x44};
    auto view = src | decode(latin1_codec{});
    auto it   = view.begin();

    CHECK(*(it + 2) == U'C');
    CHECK(*(view.end() - 1) == U'D');
    CHECK(view.end() - view.begin() == 4);
}

// ---------------------------------------------------------------------------
// Constexpr tests
// ---------------------------------------------------------------------------

TEST_CASE("decode_view: constexpr table decode_byte", "[decode_view]") {
    CHECK(constify(latin1_codec{}.decode_byte(0x41)) == U'A');
    CHECK(constify(latin1_codec{}.decode_byte(0xC0)) == U'\x00C0');
    CHECK(constify(latin1_codec{}.decode_byte(0xFF)) == U'\x00FF');
}

TEST_CASE("decode_view: constexpr random-access indexing", "[decode_view]") {
    constexpr auto cp0 = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xFF};
        auto                         view = src | decode(latin1_codec{});
        return view.begin()[0];
    }();
    constexpr auto cp1 = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xFF};
        auto                         view = src | decode(latin1_codec{});
        return view.begin()[1];
    }();
    constexpr auto cp2 = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xFF};
        auto                         view = src | decode(latin1_codec{});
        return view.begin()[2];
    }();

    CHECK(constify(cp0) == U'A');
    CHECK(constify(cp1) == U'\x00C0');
    CHECK(constify(cp2) == U'\x00FF');
}

// ---------------------------------------------------------------------------
// Mixed: std::byte input
// ---------------------------------------------------------------------------

TEST_CASE("decode_view: std::byte input range", "[decode_view]") {
    std::array<std::byte, 2> src{std::byte{0x41}, std::byte{0xC0}};
    std::u32string result;
    for (char32_t cp : src | decode(latin1_codec{}))
        result.push_back(cp);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x00C0');
}
