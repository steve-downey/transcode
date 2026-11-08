// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/dynamic_codec.hpp>
#include <beman/transcode/whatwg/dynamic_codec.hpp>
#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/whatwg/bulk.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/whatwg/transcode_string.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;
using whatwg::encoding;

namespace {

constexpr std::size_t encoding_count = static_cast<std::size_t>(encoding::euc_kr) + 1;
static_assert(encoding_count == 40);

template <typename R>
auto collect(R&& r) {
    std::vector<std::ranges::range_value_t<R>> out;
    for (auto v : r)
        out.push_back(v);
    return out;
}

// Every byte value, then the shapes that exercise the multi-byte and stateful
// decoders: a leading BOM, ISO-2022-JP escapes, a truncated sequence at the
// end so that flush has something to say.
std::string byte_sample() {
    std::string s{"\xEF\xBB\xBF"};
    for (int b = 0; b < 256; ++b)
        s.push_back(static_cast<char>(b));
    s += "\x1B$B\x30\x21\x1B(B plain \x82\xA0 \xA4\xA2 \x81\x30\x81\x30";
    s.push_back('\xD8');
    return s;
}

std::u32string scalar_sample() {
    std::u32string s{U"ASCII é€あ中가\U0001F600  end"};
    for (char32_t cp = 0x80; cp < 0x3000; cp += 7)
        if (cp < 0xD800 || cp > 0xDFFF)
            s.push_back(cp);
    return s;
}

template <encoding E>
void check_decoder_matches() {
    INFO("encoding " << static_cast<int>(E));
    const std::string             bytes = byte_sample();
    const std::span<const char>   in{bytes};
    const whatwg::dynamic_decoder d{E};
    CHECK(d.encoding() == E);
    CHECK(collect(in | decode(d)) == collect(in | whatwg::decode<E>));
    CHECK(collect(in | decode_or_error(d)) == collect(in | whatwg::decode_or_error<E>));
    CHECK(whatwg::decode_to(in, E) == whatwg::decode_to<E>(in));
}

template <encoding E>
void check_encoder_matches() {
    INFO("encoding " << static_cast<int>(E));
    const std::u32string            scalars = scalar_sample();
    const std::span<const char32_t> in{scalars};
    if constexpr (requires { typename whatwg::encoder<E>; }) {
        const auto made = whatwg::make_dynamic_encoder(E);
        REQUIRE(made.has_value());
        const whatwg::dynamic_encoder e = *made;
        CHECK(e.encoding() == E);
        CHECK(collect(in | encode(e)) == collect(in | whatwg::encode<E>));
        CHECK(collect(in | encode_or_error(e)) == collect(in | whatwg::encode_or_error<E>));
        CHECK(whatwg::encode_to(in, E) == whatwg::encode_to<E>(in));
    } else {
        CHECK_FALSE(whatwg::has_encoder(E));
        CHECK(whatwg::make_dynamic_encoder(E) == std::nullopt);
        CHECK(whatwg::encode_to(in, E) == std::nullopt);
    }
}

template <std::size_t... I>
void check_every_encoding(std::index_sequence<I...>) {
    (check_decoder_matches<static_cast<encoding>(I)>(), ...);
    (check_encoder_matches<static_cast<encoding>(I)>(), ...);
}

} // namespace

// The codec concepts: the stateful and BOM-aware refinements always, because
// a runtime choice may need them; never the single-byte ones, because a
// runtime choice cannot promise them.
static_assert(text_decoder<whatwg::dynamic_decoder>);
static_assert(flushable_text_decoder<whatwg::dynamic_decoder>);
static_assert(bom_aware_text_decoder<whatwg::dynamic_decoder>);
static_assert(!single_byte_text_decoder<whatwg::dynamic_decoder>);
static_assert(text_encoder<whatwg::dynamic_encoder>);
static_assert(flushable_text_encoder<whatwg::dynamic_encoder>);
static_assert(!single_byte_text_encoder<whatwg::dynamic_encoder>);

TEST_CASE("whatwg::dynamic_decoder and dynamic_encoder agree with the static codecs for every encoding",
          "[whatwg::dynamic_codec]") {
    check_every_encoding(std::make_index_sequence<encoding_count>{});
}

TEST_CASE("whatwg::dynamic_decoder: default, encoding and equality", "[whatwg::dynamic_codec]") {
    CHECK(whatwg::dynamic_decoder{}.encoding() == encoding::utf_8);
    CHECK(whatwg::dynamic_decoder{encoding::shift_jis} == whatwg::dynamic_decoder{encoding::shift_jis});
    CHECK(whatwg::dynamic_decoder{encoding::shift_jis} != whatwg::dynamic_decoder{encoding::euc_jp});
    CHECK(whatwg::dynamic_encoder{}.encoding() == encoding::utf_8);
    CHECK(whatwg::make_dynamic_encoder(encoding::gbk) != whatwg::make_dynamic_encoder(encoding::gb18030));
    CHECK(whatwg::make_dynamic_encoder(encoding::gbk) == whatwg::make_dynamic_encoder(encoding::gbk));
}

