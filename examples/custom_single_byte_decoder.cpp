// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// A custom single-byte codec, implemented against the pluggable codec
// protocol.
//
// This is the extension point the library offers for an encoding it does not
// ship: satisfy `decode_codec` (and, if you want to encode, `encode_codec`),
// and the same views, error handling and bulk helpers that serve the built-in
// WHATWG codecs serve yours.  Nothing here is special-cased for this file.
//
// The encoding is invented, and deliberately not a pure table: bytes 0x80-0xBE
// decode to a scalar chosen by arithmetic rather than by lookup.  A codec whose
// upper half is a plain 128-entry table does not need to be written out like
// this -- `table_codec` in <beman/transcode/detail/table_codec.hpp> already
// does that -- but `table_codec` is an implementation detail of the built-in
// codecs and is not part of the proposal, so an example of the *protocol*
// should not lean on it.

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/error.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <span>
#include <string>

namespace {

using beman::transcoding::decode_result;
using beman::transcoding::whatwg_error;

// The upper half of the encoding, for the bytes that are a straight mapping.
// Zero means "this byte decodes to nothing", which is how the codec reports an
// invalid byte below.
inline constexpr std::array<char32_t, 64> upper_table = {
    /* 0xC0 */ U'Ç', U'ü', U'é', U'â', U'ä', U'à', U'å', U'ç',
    /* 0xC8 */ U'ê', U'ë', U'è', U'ï', U'î', U'ì', U'Ä', U'Å',
    /* 0xD0 */ U'É', U'È', U'Ì', U'ô', U'ö', U'ò', U'û', U'ù',
    /* 0xD8 */ U'ÿ', U'Ö', U'Ü', U'á', U'í', U'ó', U'ú', U'ñ',
    /* 0xE0 */ U'£', U'¥', U'€', U'Ò', U'Ù', U'±', U'Õ', U'Á',
    /* 0xE8 */ U'Í', U'©', U'®', U'Ô', U'Ó', U'Ú', U'Â', U'Ê',
    /* 0xF0 */ U'õ', U'À', U'Ñ', U'¿', U'¡', U'«', U'»', U'ã',
    /* 0xF8 */ U'Ã', U'ß', U'×', 0,    0,    0,    0,    0,
};

// Models `decode_codec`.  One byte in, one scalar value out, and an error for a
// byte the encoding does not define -- which the view turns into U+FFFD, or
// into an `unexpected`, depending on which adaptor the caller reached for.
struct fractional_codec {
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end) const;
};

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr decode_result fractional_codec::decode_one(I& current, [[maybe_unused]] S end) const {
    const auto byte = static_cast<unsigned char>(*current);
    ++current;

    // ASCII, unchanged.
    if (byte < 0x80)
        return {static_cast<char32_t>(byte), {}, false};

    // 0x80-0xBE are the vulgar fractions n/64, which Unicode does not have as
    // scalar values except for a handful.  This encoding keeps the three it can
    // represent exactly and rejects the rest, which is the interesting case:
    // a byte that is valid in the encoding and has no Unicode scalar value is
    // still a decode error, because decoding produces scalar values.
    if (byte <= 0xBE) {
        switch (byte - 0x80U + 1U) {
        case 16:
            return {U'¼', {}, false}; // 16/64 = 1/4
        case 32:
            return {U'½', {}, false}; // 32/64 = 1/2
        case 48:
            return {U'¾', {}, false}; // 48/64 = 3/4
        default:
            return {{}, whatwg_error::invalid_byte, true};
        }
    }

    if (byte == 0xBF)
        return {{}, whatwg_error::invalid_byte, true};

    const char32_t cp = upper_table[byte - 0xC0U];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};
    return {cp, {}, false};
}

static_assert(beman::transcoding::decode_codec<fractional_codec>,
              "fractional_codec must model decode_codec to be usable with decode()");

// Replacement mode: an undefined byte becomes U+FFFD and decoding continues.
constexpr std::u32string decode_all(std::span<const std::byte> input) {
    std::u32string result;
    for (char32_t cp : input | beman::transcoding::decode(fractional_codec{}))
        result.push_back(cp);
    return result;
}

// Inspection mode: the same pipeline, with each failure surfaced instead.
constexpr std::size_t count_invalid(std::span<const std::byte> input) {
    std::size_t invalid = 0;
    for (auto r : input | beman::transcoding::decode_or_error(fractional_codec{})) {
        if (!r.has_value())
            ++invalid;
    }
    return invalid;
}

constexpr std::array<std::byte, 8> sample = {
    std::byte{0xC0}, // U+00C7
    std::byte{0x41}, // 'A'
    std::byte{0x9F}, // 32/64 -> U+00BD
    std::byte{0x80}, // 1/64  -> no scalar value, invalid
    std::byte{0xBF}, // undefined
    std::byte{0xE2}, // U+20AC
    std::byte{0xFA}, // U+00D7
    std::byte{0xFF}, // unassigned tail of the table, invalid
};

// The codec is constexpr, so the views over it are too.  This is the same
// guarantee the built-in codecs give, and it comes from satisfying the concept
// rather than from anything the library does for its own codecs.
static_assert(decode_all(sample) == U"ÇA½��€×�");
static_assert(count_invalid(sample) == 3);

} // namespace

int main() {
    const std::u32string decoded = decode_all(sample);
    return decoded == U"ÇA½��€×�" && count_invalid(sample) == 3 ? 0 : 1;
}
