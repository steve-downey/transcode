// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLE_CODEC_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLE_CODEC_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/codec_concepts.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <array>
    #include <iterator>

#endif
namespace beman::transcoding {

template <std::array<char32_t, 128> Table>
struct table_codec {
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end) const;

    constexpr char32_t decode_byte(unsigned char byte) const;

    constexpr encode_result encode_one(char32_t cp) const;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::array<char32_t, 128> Table>
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr decode_result table_codec<Table>::decode_one(I& current, [[maybe_unused]] S end) const {
    auto byte = static_cast<unsigned char>(*current);
    ++current;
    if (byte < 0x80)
        return {static_cast<char32_t>(byte), {}, false};
    char32_t cp = Table[byte - 0x80];
    if (cp == 0)
        return {{}, decode_error::invalid_byte, true};
    return {cp, {}, false};
}

template <std::array<char32_t, 128> Table>
constexpr char32_t table_codec<Table>::decode_byte(unsigned char byte) const {
    if (byte < 0x80)
        return static_cast<char32_t>(byte);
    char32_t cp = Table[byte - 0x80];
    return cp == 0 ? U'\xFFFD' : cp;
}

template <std::array<char32_t, 128> Table>
constexpr encode_result table_codec<Table>::encode_one(char32_t cp) const {
    if (cp < 0x80)
        return {{static_cast<unsigned char>(cp)}, 1, false};
    for (int i = 0; i < 128; ++i) {
        if (Table[static_cast<std::size_t>(i)] == cp)
            return {{static_cast<unsigned char>(0x80 + i)}, 1, false};
    }
    return {{}, 0, true};
}

// ---------------------------------------------------------------------------
// full_table_codec — for encodings that remap the entire 256-byte range
// (not ASCII-compatible in the lower half)
// ---------------------------------------------------------------------------

template <std::array<char32_t, 256> Table>
struct full_table_codec {
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end) const;

    constexpr char32_t decode_byte(unsigned char byte) const;

    constexpr encode_result encode_one(char32_t cp) const;
};

template <std::array<char32_t, 256> Table>
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr decode_result full_table_codec<Table>::decode_one(I& current, [[maybe_unused]] S end) const {
    auto byte = static_cast<unsigned char>(*current);
    ++current;
    char32_t cp = Table[byte];
    if (cp == 0)
        return {{}, decode_error::invalid_byte, true};
    return {cp, {}, false};
}

template <std::array<char32_t, 256> Table>
constexpr char32_t full_table_codec<Table>::decode_byte(unsigned char byte) const {
    char32_t cp = Table[byte];
    return cp == 0 ? U'\xFFFD' : cp;
}

template <std::array<char32_t, 256> Table>
constexpr encode_result full_table_codec<Table>::encode_one(char32_t cp) const {
    for (int i = 0; i < 256; ++i) {
        if (Table[static_cast<std::size_t>(i)] == cp)
            return {{static_cast<unsigned char>(i)}, 1, false};
    }
    return {{}, 0, true};
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLE_CODEC_HPP
