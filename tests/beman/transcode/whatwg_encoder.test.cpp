// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/whatwg/transcode_string.hpp>
#include <benchmark/benchmark_fixture.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;
using whatwg::encoding;

namespace {

template <encoding... Cs>
struct encoding_list {};

// Every encoding the Encoding Standard gives an encoder, listed rather than
// derived so that a change to whatwg::has_encoder shows up here.
using encodable = encoding_list<encoding::utf_8,
                                encoding::ibm866,
                                encoding::iso_8859_2,
                                encoding::iso_8859_3,
                                encoding::iso_8859_4,
                                encoding::iso_8859_5,
                                encoding::iso_8859_6,
                                encoding::iso_8859_7,
                                encoding::iso_8859_8,
                                encoding::iso_8859_8_i,
                                encoding::iso_8859_10,
                                encoding::iso_8859_13,
                                encoding::iso_8859_14,
                                encoding::iso_8859_15,
                                encoding::iso_8859_16,
                                encoding::koi8_r,
                                encoding::koi8_u,
                                encoding::macintosh,
                                encoding::windows_874,
                                encoding::windows_1250,
                                encoding::windows_1251,
                                encoding::windows_1252,
                                encoding::windows_1253,
                                encoding::windows_1254,
                                encoding::windows_1255,
                                encoding::windows_1256,
                                encoding::windows_1257,
                                encoding::windows_1258,
                                encoding::x_mac_cyrillic,
                                encoding::gbk,
                                encoding::gb18030,
                                encoding::big5,
                                encoding::shift_jis,
                                encoding::euc_jp,
                                encoding::iso_2022_jp,
                                encoding::euc_kr>;

// The four encodings the Encoding Standard defines no encoder for.
constexpr std::array decode_only{
    encoding::utf_16be, encoding::utf_16le, encoding::replacement, encoding::x_user_defined};

template <typename F, encoding... Cs>
void for_each_encoding(encoding_list<Cs...>, F&& f) {
    (f.template operator()<Cs>(), ...);
}

template <encoding... Cs>
constexpr std::size_t count_of(encoding_list<Cs...>) {
    return sizeof...(Cs);
}
static_assert(count_of(encodable{}) == 36);

template <encoding C>
concept has_encoder_type = requires { typename whatwg::encoder<C>; };

template <typename R>
auto collect(R&& r) {
    std::vector<std::ranges::range_value_t<R>> out;
    for (auto v : r)
        out.push_back(v);
    return out;
}

using scalars_t = std::vector<char32_t>;

// Whether encoding `input` through the codec object and through the existing
// WHATWG adaptor gives the same bytes, and the same errors, in both modes.
template <encoding C>
bool same(const scalars_t& input) {
    const std::span<const char32_t> in{input};
    const auto                      lossy_codec  = collect(in | encode(whatwg::encoder<C>{}));
    const auto                      lossy_whatwg = collect(in | whatwg::encode<C>);
    if (lossy_codec != lossy_whatwg)
        return false;
    const auto report_codec  = collect(in | encode_or_error(whatwg::encoder<C>{}));
    const auto report_whatwg = collect(in | whatwg::encode_or_error<C>);
    return report_codec == report_whatwg;
}

void require_same_everywhere(const scalars_t& input, const char* what) {
    for_each_encoding(encodable{}, [&]<encoding C>() {
        INFO(what << ", encoding " << static_cast<int>(C));
        CHECK(same<C>(input));
    });
}

scalars_t bmp() {
    scalars_t out;
    for (char32_t cp = 0; cp <= 0xFFFF; ++cp)
        if (cp < 0xD800 || cp > 0xDFFF)
            out.push_back(cp);
    return out;
}

scalars_t supplementary_sample() {
    scalars_t out;
    for (char32_t cp = 0x10000; cp <= 0x10FFFF; cp += 31)
        out.push_back(cp);
    out.push_back(0x10FFFF);
    out.push_back(0x1F600);
    out.push_back(0x20000);
    return out;
}

scalars_t ill_formed() {
    return {U'a',
            0xD800,
            U'é',
            0xDBFF,
            0xDC00,
            U'あ',
            0xDFFF,
            0x110000,
            U'z',
            0x7FFFFFFF,
            0xFFFFFFFF,
            U'¥',
            0xD800,
            0x110000};
}

// ISO-2022-JP: ASCII, Roman (U+00A5, U+203E), JIS X 0208, an unencodable
// scalar in each mode, and the end of the input in each mode.
const std::vector<scalars_t>& iso_2022_jp_sequences() {
    static const std::vector<scalars_t> v{
        {},
        {U'a', U'b'},
        {U'¥'},
        {U'‾'},
        {U'あ'},
        {U'a', U'¥', U'‾', U'a'},
        {U'a', U'あ', U'漢', U'a'},
        {U'¥', U'あ', U'¥'},
        {U'あ', U'¥', U'あ', U'a', U'‾'},
        // An unencodable scalar while in each mode.
        {U'a', U'\U0001F600', U'a'},
        {U'¥', U'\U0001F600', U'¥'},
        {U'あ', U'\U0001F600', U'あ'},
        {U'¥', U'\U0001F600'},
        {U'あ', U'\U0001F600'},
        {U'\U0001F600'},
        // Ill-formed UTF-32 while in each mode.
        {U'¥', 0xD800, U'¥'},
        {U'あ', 0x110000, U'あ'},
        {U'あ', 0xD800},
        // The end of the input in each mode.
        {U'a'},
        {U'¥', U'‾'},
        {U'あ', U'漢'},
    };
    return v;
}

} // namespace

