// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Tests for the all-or-first-error bulk helpers decode_to_or_error and
// encode_to_or_error, in the codec-object form and the WHATWG form.  Unlike
// the collecting helpers they do not need std::ranges::to, so this test builds
// on every toolchain.
//
// The inputs are order-sensitive on purpose: an implementation that collected
// every element and then looked for an error, or that reported the last error
// rather than the first, or that kept decoding past the error, would give a
// different answer to at least one case below.

#include <beman/transcode/bulk_transcode.hpp>
#include <beman/transcode/bulk_transcode.hpp>
#include <beman/transcode/whatwg/bulk.hpp>
#include <beman/transcode/whatwg/bulk.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/detail/table_codec.hpp>
#include <beman/transcode/whatwg/dynamic_codec.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/error.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <iterator>
#include <span>
#include <string>
#include <vector>

namespace whatwg = beman::transcoding::whatwg;
using beman::transcoding::decode_to_or_error;
using beman::transcoding::encode_to_or_error;
using beman::transcoding::tests::constify;

namespace {

// A Latin-1 codec that counts, through a pointer the test supplies, how many
// times it has been asked for a scalar value or a byte.  The view copies the
// codec, so a count kept inside the codec would be the copy's; the pointer
// lets the test see that the helper stopped at the first error rather than
// running the codec over the rest of the input.
struct counting_latin1 {
    int* seen{nullptr};

    constexpr void count() {
        if (seen)
            ++*seen;
    }

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr beman::transcoding::decode_result decode_one(I& current, S) {
        const auto byte = static_cast<unsigned char>(*current);
        ++current;
        count();
        if (byte == 0xFF)
            return {{}, whatwg::error::invalid_byte, true};
        return {static_cast<char32_t>(byte), {}, false};
    }

    constexpr beman::transcoding::encode_result encode_one(char32_t cp) {
        count();
        if (cp > 0xFF)
            return {{}, 0, true};
        return {{static_cast<unsigned char>(cp)}, 1, false};
    }

    friend constexpr bool operator==(const counting_latin1&, const counting_latin1&) = default;
};
static_assert(beman::transcoding::text_decoder<counting_latin1>);
static_assert(beman::transcoding::text_encoder<counting_latin1>);

constexpr bool decode_utf8_constexpr() {
    constexpr std::array<char, 5> source{'A', '\xE2', '\x82', '\xAC', 'B'};
    const auto                    result = whatwg::decode_to_or_error<whatwg::encoding::utf_8>(source);
    return result.has_value() && *result == std::vector<char32_t>{U'A', U'€', U'B'};
}

constexpr bool decode_utf8_error_constexpr() {
    constexpr std::array<char, 3> source{'A', '\xFF', 'B'};
    const auto                    result = whatwg::decode_to_or_error<whatwg::encoding::utf_8>(source);
    return !result.has_value() && result.error() == whatwg::error::invalid_byte;
}

constexpr bool encode_utf8_constexpr() {
    constexpr std::array<char32_t, 2> source{U'A', U'€'};
    const auto                        result = whatwg::encode_to_or_error<whatwg::encoding::utf_8>(source);
    return result.has_value() && *result == std::string{'A', '\xE2', '\x82', '\xAC'};
}

constexpr bool encode_codec_error_constexpr() {
    constexpr std::array<char32_t, 2> source{U'A', U'€'};
    const auto                        result = encode_to_or_error(source, counting_latin1{});
    return !result.has_value() && result.error() == whatwg::error::unmapped_codepoint;
}

} // namespace

// ---------------------------------------------------------------------------
// Codec-object form
// ---------------------------------------------------------------------------

TEST_CASE("decode_to_or_error: valid input yields every scalar value in order", "[bulk][or_error]") {
    const std::string input{'A', 'B', static_cast<char>(0xE9)};
    const auto        result = decode_to_or_error(input, counting_latin1{});
    REQUIRE(result.has_value());
    CHECK(*result == std::vector<char32_t>{U'A', U'B', U'é'});
}

TEST_CASE("decode_to_or_error: the first error is the result, and decoding stops there", "[bulk][or_error]") {
    // Two bad bytes.  The count says the codec saw 'A' and the first 0xFF and
    // nothing after: a helper that decoded everything and then looked for an
    // error would count four.
    const std::string input{'A', static_cast<char>(0xFF), 'B', static_cast<char>(0xFF)};
    int               seen   = 0;
    const auto        result = decode_to_or_error(input, counting_latin1{&seen});
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == whatwg::error::invalid_byte);
    CHECK(seen == 2);
}

