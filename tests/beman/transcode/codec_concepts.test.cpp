// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/codec_concepts.hpp>
#include <beman/transcode/detail/codec_concepts.hpp>

#include <beman/transcode/detail/table_codec.hpp>
#include <beman/transcode/detail/table_codec.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <optional>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

// A minimal test table: bytes 0x80-0xFF map to U+0080-U+00FF (Latin-1 supplement)
inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = table_codec<latin1_upper>;

// A table with holes (position 0 is unmapped)
inline constexpr std::array<char32_t, 128> sparse_table = [] {
    std::array<char32_t, 128> t{};
    t[1] = U'\x00A1'; // 0x81 -> inverted exclamation
    t[2] = U'\x00A2'; // 0x82 -> cent sign
    return t;
}();

using sparse_codec = table_codec<sparse_table>;

// A stateful codec for concept testing
struct stateful_test_codec {
    int state = 0;

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end) {
        if (current == end)
            return {{}, decode_error::truncated_sequence, true};
        auto byte = static_cast<unsigned char>(*current);
        ++current;
        ++state;
        return {static_cast<char32_t>(byte + state), {}, false};
    }

    constexpr std::optional<decode_result> flush() {
        if (state > 0) {
            state = 0;
            return std::nullopt;
        }
        return std::nullopt;
    }
};

// A type that does NOT satisfy decode_codec (missing decode_one)
struct not_a_codec {};

// ---------------------------------------------------------------------------
// Concept satisfaction: static_assert checks
// ---------------------------------------------------------------------------

static_assert(decode_codec<latin1_codec>);
static_assert(random_access_decode_codec_type<latin1_codec>);
static_assert(encode_codec<latin1_codec>);
static_assert(!flushable_decode_codec<latin1_codec>);

static_assert(decode_codec<sparse_codec>);
static_assert(random_access_decode_codec_type<sparse_codec>);
static_assert(encode_codec<sparse_codec>);

static_assert(decode_codec<stateful_test_codec>);
static_assert(!random_access_decode_codec_type<stateful_test_codec>);
static_assert(!encode_codec<stateful_test_codec>);
static_assert(flushable_decode_codec<stateful_test_codec>);

static_assert(!decode_codec<not_a_codec>);
static_assert(!encode_codec<not_a_codec>);
static_assert(!decode_codec<int>);

// ---------------------------------------------------------------------------
// table_codec: decode tests
// ---------------------------------------------------------------------------

TEST_CASE("table_codec: ASCII passthrough", "[codec_concepts]") {
    latin1_codec                 codec;
    std::array<unsigned char, 3> input{0x41, 0x42, 0x43}; // ABC
    auto                         it  = input.begin();
    auto                         end = input.end();

    auto r1 = codec.decode_one(it, end);
    CHECK(!r1.is_error);
    CHECK(r1.code_point == U'A');

    auto r2 = codec.decode_one(it, end);
    CHECK(!r2.is_error);
    CHECK(r2.code_point == U'B');

    auto r3 = codec.decode_one(it, end);
    CHECK(!r3.is_error);
    CHECK(r3.code_point == U'C');
}

TEST_CASE("table_codec: upper-half decode", "[codec_concepts]") {
    latin1_codec                 codec;
    std::array<unsigned char, 2> input{0x80, 0xFF};
    auto                         it  = input.begin();
    auto                         end = input.end();

    auto r1 = codec.decode_one(it, end);
    CHECK(!r1.is_error);
    CHECK(r1.code_point == U'\x0080');

    auto r2 = codec.decode_one(it, end);
    CHECK(!r2.is_error);
    CHECK(r2.code_point == U'\x00FF');
}

TEST_CASE("table_codec: unmapped byte returns error", "[codec_concepts]") {
    sparse_codec                 codec;
    std::array<unsigned char, 1> input{0x80}; // index 0 is zero (unmapped)
    auto                         it  = input.begin();
    auto                         end = input.end();

    auto r = codec.decode_one(it, end);
    CHECK(r.is_error);
    CHECK(r.error == decode_error::invalid_byte);
}

TEST_CASE("table_codec: decode_byte random access", "[codec_concepts]") {
    latin1_codec codec;
    CHECK(codec.decode_byte(0x41) == U'A');
    CHECK(codec.decode_byte(0x80) == U'\x0080');
    CHECK(codec.decode_byte(0xFF) == U'\x00FF');

    sparse_codec sparse;
    CHECK(sparse.decode_byte(0x80) == U'\xFFFD'); // unmapped
    CHECK(sparse.decode_byte(0x81) == U'\x00A1');
}

// ---------------------------------------------------------------------------
// table_codec: encode tests
// ---------------------------------------------------------------------------

TEST_CASE("table_codec: encode ASCII", "[codec_concepts]") {
    latin1_codec codec;
    auto         r = codec.encode_one(U'A');
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0x41);
}

TEST_CASE("table_codec: encode upper-half", "[codec_concepts]") {
    latin1_codec codec;
    auto         r = codec.encode_one(U'\x00C0'); // Latin capital A with grave
    CHECK(!r.is_error);
    CHECK(r.count == 1);
    CHECK(r.bytes[0] == 0xC0); // 0x80 + 0x40
}

TEST_CASE("table_codec: encode unmapped codepoint", "[codec_concepts]") {
    sparse_codec codec;
    auto         r = codec.encode_one(U'\x1234'); // not in sparse_table
    CHECK(r.is_error);
    CHECK(r.count == 0);
}

// ---------------------------------------------------------------------------
// table_codec: constexpr tests
// ---------------------------------------------------------------------------

TEST_CASE("table_codec: constexpr decode_byte", "[codec_concepts]") {
    CHECK(constify(latin1_codec{}.decode_byte(0x41)) == U'A');
    CHECK(constify(latin1_codec{}.decode_byte(0x80)) == U'\x0080');
    CHECK(constify(sparse_codec{}.decode_byte(0x80)) == U'\xFFFD');
}

TEST_CASE("table_codec: constexpr encode_one", "[codec_concepts]") {
    CHECK(constify(latin1_codec{}.encode_one(U'A').bytes[0]) == 0x41);
    CHECK(constify(latin1_codec{}.encode_one(U'A').count) == 1);
    CHECK(constify(latin1_codec{}.encode_one(U'A').is_error) == false);
}

// ---------------------------------------------------------------------------
// stateful_codec: basic test
// ---------------------------------------------------------------------------

TEST_CASE("stateful_test_codec: state advances", "[codec_concepts]") {
    stateful_test_codec          codec;
    std::array<unsigned char, 3> input{0x10, 0x20, 0x30};
    auto                         it  = input.begin();
    auto                         end = input.end();

    auto r1 = codec.decode_one(it, end);
    CHECK(!r1.is_error);
    CHECK(r1.code_point == char32_t(0x10 + 1)); // byte + state(1)

    auto r2 = codec.decode_one(it, end);
    CHECK(!r2.is_error);
    CHECK(r2.code_point == char32_t(0x20 + 2)); // byte + state(2)
}
