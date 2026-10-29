// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode_view.hpp>
#include <beman/transcode/transcode_view.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/detail/table_codec.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/transcode_view.hpp>

#include <tests/beman/transcode/test_utilities.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
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
// Basic transcode via pipe adaptor
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: ASCII identity round-trip via pipe", "[pluggable_transcode]") {
    std::string src = "Hello";
    std::string result;
    for (char c : src | transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    CHECK(result == "Hello");
}

TEST_CASE("pluggable transcode: upper-half identity round-trip via pipe", "[pluggable_transcode]") {
    std::string src{'\xC0', '\xC1', '\xC2'};
    std::string result;
    for (char c : src | transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    CHECK(result == src);
}

TEST_CASE("pluggable transcode: empty input via pipe", "[pluggable_transcode]") {
    std::string src;
    std::string result;
    for (char c : src | transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    CHECK(result.empty());
}

TEST_CASE("pluggable transcode: mixed ASCII and upper-half bytes", "[pluggable_transcode]") {
    std::string src{'\x41', '\xC0', '\x42', '\xC1'};
    std::string result;
    for (char c : src | transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    REQUIRE(result.size() == 4);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0xC0u);
    CHECK(result[2] == '\x42');
    CHECK(static_cast<unsigned char>(result[3]) == 0xC1u);
}

// ---------------------------------------------------------------------------
// pluggable_transcode closure called directly (not pipe)
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: operator() called directly", "[pluggable_transcode]") {
    std::string src     = "Hi";
    auto        closure = transcode(latin1_codec{}, latin1_codec{});
    std::string result;
    for (char c : closure(src))
        result += c;
    CHECK(result == "Hi");
}

// ---------------------------------------------------------------------------
// span input
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: span<const char> input", "[pluggable_transcode]") {
    std::string           src{'\x41', '\xC0', '\xC1'};
    std::span<const char> sp(src);
    std::string           result;
    for (char c : sp | transcode(latin1_codec{}, latin1_codec{}))
        result += c;
    REQUIRE(result.size() == 3);
    CHECK(result[0] == '\x41');
    CHECK(static_cast<unsigned char>(result[1]) == 0xC0u);
    CHECK(static_cast<unsigned char>(result[2]) == 0xC1u);
}

// ---------------------------------------------------------------------------
// Consteval tests
// ---------------------------------------------------------------------------

TEST_CASE("pluggable transcode: constexpr ASCII round-trip", "[pluggable_transcode]") {
    constexpr auto result = [] {
        std::array<unsigned char, 3> src{0x41, 0x42, 0x43};
        std::array<char, 3>          out{};
        int                          idx = 0;
        for (char c : src | transcode(latin1_codec{}, latin1_codec{}))
            out[static_cast<std::size_t>(idx++)] = c;
        return out;
    }();
    CHECK(constify(result[0]) == 'A');
    CHECK(constify(result[1]) == 'B');
    CHECK(constify(result[2]) == 'C');
}

TEST_CASE("pluggable transcode: constexpr upper-half round-trip", "[pluggable_transcode]") {
    constexpr auto result = [] {
        std::array<unsigned char, 2> src{0xC0, 0xC1};
        std::array<unsigned char, 2> out{};
        int                          idx = 0;
        for (char c : src | transcode(latin1_codec{}, latin1_codec{}))
            out[static_cast<std::size_t>(idx++)] = static_cast<unsigned char>(c);
        return out;
    }();
    CHECK(constify(result[0]) == 0xC0u);
    CHECK(constify(result[1]) == 0xC1u);
}

// ---------------------------------------------------------------------------
// Mixing the library's codec objects with a program's own
// ---------------------------------------------------------------------------

namespace {

// ISO-8859-1: each byte is the scalar value of the same number.
struct custom_latin1 {
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, [[maybe_unused]] S end) const {
        const auto byte = static_cast<unsigned char>(*current);
        ++current;
        return {char32_t{byte}, {}, false};
    }

    constexpr encode_result encode_one(char32_t cp) const {
        if (cp > 0xFF)
            return {{}, 0, true};
        encode_result result{};
        result.bytes[0] = static_cast<unsigned char>(cp);
        result.count    = 1;
        return result;
    }
};