// ---------------------------------------------------------------------------
// Properties of the codec objects
// ---------------------------------------------------------------------------

namespace {

template <typename T>
constexpr bool is_flushable = flushable_text_encoder<T>;

template <encoding... Cs>
constexpr bool all_text_encoders(encoding_list<Cs...>) {
    return (text_encoder<whatwg::encoder<Cs>> && ...);
}
static_assert(all_text_encoders(encodable{}));

template <encoding... Cs>
constexpr bool all_equality_comparable(encoding_list<Cs...>) {
    return (std::equality_comparable<whatwg::encoder<Cs>> && ...);
}
static_assert(all_equality_comparable(encodable{}));

template <encoding... Cs>
constexpr bool none_flushable_but_iso_2022_jp(encoding_list<Cs...>) {
    return ((flushable_text_encoder<whatwg::encoder<Cs>> == (Cs == encoding::iso_2022_jp)) && ...);
}
static_assert(none_flushable_but_iso_2022_jp(encodable{}));
static_assert(flushable_text_encoder<whatwg::encoder<encoding::iso_2022_jp>>);
static_assert(!flushable_text_encoder<whatwg::encoder<encoding::shift_jis>>);
static_assert(!flushable_text_encoder<whatwg::encoder<encoding::utf_8>>);

template <encoding... Cs>
constexpr bool all_have_encoder(encoding_list<Cs...>) {
    return (whatwg::has_encoder(Cs) && ...);
}
static_assert(all_have_encoder(encodable{}));

template <encoding... Cs>
constexpr bool all_detected(encoding_list<Cs...>) {
    return (has_encoder_type<Cs> && ...);
}
static_assert(all_detected(encodable{}));
static_assert(has_encoder_type<encoding::shift_jis>);
static_assert(!has_encoder_type<encoding::utf_16be>);
static_assert(!has_encoder_type<encoding::utf_16le>);
static_assert(!has_encoder_type<encoding::replacement>);
static_assert(!has_encoder_type<encoding::x_user_defined>);

// The range properties the codec-object encode_view already has.
template <encoding C>
using encoded_span = decltype(std::declval<std::span<const char32_t>>() | encode(whatwg::encoder<C>{}));

template <encoding... Cs>
constexpr bool all_range_properties(encoding_list<Cs...>) {
    return ((std::ranges::forward_range<encoded_span<Cs>> && std::ranges::range<const encoded_span<Cs>> &&
             std::ranges::borrowed_range<encoded_span<Cs>> && std::ranges::common_range<encoded_span<Cs>>) &&
            ...);
}
static_assert(all_range_properties(encodable{}));

} // namespace

TEST_CASE("whatwg::encoder: the four decode-only encodings have no encoder", "[whatwg_encoder]") {
    for (auto c : decode_only)
        CHECK_FALSE(whatwg::has_encoder(c));
}

// ---------------------------------------------------------------------------
// Differential tests: encode(whatwg::encoder<C>{}) against whatwg::encode<C>
// ---------------------------------------------------------------------------

TEST_CASE("whatwg::encoder: every BMP scalar value, in both modes", "[whatwg_encoder][differential]") {
    require_same_everywhere(bmp(), "bmp");
}

TEST_CASE("whatwg::encoder: supplementary-plane scalar values, in both modes", "[whatwg_encoder][differential]") {
    require_same_everywhere(supplementary_sample(), "supplementary");
}

TEST_CASE("whatwg::encoder: ill-formed UTF-32, in both modes", "[whatwg_encoder][differential]") {
    require_same_everywhere(ill_formed(), "ill-formed");
}

TEST_CASE("whatwg::encoder: ISO-2022-JP mode transitions and the trailing escape", "[whatwg_encoder][differential]") {
    for (const auto& seq : iso_2022_jp_sequences()) {
        CHECK(same<encoding::iso_2022_jp>(seq));
        // The same sequences, through every other encoding, as a plain mix.
        require_same_everywhere(seq, "iso-2022-jp sequence");
    }
}

