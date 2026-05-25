// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_BIG5_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_BIG5_HPP

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/big5.hpp>

#include <iterator>

namespace beman::transcoding::detail {

struct big5_decode_result {
    char32_t     code_point{0xFFFD};
    char32_t     code_point2{0};
    whatwg_error error{};
    bool         is_error{false};
};

struct big5_encode_result {
    unsigned char bytes[2]{};
    int           count{0};
    bool          is_error{false};
};

// Decode one Big5 character from [current, end).
// ASCII bytes (0x00–0x7F) pass through directly (1 byte consumed).
// Lead bytes 0x81–0xFE are followed by a trail byte in 0x40–0x7E or 0xA1–0xFE.
// Four pointer values (1133, 1135, 1164, 1165) yield two codepoints (code_point2 != 0).
// Advances current by the bytes consumed.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr big5_decode_result big5_decode_one(I& current, S end);

// Encode one Unicode codepoint as Big5.
// ASCII (cp < 0x80) outputs 1 byte. Non-ASCII searches the decode table for
// the lowest pointer that maps to cp, then converts pointer to lead/trail bytes.
// Returns is_error=true if the codepoint is not representable in Big5.
constexpr big5_encode_result big5_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr big5_decode_result big5_decode_one(I& current, S end) {
    if (current == end)
        return {{}, {}, whatwg_error::truncated_sequence, true};

    auto lead = static_cast<unsigned char>(*current);
    ++current;

    if (lead < 0x80)
        return {static_cast<char32_t>(lead), {}, {}, false};

    if (lead < 0x81 || lead > 0xFE)
        return {{}, {}, whatwg_error::invalid_byte, true};

    if (current == end)
        return {{}, {}, whatwg_error::truncated_sequence, true};

    auto trail = static_cast<unsigned char>(*current);
    ++current;

    int offset = -1;
    if (trail >= 0x40 && trail <= 0x7E)
        offset = trail - 0x40;
    else if (trail >= 0xA1 && trail <= 0xFE)
        offset = trail - 0x62;

    if (offset < 0) {
        // WHATWG: if trail is ASCII, prepend it back (emit as next codepoint)
        char32_t repush = (trail < 0x80) ? static_cast<char32_t>(trail) : 0;
        return {{}, repush, whatwg_error::invalid_byte, true};
    }

    int pointer = (lead - 0x81) * 157 + offset;

    // WHATWG special cases: four pointers yield two codepoints each
    if (pointer == 1133)
        return {U'\x00CA', U'\x0304', {}, false};
    if (pointer == 1135)
        return {U'\x00CA', U'\x030C', {}, false};
    if (pointer == 1164)
        return {U'\x00EA', U'\x0304', {}, false};
    if (pointer == 1165)
        return {U'\x00EA', U'\x030C', {}, false};

    char32_t cp = tables::big5[pointer];
    if (cp == 0) {
        // WHATWG: if trail is ASCII, prepend it back
        char32_t repush = (trail < 0x80) ? static_cast<char32_t>(trail) : 0;
        return {{}, repush, whatwg_error::invalid_byte, true};
    }

    return {cp, {}, {}, false};
}

constexpr big5_encode_result big5_encode_one(char32_t cp) {
    if (cp < 0x80) {
        big5_encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    for (int i = 0; i < 19782; ++i) {
        if (tables::big5[i] == cp) {
            int                lead   = i / 157 + 0x81;
            int                offset = i % 157;
            int                trail  = (offset < 63) ? offset + 0x40 : offset + 0x62;
            big5_encode_result r{};
            r.bytes[0] = static_cast<unsigned char>(lead);
            r.bytes[1] = static_cast<unsigned char>(trail);
            r.count    = 2;
            return r;
        }
    }

    return {{}, 0, true};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_BIG5_HPP
