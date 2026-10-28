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
#include <filesystem>
#include <fstream>
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

// Whether the codec-object decode and the enumerator decode yield the same
// sequence for `input`, in both error modes.
template <encoding C, typename Input>
bool same(const Input& input) {
    const auto lossy_new  = collect(input | decode(whatwg::decoder<C>{}));
    const auto lossy_old  = collect(input | whatwg::decode<C>);
    const auto report_new = collect(input | decode_or_error(whatwg::decoder<C>{}));
    const auto report_old = collect(input | whatwg::decode_or_error<C>);
    return lossy_new == lossy_old && report_new == report_old;
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

bytes_t read_file(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary);
    return bytes_t(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

template <encoding C>
void check_encoding() {
    INFO("encoding " << static_cast<int>(C));

    // every single byte, and every lead byte >= 0x80 with every trail byte
    for (int b = 0; b < 256; ++b) {
        const bytes_t in{static_cast<unsigned char>(b)};
        if (!same<C>(in))
            FAIL("single byte differs: " << describe(in));
    }
    for (int lead = 0x80; lead < 256; ++lead) {
        for (int trail = 0; trail < 256; ++trail) {
            const bytes_t in{static_cast<unsigned char>(lead), static_cast<unsigned char>(trail)};
            if (!same<C>(in))
                FAIL("two bytes differ: " << describe(in));
        }
    }

    for (const auto& in : edge_inputs()) {
        if (!same<C>(in))
            FAIL("edge input differs: " << describe(in));
        // the same through a range that is not contiguous or random access
        const std::list<unsigned char> as_list(in.begin(), in.end());
        if (!same<C>(as_list))
            FAIL("edge input differs through a list: " << describe(in));
    }
}

template <encoding C>
void check_corpus(const bytes_t& corpus, const std::string& name) {
    INFO("encoding " << static_cast<int>(C) << ", corpus " << name);
    CHECK(same<C>(corpus));
}

template <std::size_t... I>
void check_all_encodings(std::index_sequence<I...>) {
    (check_encoding<all_encodings[I]>(), ...);
}

template <std::size_t... I>
void check_all_corpora(const bytes_t& corpus, const std::string& name, std::index_sequence<I...>) {
    (check_corpus<all_encodings[I]>(corpus, name), ...);
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
    static_assert(std::equality_comparable<D>);

    using V = decltype(std::declval<std::span<const char>>() | decode(D{}));
    using O = decltype(std::declval<std::span<const char>>() | whatwg::decode<C>);
    if constexpr (detail::random_access_decode_codec<C>)
        static_assert(std::ranges::random_access_range<V>);
    else
        static_assert(std::ranges::forward_range<V> && !std::ranges::random_access_range<V>);
    static_assert(std::ranges::borrowed_range<V>);
    static_assert(std::ranges::common_range<V>);
    static_assert(std::ranges::range<const V>);
    // what step 09 must preserve: the replacement for the old view has the old view's properties
    static_assert(std::ranges::random_access_range<V> == std::ranges::random_access_range<O>);
    static_assert(std::ranges::forward_range<V> == std::ranges::forward_range<O>);
    static_assert(std::ranges::borrowed_range<V> == std::ranges::borrowed_range<O>);
    static_assert(std::ranges::common_range<V> == std::ranges::common_range<O>);
    static_assert(std::ranges::range<const V> == std::ranges::range<const O>);
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
    for (char32_t cp : in | decode(whatwg::decoder<C>{}))
        out[n++] = cp;
    return {out, n};
}

constexpr std::array<unsigned char, 5> shift_jis_input{0x82, 0xA0, 'A', 0x83, 0x41};
constexpr std::array<unsigned char, 7> utf8_bom_input{0xEF, 0xBB, 0xBF, 'h', 'i', 0xC3, 0xA9};

} // namespace

TEST_CASE("whatwg::decoder is equivalent to whatwg::decode for every encoding", "[transcoding::whatwg::decoder]") {
    check_all_encodings(std::make_index_sequence<all_encodings.size()>{});
}

TEST_CASE("whatwg::decoder is equivalent to whatwg::decode over the benchmark corpora",
          "[transcoding::whatwg::decoder]") {
    const std::filesystem::path dir{BENCHMARK_CORPUS_DIR};
    REQUIRE(std::filesystem::is_directory(dir));
    std::size_t files = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        const auto name = entry.path().filename().string();
        if (!(name.ends_with(".bin") || name.ends_with("_utf8.txt")))
            continue;
        ++files;
        check_all_corpora(read_file(entry.path()), name, std::make_index_sequence<all_encodings.size()>{});
    }
    CHECK(files >= 10);
}

TEST_CASE("whatwg::decoder drops one leading byte order mark", "[transcoding::whatwg::decoder]") {
    const bytes_t two{0xEF, 0xBB, 0xBF, 0xEF, 0xBB, 0xBF, 'x'};
    CHECK(collect(two | decode(whatwg::decoder<encoding::utf_8>{})) == std::vector<char32_t>{U'\xFEFF', U'x'});
    const bytes_t alone{0xEF, 0xBB, 0xBF};
    CHECK(collect(alone | decode(whatwg::decoder<encoding::utf_8>{})).empty());
    // a copy of the decoder made before decoding restarts the rule: each begin() copies a fresh codec
    auto view = two | decode(whatwg::decoder<encoding::utf_8>{});
    CHECK(collect(view) == collect(view));
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