TEST_CASE("whatwg::dynamic_decoder: a leading BOM is stripped only where the Standard says",
          "[whatwg::dynamic_codec]") {
    const std::string utf8{"\xEF\xBB\xBF"
                           "A"};
    CHECK(collect(std::span<const char>{utf8} | decode(whatwg::dynamic_decoder{encoding::utf_8})) ==
          std::vector<char32_t>{U'A'});
    const std::string le{"\xFF\xFE"
                         "A\x00",
                         4};
    CHECK(collect(std::span<const char>{le} | decode(whatwg::dynamic_decoder{encoding::utf_16le})) ==
          std::vector<char32_t>{U'A'});
    // windows-1252 has no BOM; the bytes are ordinary characters.
    CHECK(collect(std::span<const char>{utf8} | decode(whatwg::dynamic_decoder{encoding::windows_1252})) ==
          std::vector<char32_t>{U'ï', U'»', U'¿', U'A'});
}

TEST_CASE("whatwg::dynamic_decoder: flush reports a truncated sequence", "[whatwg::dynamic_codec]") {
    const std::string odd{"A\x00"
                          "B",
                          3};
    const auto r = collect(std::span<const char>{odd} | decode_or_error(whatwg::dynamic_decoder{encoding::utf_16le}));
    REQUIRE(r.size() == 2);
    CHECK(r[0] == U'A');
    CHECK(!r[1].has_value());
    whatwg::dynamic_decoder stateless{encoding::windows_1252};
    CHECK(stateless.flush() == std::nullopt);
}

TEST_CASE("whatwg::dynamic_encoder: ISO-2022-JP returns to ASCII at the end", "[whatwg::dynamic_codec]") {
    const std::u32string s{U"あ"};
    const auto           out =
        collect(std::span<const char32_t>{s} | encode(*whatwg::make_dynamic_encoder(encoding::iso_2022_jp)));
    CHECK(std::string(out.begin(), out.end()) == "\x1B$B\x24\x22\x1B(B");
}

TEST_CASE("whatwg::decode_to and encode_to by label", "[whatwg::dynamic_codec][whatwg::bulk]") {
    const std::string sjis{"\x82\xA0"};
    CHECK(whatwg::decode_to(sjis, "Shift_JIS") == std::vector<char32_t>{U'あ'});
    CHECK(whatwg::decode_to(sjis, "x-made-up") == std::nullopt);
    const std::u32string s{U"€"};
    CHECK(whatwg::encode_to(s, "latin1") == std::string{"\x80"});
    CHECK(whatwg::encode_to(s, "utf-16le") == std::nullopt);
    CHECK(whatwg::encode_to(s, "x-made-up") == std::nullopt);
}

TEST_CASE("whatwg::decode_into and encode_into with a runtime encoding", "[whatwg::dynamic_codec][whatwg::bulk]") {
    const std::string     koi8{"\xF0\xD2\xC9"};
    std::vector<char32_t> scalars;
    whatwg::decode_into(koi8, encoding::koi8_r, std::back_inserter(scalars));
    CHECK(scalars == std::vector<char32_t>{U'П', U'р', U'и'});
    std::string back;
    CHECK(whatwg::encode_into(scalars, encoding::koi8_r, std::back_inserter(back)));
    CHECK(back == koi8);
    std::string none;
    CHECK_FALSE(whatwg::encode_into(scalars, encoding::replacement, std::back_inserter(none)));
    CHECK(none.empty());
}

TEST_CASE("whatwg::dynamic_decoder and dynamic_encoder in a constant expression",
          "[whatwg::dynamic_codec][constexpr]") {
    CHECK(constify([] {
              const char            bytes[] = {'\x80'};
              std::span<const char> in{bytes};
              return *(in | decode(whatwg::dynamic_decoder{encoding::windows_1252})).begin();
          }()) == U'€');
    CHECK(constify([] {
              auto e = *whatwg::make_dynamic_encoder(encoding::windows_1252);
              return e.encode_one(U'€').bytes[0];
          }()) == 0x80);
    CHECK(constify(whatwg::dynamic_decoder{encoding::big5}.encoding()) == encoding::big5);
}

TEST_CASE("whatwg::make_dynamic_encoder: no encoder, no object", "[whatwg::dynamic_codec]") {
    // The check a program holding an encoding from its input needs is the call
    // that creates the encoder.
    CHECK(whatwg::make_dynamic_encoder(encoding::utf_16be) == std::nullopt);
    CHECK(whatwg::make_dynamic_encoder(encoding::utf_16le) == std::nullopt);
    CHECK(whatwg::make_dynamic_encoder(encoding::replacement) == std::nullopt);
    CHECK(whatwg::make_dynamic_encoder(encoding::x_user_defined).has_value());
    CHECK(constify(whatwg::make_dynamic_encoder(encoding::replacement).has_value()) == false);
    CHECK(constify(whatwg::make_dynamic_encoder(encoding::shift_jis)->encoding()) == encoding::shift_jis);
}