TEST_CASE("decode_to_or_error: a UTF-8 error after good text is still the result", "[bulk][or_error]") {
    // "A€" then a truncated three-byte sequence: the valid prefix must not
    // turn into a successful result.
    const std::string input{'A', '\xE2', '\x82', '\xAC', '\xE2', '\x82'};
    const auto        result = decode_to_or_error(input, whatwg::decoder<whatwg::encoding::utf_8>{});
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == whatwg::error::truncated_sequence);
}

TEST_CASE("decode_to_or_error: distinct errors are reported in input order", "[bulk][or_error]") {
    // An encoded surrogate (ED A0 80) and an invalid lead byte (FF), in both
    // orders.  Reversing the input reverses the answer, so a helper that
    // picked the wrong element is caught.
    const std::string surrogate_first{'\xED', '\xA0', '\x80', '\xFF'};
    const std::string invalid_first{'\xFF', '\xED', '\xA0', '\x80'};
    const auto        r1 = decode_to_or_error(surrogate_first, whatwg::decoder<whatwg::encoding::utf_8>{});
    const auto        r2 = decode_to_or_error(invalid_first, whatwg::decoder<whatwg::encoding::utf_8>{});
    REQUIRE_FALSE(r1.has_value());
    REQUIRE_FALSE(r2.has_value());
    CHECK(r1.error() == whatwg::error::surrogate_code_point);
    CHECK(r2.error() == whatwg::error::invalid_byte);
}

TEST_CASE("decode_to_or_error: empty input is an empty vector, not an error", "[bulk][or_error]") {
    const std::span<const char> empty{};
    const auto                  result = decode_to_or_error(empty, counting_latin1{});
    REQUIRE(result.has_value());
    CHECK(result->empty());
}

TEST_CASE("decode_to_or_error: std::byte input is accepted", "[bulk][or_error]") {
    constexpr std::array<std::byte, 2> input{std::byte{0x41}, std::byte{0xA4}};
    const auto result = decode_to_or_error(input, whatwg::decoder<whatwg::encoding::iso_8859_15>{});
    REQUIRE(result.has_value());
    CHECK(*result == std::vector<char32_t>{U'A', U'€'});
}

TEST_CASE("decode_to_or_error: an encoding chosen at run time goes through dynamic_decoder", "[bulk][or_error]") {
    const std::string input{'A', static_cast<char>(0xA4)};
    const auto        result = decode_to_or_error(input, whatwg::dynamic_decoder(whatwg::encoding::iso_8859_15));
    REQUIRE(result.has_value());
    CHECK(*result == std::vector<char32_t>{U'A', U'€'});
}

TEST_CASE("encode_to_or_error: valid input yields every byte in order", "[bulk][or_error]") {
    const std::u32string input{U'A', U'é', U'B'};
    const auto           result = encode_to_or_error(input, counting_latin1{});
    REQUIRE(result.has_value());
    CHECK(*result == std::string{'A', static_cast<char>(0xE9), 'B'});
}

TEST_CASE("encode_to_or_error: an unmapped scalar value is the result", "[bulk][or_error]") {
    const std::u32string input{U'A', U'€', U'B'};
    const auto           result = encode_to_or_error(input, counting_latin1{});
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == whatwg::error::unmapped_codepoint);
}

TEST_CASE("encode_to_or_error: ill-formed UTF-32 input is reported before the encoder sees it", "[bulk][or_error]") {
    const std::u32string surrogate{U'A', static_cast<char32_t>(0xD800), U'B'};
    const std::u32string too_large{U'A', static_cast<char32_t>(0x110000), U'B'};
    const auto           r1 = encode_to_or_error(surrogate, counting_latin1{});
    const auto           r2 = encode_to_or_error(too_large, counting_latin1{});
    REQUIRE_FALSE(r1.has_value());
    REQUIRE_FALSE(r2.has_value());
    CHECK(r1.error() == whatwg::error::surrogate_code_point);
    CHECK(r2.error() == whatwg::error::out_of_range);
}

