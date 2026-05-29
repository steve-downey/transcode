// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/bulk_transcode.hpp>
#include <beman/transcode/detail/bulk_transcode.hpp>

#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <iterator>
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

// ---------------------------------------------------------------------------
// decode_to(codec, range) — random_access path (latin1_codec satisfies it)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable decode_to: ASCII bytes decode correctly", "[pluggable_bulk]") {
    std::string src  = "ABC";
    auto        result = decode_to(latin1_codec{}, std::span<const char>(src));
    REQUIRE(result.size() == 3);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'B');
    CHECK(result[2] == U'C');
}

TEST_CASE("pluggable decode_to: upper-half bytes decode via table", "[pluggable_bulk]") {
    std::array<unsigned char, 2> src{0xC0, 0xC1};
    auto                         result = decode_to(latin1_codec{}, std::span<const unsigned char>(src));
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'\x00C0');
    CHECK(result[1] == U'\x00C1');
}

TEST_CASE("pluggable decode_to: empty input yields empty output", "[pluggable_bulk]") {
    std::string src;
    auto        result = decode_to(latin1_codec{}, std::span<const char>(src));
    CHECK(result.empty());
}

TEST_CASE("pluggable decode_to: vector<char> input", "[pluggable_bulk]") {
    std::vector<char> src{'\x41', '\xC0'};
    auto              result = decode_to(latin1_codec{}, src);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x00C0');
}

// ---------------------------------------------------------------------------
// encode_to(codec, range)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable encode_to: ASCII codepoints encode correctly", "[pluggable_bulk]") {
    std::u32string src = U"ABC";
    auto           result = encode_to(latin1_codec{}, src);
    CHECK(result == "ABC");
}

TEST_CASE("pluggable encode_to: upper-half codepoints encode correctly", "[pluggable_bulk]") {
    std::u32string src{U'\x00C0', U'\x00C1'};
    auto           result = encode_to(latin1_codec{}, src);
    REQUIRE(result.size() == 2);
    CHECK(static_cast<unsigned char>(result[0]) == 0xC0);
    CHECK(static_cast<unsigned char>(result[1]) == 0xC1);
}

TEST_CASE("pluggable encode_to: unmapped codepoint yields replacement '?'", "[pluggable_bulk]") {
    std::u32string src{U'\x1F600'};
    auto           result = encode_to(latin1_codec{}, src);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == '?');
}

TEST_CASE("pluggable encode_to: empty input yields empty output", "[pluggable_bulk]") {
    std::u32string src;
    auto           result = encode_to(latin1_codec{}, src);
    CHECK(result.empty());
}

TEST_CASE("pluggable encode_to: vector<char> container type", "[pluggable_bulk]") {
    std::u32string src = U"Hi";
    auto           result = encode_to<latin1_codec, std::vector<char>>(latin1_codec{}, src);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 'H');
    CHECK(result[1] == 'i');
}

// ---------------------------------------------------------------------------
// decode_into(codec, range, output)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable decode_into: decodes bytes to output iterator", "[pluggable_bulk]") {
    std::string          src{'\x41', '\xC0'};
    std::vector<char32_t> result;
    decode_into(latin1_codec{}, std::span<const char>(src), std::back_inserter(result));
    REQUIRE(result.size() == 2);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'\x00C0');
}

TEST_CASE("pluggable decode_into: empty input writes nothing", "[pluggable_bulk]") {
    std::string          src;
    std::vector<char32_t> result;
    decode_into(latin1_codec{}, std::span<const char>(src), std::back_inserter(result));
    CHECK(result.empty());
}

// ---------------------------------------------------------------------------
// encode_into(codec, range, output)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable encode_into: encodes codepoints to output iterator", "[pluggable_bulk]") {
    std::u32string src{U'A', U'\x00C0'};
    std::string    result;
    encode_into(latin1_codec{}, src, std::back_inserter(result));
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 'A');
    CHECK(static_cast<unsigned char>(result[1]) == 0xC0);
}

TEST_CASE("pluggable encode_into: empty input writes nothing", "[pluggable_bulk]") {
    std::u32string src;
    std::string    result;
    encode_into(latin1_codec{}, src, std::back_inserter(result));
    CHECK(result.empty());
}

// ---------------------------------------------------------------------------
// Round-trip: decode_to then encode_to
// ---------------------------------------------------------------------------

TEST_CASE("pluggable bulk: round-trip decode_to then encode_to", "[pluggable_bulk]") {
    std::string src{'\x41', '\xC0', '\xC1'};
    auto        codepoints = decode_to(latin1_codec{}, std::span<const char>(src));
    auto        back       = encode_to(latin1_codec{}, codepoints);
    CHECK(back == src);
}

// ---------------------------------------------------------------------------
// Consteval tests
// ---------------------------------------------------------------------------

TEST_CASE("pluggable bulk: constexpr decode_to", "[pluggable_bulk]") {
    constexpr auto result = [] {
        std::array<unsigned char, 3> src{0x41, 0xC0, 0xC1};
        std::array<char32_t, 3>      out{};
        auto                         vec = decode_to(latin1_codec{}, src);
        for (int i = 0; i < 3; ++i)
            out[static_cast<std::size_t>(i)] = vec[static_cast<std::size_t>(i)];
        return out;
    }();
    CHECK(constify(result[0]) == U'A');
    CHECK(constify(result[1]) == U'\x00C0');
    CHECK(constify(result[2]) == U'\x00C1');
}

TEST_CASE("pluggable bulk: constexpr encode_to", "[pluggable_bulk]") {
    constexpr auto result = [] {
        std::array<char32_t, 2> src{U'\x00C0', U'\x00C1'};
        std::array<char, 2>     out{};
        auto                    str = encode_to(latin1_codec{}, src);
        for (int i = 0; i < 2; ++i)
            out[static_cast<std::size_t>(i)] = str[static_cast<std::size_t>(i)];
        return out;
    }();
    CHECK(constify(static_cast<unsigned char>(result[0])) == 0xC0u);
    CHECK(constify(static_cast<unsigned char>(result[1])) == 0xC1u);
}

TEST_CASE("pluggable bulk: constexpr decode_into", "[pluggable_bulk]") {
    constexpr auto result = [] {
        std::array<unsigned char, 2> src{0x41, 0xC0};
        std::array<char32_t, 2>      out{};
        decode_into(latin1_codec{}, src, out.data());
        return out;
    }();
    CHECK(constify(result[0]) == U'A');
    CHECK(constify(result[1]) == U'\x00C0');
}

TEST_CASE("pluggable bulk: constexpr encode_into", "[pluggable_bulk]") {
    constexpr auto result = [] {
        std::array<char32_t, 2> src{U'A', U'\x00C0'};
        std::array<char, 2>     out{};
        encode_into(latin1_codec{}, src, out.data());
        return out;
    }();
    CHECK(constify(result[0]) == 'A');
    CHECK(constify(static_cast<unsigned char>(result[1])) == 0xC0u);
}