TEST_CASE("whatwg::encoder: the decoded corpora, in both modes", "[whatwg_encoder][differential]") {
    for (const char* name : {"ar_mars_utf8.txt",
                             "en_mars_utf8.txt",
                             "ja_mars_utf8.txt",
                             "ko_mars_utf8.txt",
                             "ru_mars_utf8.txt",
                             "war_and_peace_utf8.txt",
                             "zh_mars_utf8.txt"}) {
        const auto text = beman::transcoding::bench::load_corpus(name);
        REQUIRE_FALSE(text.empty());
        const auto decoded =
            collect(std::span<const char>{text.data(), text.size()} | whatwg::decode<encoding::utf_8>);
        require_same_everywhere(decoded, name);
    }
}

// ---------------------------------------------------------------------------
// ISO-2022-JP, explicitly
// ---------------------------------------------------------------------------

namespace {
std::string encode_iso_2022_jp(std::u32string_view in) {
    std::string out;
    for (auto b : in | encode(whatwg::encoder<encoding::iso_2022_jp>{}))
        out.push_back(b);
    return out;
}
} // namespace

TEST_CASE("whatwg::encoder<iso_2022_jp>: escapes, replacement and trailing escape", "[whatwg_encoder]") {
    using namespace std::string_literals;
    CHECK(encode_iso_2022_jp(U"ab") == "ab"s);
    CHECK(encode_iso_2022_jp(U"あ") == "\x1B$B\x24\x22\x1B(B"s);
    CHECK(encode_iso_2022_jp(U"¥") == "\x1B(J\x5C\x1B(B"s);
    // The replacement for an unencodable scalar returns to ASCII first.
    CHECK(encode_iso_2022_jp(U"¥\U0001F600") == "\x1B(J\x5C\x1B(B?"s);
    CHECK(encode_iso_2022_jp(U"あ\U0001F600") == "\x1B$B\x24\x22\x1B(B?"s);
    CHECK(encode_iso_2022_jp(U"a\U0001F600") == "a?"s);
}

TEST_CASE("whatwg::encoder<iso_2022_jp>: flush", "[whatwg_encoder]") {
    whatwg::encoder<encoding::iso_2022_jp> e;
    CHECK_FALSE(e.flush().has_value());
    (void)e.encode_one(U'あ');
    auto f = e.flush();
    REQUIRE(f.has_value());
    CHECK(f->count == 3);
    CHECK_FALSE(e.flush().has_value());
}

TEST_CASE("whatwg::encoder: report mode reports an unencodable scalar once", "[whatwg_encoder]") {
    std::vector<std::expected<char, whatwg::error>> out;
    for (auto r : std::u32string_view{U"あ\U0001F600"} | encode_or_error(whatwg::encoder<encoding::iso_2022_jp>{}))
        out.push_back(r);
    // ESC $ B, two bytes, then the error; the state went back to ASCII with
    // the error, so nothing is owed at the end.
    REQUIRE(out.size() == 6);
    CHECK_FALSE(out.back().has_value());
    CHECK(out.back().error() == whatwg::error::unmapped_codepoint);
}

// ---------------------------------------------------------------------------
// Compile-time
// ---------------------------------------------------------------------------

namespace {

template <encoding C>
constexpr std::array<char, 16> encode_in_constant_expression() {
    std::array<char, 16> out{};
    std::size_t          n = 0;
    for (auto b : std::u32string_view{U"aあ¥z"} | encode(whatwg::encoder<C>{}))
        out[n++] = b;
    out[15] = static_cast<char>(n);
    return out;
}

} // namespace

TEST_CASE("whatwg::encoder: encoding in a constant expression", "[whatwg_encoder][constexpr]") {
    // Shift_JIS: a, 0x82 0xA0 (U+3042), 0x5C (U+00A5), z.
    constexpr auto sjis = constify(encode_in_constant_expression<encoding::shift_jis>());
    CHECK(sjis[15] == 5);
    CHECK(sjis[0] == 'a');
    CHECK(static_cast<unsigned char>(sjis[1]) == 0x82);
    CHECK(static_cast<unsigned char>(sjis[2]) == 0xA0);
    CHECK(sjis[3] == '\x5C');
    CHECK(sjis[4] == 'z');

    // ISO-2022-JP: a, ESC $ B 24 22 (U+3042), ESC ( J 5C (U+00A5), ESC ( B z,
    // and no trailing escape because the input ends in ASCII.
    constexpr auto jp = constify(encode_in_constant_expression<encoding::iso_2022_jp>());
    CHECK(jp[15] == 1 + 5 + 4 + 4);
    CHECK(jp[0] == 'a');
    CHECK(jp[1] == '\x1B');
    CHECK(jp[13] == 'z');

    constexpr auto trailing = constify([] {
        std::array<char, 16> out{};
        std::size_t          n = 0;
        for (auto b : std::u32string_view{U"あ"} | encode(whatwg::encoder<encoding::iso_2022_jp>{}))
            out[n++] = b;
        out[15] = static_cast<char>(n);
        return out;
    }());
    CHECK(trailing[15] == 8);
    CHECK(trailing[5] == '\x1B');
    CHECK(trailing[6] == '(');
    CHECK(trailing[7] == 'B');
}