TEST_CASE("encode_to_or_error: the container is the caller's choice", "[bulk][or_error]") {
    const std::u32string input{U'H', U'i'};
    const auto           result = encode_to_or_error<std::vector<char>>(input, counting_latin1{});
    REQUIRE(result.has_value());
    CHECK(*result == std::vector<char>{'H', 'i'});
}

TEST_CASE("encode_to_or_error: a stateful encoder's flush is part of the result", "[bulk][or_error]") {
    // ISO-2022-JP shifts out of ASCII for the kana and owes the escape back.
    const std::u32string input{U'A', U'あ'};
    const auto           result = encode_to_or_error(input, whatwg::encoder<whatwg::encoding::iso_2022_jp>{});
    REQUIRE(result.has_value());
    CHECK(*result == std::string{'A', '\x1B', '$', 'B', '$', '"', '\x1B', '(', 'B'});
}

TEST_CASE("encode_to_or_error: input after the first error is not examined", "[bulk][or_error]") {
    // The encoder saw 'A' and the first U+20AC; 'B' and the second U+20AC
    // were never offered to it.
    const std::u32string input{U'A', U'\u20AC', U'B', U'\u20AC'};
    int                  seen   = 0;
    const auto           result = encode_to_or_error(input, counting_latin1{&seen});
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == whatwg::error::unmapped_codepoint);
    CHECK(seen == 2);
}

TEST_CASE("encode_to_or_error: ill-formed UTF-32 never reaches the encoder", "[bulk][or_error]") {
    const std::u32string input{U'A', static_cast<char32_t>(0xD800), U'B'};
    int                  seen   = 0;
    const auto           result = encode_to_or_error(input, counting_latin1{&seen});
    REQUIRE_FALSE(result.has_value());
    CHECK(seen == 1);
}

// ---------------------------------------------------------------------------
// WHATWG form
// ---------------------------------------------------------------------------

TEST_CASE("whatwg::decode_to_or_error: the whole text or the first error", "[bulk][or_error]") {
    const std::string good{'A', '\xE2', '\x82', '\xAC'};
    const std::string bad{'A', '\xE2', '\x82', '\xAC', '\xC0', 'B'};
    const auto        r1 = whatwg::decode_to_or_error<whatwg::encoding::utf_8>(good);
    const auto        r2 = whatwg::decode_to_or_error<whatwg::encoding::utf_8>(bad);
    REQUIRE(r1.has_value());
    CHECK(*r1 == std::vector<char32_t>{U'A', U'€'});
    REQUIRE_FALSE(r2.has_value());
    CHECK(r2.error() == whatwg::error::invalid_byte);
}

TEST_CASE("whatwg::decode_to_or_error: a single-byte encoding's unmapped byte", "[bulk][or_error]") {
    const std::string input{'A', static_cast<char>(0xA1)};
    const auto        result = whatwg::decode_to_or_error<whatwg::encoding::iso_8859_6>(input);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == whatwg::error::invalid_byte);
}

TEST_CASE("whatwg::encode_to_or_error: the whole text or the first error", "[bulk][or_error]") {
    const std::u32string good{U'A', U'€'};
    const std::u32string bad{U'A', U'中'};
    const auto           r1 = whatwg::encode_to_or_error<whatwg::encoding::windows_1252>(good);
    const auto           r2 = whatwg::encode_to_or_error<whatwg::encoding::windows_1252>(bad);
    REQUIRE(r1.has_value());
    CHECK(*r1 == std::string{'A', static_cast<char>(0x80)});
    REQUIRE_FALSE(r2.has_value());
    CHECK(r2.error() == whatwg::error::unmapped_codepoint);
}

TEST_CASE("whatwg::encode_to_or_error: alternate container", "[bulk][or_error]") {
    const std::u32string input{U'H', U'i'};
    const auto           result = whatwg::encode_to_or_error<whatwg::encoding::utf_8, std::vector<char>>(input);
    REQUIRE(result.has_value());
    CHECK(*result == std::vector<char>{'H', 'i'});
}

// ---------------------------------------------------------------------------
// Compile-time evaluation
// ---------------------------------------------------------------------------

TEST_CASE("or_error bulk helpers are constexpr", "[bulk][or_error][constexpr]") {
    CHECK(constify(decode_utf8_constexpr()));
    CHECK(constify(decode_utf8_error_constexpr()));
    CHECK(constify(encode_utf8_constexpr()));
    CHECK(constify(encode_codec_error_constexpr()));
}
