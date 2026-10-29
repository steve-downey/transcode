// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <iterator>
#include <list>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;
using whatwg::encoding;

namespace {

using bytes_t = std::vector<unsigned char>;

// Every enumerator of whatwg::encoding, listed rather than counted so that a
// new enumerator that is not added here fails the size check below.
constexpr std::array all_encodings{
    encoding::utf_8,        encoding::replacement,  encoding::x_user_defined, encoding::ibm866,
    encoding::iso_8859_2,   encoding::iso_8859_3,   encoding::iso_8859_4,     encoding::iso_8859_5,
    encoding::iso_8859_6,   encoding::iso_8859_7,   encoding::iso_8859_8,     encoding::iso_8859_8_i,
    encoding::iso_8859_10,  encoding::iso_8859_13,  encoding::iso_8859_14,    encoding::iso_8859_15,
    encoding::iso_8859_16,  encoding::koi8_r,       encoding::koi8_u,         encoding::macintosh,
    encoding::windows_874,  encoding::windows_1250, encoding::windows_1251,   encoding::windows_1252,
    encoding::windows_1253, encoding::windows_1254, encoding::windows_1255,   encoding::windows_1256,
    encoding::windows_1257, encoding::windows_1258, encoding::x_mac_cyrillic, encoding::utf_16be,
    encoding::utf_16le,     encoding::gbk,          encoding::gb18030,        encoding::big5,
    encoding::shift_jis,    encoding::euc_jp,       encoding::iso_2022_jp,    encoding::euc_kr,
};
static_assert(all_encodings.size() == 40);
static_assert(static_cast<int>(encoding::euc_kr) + 1 == static_cast<int>(all_encodings.size()));

template <typename R>
auto collect(R&& r) {
    std::vector<std::ranges::range_value_t<R>> out;
    for (auto v : r)
        out.push_back(v);
    return out;
}

// Whether the lossy and the reporting decode of `input` agree with each other,
// element for element, and present only Unicode scalar values.
template <encoding C, typename Input>
bool consistent(const Input& input) {
    const auto lossy  = collect(input | whatwg::decode<C>);
    const auto report = collect(input | whatwg::decode_or_error<C>);
    if (lossy.size() != report.size())
        return false;
    for (std::size_t i = 0; i < lossy.size(); ++i) {
        const char32_t cp = lossy[i];
        if ((cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF)
            return false;
        if (report[i].has_value() ? *report[i] != cp : cp != U'\xFFFD')
            return false;
    }
    return true;
}

// Whether decoding `input` from a contiguous range and from a list, which
// takes the forward-iteration path with no random access, gives one answer.
template <encoding C>
bool same_through_a_list(const bytes_t& input) {
    const std::list<unsigned char> as_list(input.begin(), input.end());
    return collect(input | whatwg::decode<C>) == collect(as_list | whatwg::decode<C>) &&
           collect(input | whatwg::decode_or_error<C>) == collect(as_list | whatwg::decode_or_error<C>);
}

std::string describe(const bytes_t& b) {
    static constexpr char hex[] = "0123456789ABCDEF";
    std::string           s;
    for (auto c : b) {
        s += hex[c >> 4];
        s += hex[c & 15];
        s += ' ';
    }
    return s;
}

// Hand-picked inputs where decoders disagree about state: byte order marks,
// escape sequences, sequences that decode to two code points, and sequences
// cut off by the end of the input.
std::vector<bytes_t> edge_inputs() {
    return {
        {},
        {'A'},
        {'H', 'e', 'l', 'l', 'o'},
        // BOMs alone, followed by text, followed by a truncated sequence
        {0xEF, 0xBB, 0xBF},
        {0xFE, 0xFF},
        {0xFF, 0xFE},
        {0xEF, 0xBB, 0xBF, 'h', 'i'},
        {0xFE, 0xFF, 0x00, 'h', 0x00, 'i'},
        {0xFF, 0xFE, 'h', 0x00, 'i', 0x00},
        {0xEF, 0xBB, 0xBF, 0xE3, 0x81},
        {0xFE, 0xFF, 0xD8, 0x00},
        {0xFF, 0xFE, 0x00, 0xD8},
        {0xFE, 0xFF, 0x00},
        {0xEF, 0xBB, 0xBF, 0xEF, 0xBB, 0xBF, 'x'},
        {0xFE, 0xFF, 0xFE, 0xFF, 0x00, 'x'},
        {'a', 0xEF, 0xBB, 0xBF, 'b'},
        // ISO-2022-JP
        {'A', 0x1B, '(', 'B'},
        {0x1B, '(', 'B'},
        {0x1B},
        {0x1B, '$'},
        {0x1B, '$', 'B'},
        {0x1B, '$', 'B', 0x30, 0x21},
        {0x1B, '$', 'B', 0x30, 0x21, 0x1B, '(', 'B'},
        {0x1B, '$', 'B', 0x30},
        {0x1B, '(', 'I', 0x21, 0x22},
        {0x1B, '(', 'J', 'a', '~'},
        {'x', 0x1B, '$', 'B', 0x30, 0x21, 0x1B, '(', 'B', 'y', 0x1B},
        {0x1B, '$', 'X'},
        {0x0E, 0x0F, 'a'},
        // Big5: sequences that decode to two code points
        {0x88, 0x62},
        {0x88, 0x64},
        {0x88, 0xA3},
        {0x88, 0xA5},
        {0x88, 0x62, 'A'},
        {0x88, 0x64, 0x88, 0x62},
        {'x', 0x88, 0xA5},
        {0x88},
        // UTF-16: odd length, unpaired surrogates, a pair
        {0x00, 0x41, 0x00},
        {0xD8, 0x00},
        {0x00, 0xD8},
        {0xD8, 0x00, 0x00, 0x41},
        {0x00, 0xD8, 'A', 0x00},
        {0xD8, 0x3D, 0xDE, 0x00},
        {0x3D, 0xD8, 0x00, 0xDE},
        {0xDC, 0x00},
        // Truncated multi-byte sequences
        {0xE3},
        {0xE3, 0x81},
        {0xF0, 0x9F, 0x98},
        {0x81},
        {0x81, 0x30},
        {0x81, 0x30, 0x81},
        {0x8E},
        {0x8F, 0xA1},
        {0xA1},
    };
}

template <encoding C>
void check_encoding() {
    INFO("encoding " << static_cast<int>(C));

    for (const auto& in : edge_inputs()) {
        if (!consistent<C>(in))
            FAIL("edge input is inconsistent between the modes: " << describe(in));
        if (!same_through_a_list<C>(in))
            FAIL("edge input differs through a list: " << describe(in));
    }
}

template <std::size_t... I>
void check_all_encodings(std::index_sequence<I...>) {
    (check_encoding<all_encodings[I]>(), ...);
}

// -- property static_asserts ------------------------------------------------

template <encoding C>
constexpr bool is_stateful_encoding =
    C == encoding::iso_2022_jp || C == encoding::utf_16be || C == encoding::utf_16le || C == encoding::gbk ||
    C == encoding::gb18030 || C == encoding::big5;

template <encoding C>
constexpr bool decoder_properties() {
    using D = whatwg::decoder<C>;
    static_assert(text_decoder<D>);
    static_assert(single_byte_text_decoder<D> == detail::random_access_decode_codec<C>);
    static_assert(flushable_text_decoder<D> == is_stateful_encoding<C>);
    static_assert(bom_aware_text_decoder<D> ==
                  (C == encoding::utf_8 || C == encoding::utf_16be || C == encoding::utf_16le));
    static_assert(std::equality_comparable<D>);

    using V = decltype(std::declval<std::span<const char>>() | decode(D{}));
    if constexpr (detail::random_access_decode_codec<C>)
        static_assert(std::ranges::random_access_range<V>);
    else
        static_assert(std::ranges::forward_range<V> && !std::ranges::random_access_range<V>);
    static_assert(std::ranges::borrowed_range<V>);
    static_assert(std::ranges::common_range<V>);
    static_assert(std::ranges::range<const V>);
    return true;
}

template <std::size_t... I>
constexpr bool all_decoder_properties(std::index_sequence<I...>) {
    return (decoder_properties<all_encodings[I]>() && ...);
}
static_assert(all_decoder_properties(std::make_index_sequence<all_encodings.size()>{}));

// -- constant evaluation ----------------------------------------------------

template <encoding C, std::size_t N>
constexpr std::pair<std::array<char32_t, N>, std::size_t> decode_all(const std::array<unsigned char, N>& in) {
    std::array<char32_t, N> out{};
    std::size_t             n = 0;
    for (char32_t cp : in | whatwg::decode<C>)
        out[n++] = cp;
    return {out, n};
}

constexpr std::array<unsigned char, 5> shift_jis_input{0x82, 0xA0, 'A', 0x83, 0x41};
constexpr std::array<unsigned char, 7> utf8_bom_input{0xEF, 0xBB, 0xBF, 'h', 'i', 0xC3, 0xA9};

} // namespace

TEST_CASE("whatwg::decode is consistent between modes and range kinds for every encoding",
          "[transcoding::whatwg::decoder]") {
    check_all_encodings(std::make_index_sequence<all_encodings.size()>{});
}

TEST_CASE("whatwg::decode of edge inputs", "[transcoding::whatwg::decoder]") {
    using cps = std::vector<char32_t>;
    // a byte order mark is how the encoding was chosen, not text
    CHECK(collect(bytes_t{0xFF, 0xFE, 'h', 0x00, 'i', 0x00} | whatwg::decode<encoding::utf_16le>) == cps{U'h', U'i'});
    CHECK(collect(bytes_t{0xFE, 0xFF, 0x00, 'h', 0x00, 'i'} | whatwg::decode<encoding::utf_16be>) == cps{U'h', U'i'});
    CHECK(collect(bytes_t{0xEF, 0xBB, 0xBF, 'h', 'i'} | whatwg::decode<encoding::utf_8>) == cps{U'h', U'i'});
    // Big5: one pair of bytes, two code points
    CHECK(collect(bytes_t{0x88, 0x62} | whatwg::decode<encoding::big5>) == cps{U'\x00CA', U'\x0304'});
    CHECK(collect(bytes_t{0x88, 0x62, 'A'} | whatwg::decode<encoding::big5>) == cps{U'\x00CA', U'\x0304', U'A'});
    // ISO-2022-JP: an escape sequence changes the decoder's mode
    CHECK(collect(bytes_t{0x1B, '$', 'B', 0x30, 0x21, 0x1B, '(', 'B', 'y'} | whatwg::decode<encoding::iso_2022_jp>) ==
          cps{U'\x4E9C', U'y'});
    // truncated input is an error, not a read past the end
    CHECK(collect(bytes_t{0xE3, 0x81} | whatwg::decode<encoding::utf_8>) == cps{U'\xFFFD'});
    CHECK(collect(bytes_t{0x00, 0x41, 0x00} | whatwg::decode<encoding::utf_16be>) == cps{U'A', U'\xFFFD'});
    CHECK(collect(bytes_t{0xD8, 0x00} | whatwg::decode<encoding::utf_16be>) == cps{U'\xFFFD'});
    CHECK(collect(bytes_t{'A'} | whatwg::decode<encoding::windows_1252>) == cps{U'A'});
    CHECK(collect(bytes_t{'A'} | whatwg::decode<encoding::shift_jis>) == cps{U'A'});
    CHECK(collect(bytes_t{'A'} | whatwg::decode<encoding::iso_2022_jp>) == cps{U'A'});
    CHECK(collect(bytes_t{'A'} | whatwg::decode<encoding::replacement>) == cps{U'\xFFFD'});
}

TEST_CASE("whatwg::decoder drops one leading byte order mark", "[transcoding::whatwg::decoder]") {
    const bytes_t two{0xEF, 0xBB, 0xBF, 0xEF, 0xBB, 0xBF, 'x'};
    CHECK(collect(two | decode(whatwg::decoder<encoding::utf_8>{})) == std::vector<char32_t>{U'\xFEFF', U'x'});
    const bytes_t alone{0xEF, 0xBB, 0xBF};
    CHECK(collect(alone | decode(whatwg::decoder<encoding::utf_8>{})).empty());
    // a copy of the decoder made before decoding restarts the rule: each begin() copies a fresh codec
    auto view = two | decode(whatwg::decoder<encoding::utf_8>{});
    CHECK(collect(view) == collect(view));
    using cps = std::vector<char32_t>;
    // UTF-16: a byte order mark of the decoder's own byte order only
    CHECK(collect(bytes_t{0xFE, 0xFF, 0x00, 'A'} | whatwg::decode<encoding::utf_16be>) == cps{U'A'});
    CHECK(collect(bytes_t{0xFF, 0xFE, 'A', 0x00} | whatwg::decode<encoding::utf_16le>) == cps{U'A'});
    CHECK(collect(bytes_t{0xFE, 0xFF} | whatwg::decode<encoding::utf_16be>).empty());
    CHECK(collect(bytes_t{0xFE, 0xFF, 0x00} | whatwg::decode<encoding::utf_16be>) == cps{U'\xFFFD'});
    CHECK(collect(bytes_t{0xFF, 0xFE, 'A', 0x00} | whatwg::decode<encoding::utf_16be>) == cps{U'\xFFFE', U'\x4100'});
    // decode_one never drops one; decode_first does, once
    whatwg::decoder<encoding::utf_8> d;
    auto                             it = alone.begin();
    CHECK(d.decode_one(it, alone.end()).code_point == U'\xFEFF');
    it = alone.begin();
    CHECK(d.decode_first(it, alone.end()).is_empty);
    it = two.begin();
    CHECK(d.decode_first(it, two.end()).code_point == U'\xFEFF');
}

TEST_CASE("whatwg::decoder decodes in a constant expression", "[transcoding::whatwg::decoder]") {
    constexpr auto sjis = constify(decode_all<encoding::shift_jis>(shift_jis_input));
    CHECK(sjis.second == 3);
    CHECK(sjis.first[0] == U'\x3042');
    CHECK(sjis.first[1] == U'A');
    CHECK(sjis.first[2] == U'\x30A2');

    constexpr auto utf8 = constify(decode_all<encoding::utf_8>(utf8_bom_input));
    CHECK(utf8.second == 3);
    CHECK(utf8.first[0] == U'h');
    CHECK(utf8.first[1] == U'i');
    CHECK(utf8.first[2] == U'\xE9');
}

TEST_CASE("whatwg::decoder decode_byte", "[transcoding::whatwg::decoder]") {
    constexpr whatwg::decoder<encoding::windows_1252> w1252;
    CHECK(constify(w1252.decode_byte(0x41)) == U'A');
    CHECK(constify(w1252.decode_byte(0x80)) == U'\x20AC');
    constexpr whatwg::decoder<encoding::x_user_defined> xud;
    CHECK(constify(xud.decode_byte(0x80)) == U'\xF780');
    constexpr whatwg::decoder<encoding::iso_8859_3> latin3;
    CHECK(constify(latin3.decode_byte(0xA5)) == U'\xFFFD');
}

namespace {
constexpr char32_t decode_first_of_bom_then_x() {
    constexpr unsigned char          in[] = {0xEF, 0xBB, 0xBF, 'x'};
    whatwg::decoder<encoding::utf_8> d;
    const unsigned char*             it = in;
    return d.decode_first(it, in + 4).code_point == U'x' ? U'x' : U'?';
}

constexpr bool flush_after_big5_pair() {
    constexpr unsigned char         in[] = {0x88, 0x62};
    whatwg::decoder<encoding::big5> d;
    const unsigned char*            it = in;
    (void)d.decode_one(it, in + 2);
    return d.flush().has_value();
}
} // namespace

TEST_CASE("whatwg::decoder decode_first and flush in a constant expression", "[transcoding::whatwg::decoder]") {
    CHECK(constify(decode_first_of_bom_then_x()) == U'x');
    CHECK(constify(flush_after_big5_pair()));
}

TEST_CASE("whatwg::decoder compares its state", "[transcoding::whatwg::decoder]") {
    const bytes_t                   in{0x88, 0x62};
    whatwg::decoder<encoding::big5> a;
    whatwg::decoder<encoding::big5> b;
    CHECK(a == b);
    auto it  = in.begin();
    auto res = a.decode_one(it, in.end());
    CHECK_FALSE(res.is_error);
    CHECK_FALSE(a == b);
    CHECK(a.flush().has_value());
    CHECK_FALSE(a.flush().has_value());
}
