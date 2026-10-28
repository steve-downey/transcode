// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// A codec the library does not ship, composed with the ones it does.
//
// The WHATWG Encoding Standard has no ISO-8859-1: its "latin1" and
// "iso-8859-1" labels name windows-1252, which differs in 0x80-0x9F.  A program
// that needs the ISO encoding writes it against the codec concepts, and the
// pluggable `decode` and `encode` adaptors drive it.  `char32_t` is the seam
// between codecs, so the custom decoder pipes straight into a WHATWG encoder,
// and a WHATWG decoder into the custom encoder.

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>

#include <iterator>
#include <string>
#include <string_view>

namespace {

using namespace beman::transcoding;

// ISO-8859-1: each byte is the scalar value of the same number.
struct iso_8859_1 {
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end) const;

    constexpr encode_result encode_one(char32_t cp) const;
};

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr decode_result iso_8859_1::decode_one(I& current, [[maybe_unused]] S end) const {
    const auto byte = static_cast<unsigned char>(*current);
    ++current;
    return {char32_t{byte}, {}, false};
}

constexpr encode_result iso_8859_1::encode_one(char32_t cp) const {
    if (cp > 0xFF)
        return {{}, 0, true};
    encode_result result{};
    result.bytes[0] = static_cast<unsigned char>(cp);
    result.count    = 1;
    return result;
}

static_assert(text_decoder<iso_8859_1> && text_encoder<iso_8859_1>);

// ISO-8859-1 to UTF-8: a custom decoder, then the library's UTF-8 encoder.
constexpr std::string to_utf8(std::string_view latin1) {
    std::string out;
    for (char b : latin1 | decode(iso_8859_1{}) | whatwg::encode<whatwg::encoding::utf_8>)
        out.push_back(b);
    return out;
}

// UTF-8 to ISO-8859-1: the library's decoder, then the custom encoder.  A
// scalar value the encoding cannot represent becomes '?'.
constexpr std::string from_utf8(std::string_view utf8) {
    std::string out;
    for (char b : utf8 | whatwg::decode<whatwg::encoding::utf_8> | encode(iso_8859_1{}))
        out.push_back(b);
    return out;
}

// 0x80 is a C1 control in ISO-8859-1 and the euro sign in windows-1252.
static_assert(to_utf8("\x80") == "\xC2\x80");
static_assert(to_utf8("caf\xE9") == "caf\xC3\xA9");
static_assert(from_utf8("caf\xC3\xA9") == "caf\xE9");
static_assert(from_utf8("\xE2\x82\xAC") == "?"); // U+20AC has no ISO-8859-1 byte

} // namespace

int main() { return to_utf8("caf\xE9") == "caf\xC3\xA9" && from_utf8("\xE2\x82\xAC") == "?" ? 0 : 1; }