static_assert(text_decoder<custom_latin1> && text_encoder<custom_latin1>);

constexpr std::string shift_jis_to_custom(std::string_view bytes) {
    std::string out;
    for (char b : bytes | transcode(whatwg::decoder<whatwg::encoding::shift_jis>{}, custom_latin1{}))
        out.push_back(b);
    return out;
}

} // namespace

TEST_CASE("pluggable transcode: library decoder into a custom encoder", "[pluggable_transcode]") {
    // 'A', U+3042 (not representable in ISO-8859-1), 0xB1 (halfwidth katakana A, U+FF71), 'B'
    CHECK(shift_jis_to_custom("A\x82\xA0\xB1"
                              "B") == "A??B");
    CHECK(shift_jis_to_custom("abc") == "abc");
}

TEST_CASE("pluggable transcode: custom decoder into a library encoder", "[pluggable_transcode]") {
    std::string src{'a', '\xE9'};
    std::string result;
    for (char b : src | transcode(custom_latin1{}, whatwg::encoder<whatwg::encoding::utf_8>{}))
        result.push_back(b);
    CHECK(result == "a\xC3\xA9");
}

TEST_CASE("pluggable transcode: constexpr mixed pipeline", "[pluggable_transcode]") {
    CHECK(constify(shift_jis_to_custom("A\x82\xA0"
                                       "B") == "A?B"));
}

namespace {
// whatwg::transcode<F, T> and transcode(whatwg::decoder<F>{}, whatwg::encoder<T>{}) are the same adaptor.
template <whatwg::encoding F, whatwg::encoding T, typename R>
constexpr bool same_pipeline_type() {
    return std::is_same_v<decltype(std::declval<R>() | whatwg::transcode<F, T>),
                          decltype(std::declval<R>() | transcode(whatwg::decoder<F>{}, whatwg::encoder<T>{}))>;
}
} // namespace

TEST_CASE("pluggable transcode: whatwg::transcode is transcode(decoder, encoder)", "[pluggable_transcode]") {
    using whatwg::encoding;
    using bytes = std::span<const char>;
    static_assert(same_pipeline_type<encoding::utf_8, encoding::windows_1252, bytes>());
    static_assert(same_pipeline_type<encoding::shift_jis, encoding::utf_8, bytes>());
    static_assert(same_pipeline_type<encoding::windows_1252, encoding::iso_8859_2, bytes>());
    // Flushing target.
    static_assert(same_pipeline_type<encoding::utf_8, encoding::iso_2022_jp, bytes>());
    static_assert(same_pipeline_type<encoding::utf_8, encoding::iso_2022_jp, std::string&>());

    // Single-byte to single-byte over a span is random access end to end.
    using sb = decltype(std::declval<bytes>() | whatwg::transcode<encoding::windows_1252, encoding::iso_8859_2>);
    static_assert(std::ranges::random_access_range<sb>);
    CHECK(true);
}

TEST_CASE("pluggable transcode: whatwg::transcode agrees with decode | encode", "[pluggable_transcode]") {
    std::string src{'a', '\x80', '\xE9'};
    std::string via_transcode;
    std::string via_pipeline;
    using whatwg::encoding;
    for (char b : std::span<const char>(src) | whatwg::transcode<encoding::windows_1252, encoding::iso_8859_2>)
        via_transcode.push_back(b);
    for (char b :
         std::span<const char>(src) | whatwg::decode<encoding::windows_1252> | whatwg::encode<encoding::iso_8859_2>)
        via_pipeline.push_back(b);
    CHECK(via_transcode == via_pipeline);
}

TEST_CASE("pluggable transcode: ISO-2022-JP target flushes", "[pluggable_transcode]") {
    // U+3042 (hiragana A) from UTF-8: ESC $ B 0x24 0x22, then the flush returns to ASCII: ESC ( B
    std::string src{'\xE3', '\x81', '\x82'};
    std::string result;
    for (char b :
         std::span<const char>(src) | whatwg::transcode<whatwg::encoding::utf_8, whatwg::encoding::iso_2022_jp>)
        result.push_back(b);
    CHECK(result == "\x1B$B$\"\x1B(B");
}
