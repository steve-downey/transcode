// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SHIFT_JIS_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SHIFT_JIS_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/shift_jis.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
#include <iterator>

#endif
namespace beman::transcoding::detail {

struct shift_jis_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct shift_jis_encode_result {
    unsigned char bytes[2]{};
    int           count{0};
    bool          is_error{false};
};

// Decode one Shift_JIS character from the range [current, end).
// ASCII bytes (0x00-0x7F) pass through directly (1 byte consumed).
// Half-width katakana (0xA1-0xDF) map to U+FF61-U+FF9F (1 byte consumed).
// Lead bytes 0x81-0x9F and 0xE0-0xFC are followed by a trail byte.
// Valid trail ranges: 0x40-0x7E and 0x80-0xFC (0x7F is excluded).
// Advances current by the number of bytes consumed.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr shift_jis_decode_result shift_jis_decode_one(I& current, S end);

// Encode one Unicode codepoint as Shift_JIS.
// ASCII (cp < 0x80) outputs 1 byte. U+00A5 maps to 0x5C; U+203E to 0x7E.
// U+FF61-U+FF9F (half-width katakana) map to 0xA1-0xDF.
// Other codepoints are searched in the decode table.
// Returns is_error=true if the codepoint is not representable in Shift_JIS.
constexpr shift_jis_encode_result shift_jis_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr shift_jis_decode_result shift_jis_decode_one(I& current, S end) {
    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto lead = static_cast<unsigned char>(*current);
    ++current;

    // ASCII passthrough
    if (lead < 0x80)
        return {static_cast<char32_t>(lead), {}, false};

    // Half-width katakana (U+FF61-U+FF9F)
    if (lead >= 0xA1 && lead <= 0xDF)
        return {static_cast<char32_t>(0xFF61 + (lead - 0xA1)), {}, false};

    // Multi-byte lead bytes: 0x81-0x9F and 0xE0-0xFC
    bool is_lead = (lead >= 0x81 && lead <= 0x9F) || (lead >= 0xE0 && lead <= 0xFC);
    if (!is_lead)
        return {{}, whatwg_error::invalid_byte, true};

    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto trail = static_cast<unsigned char>(*current);
    ++current;

    // Valid trail ranges: 0x40-0x7E and 0x80-0xFC (0x7F excluded)
    bool valid_trail = (trail >= 0x40 && trail <= 0x7E) || (trail >= 0x80 && trail <= 0xFC);
    if (!valid_trail)
        return {{}, whatwg_error::invalid_byte, true};

    int lead_offset  = (lead <= 0x9F) ? lead - 0x81 : lead - 0xC1;
    int trail_offset = (trail < 0x7F) ? trail - 0x40 : trail - 0x41;
    int pointer      = lead_offset * 188 + trail_offset;

    char32_t cp = tables::shift_jis[pointer];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};

    return {cp, {}, false};
}

constexpr shift_jis_encode_result shift_jis_encode_one(char32_t cp) {
    // ASCII passthrough
    if (cp < 0x80) {
        shift_jis_encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    // WHATWG special cases
    if (cp == 0x00A5) {
        shift_jis_encode_result r{};
        r.bytes[0] = 0x5C;
        r.count    = 1;
        return r;
    }
    if (cp == 0x203E) {
        shift_jis_encode_result r{};
        r.bytes[0] = 0x7E;
        r.count    = 1;
        return r;
    }

    // Half-width katakana (U+FF61-U+FF9F)
    if (cp >= 0xFF61 && cp <= 0xFF9F) {
        shift_jis_encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(0xA1 + (cp - 0xFF61));
        r.count    = 1;
        return r;
    }

    // Search the decode table for the lowest pointer mapping to cp
    for (int i = 0; i < 11280; ++i) {
        if (tables::shift_jis[i] == cp) {
            int                     lead_offset  = i / 188;
            int                     trail_offset = i % 188;
            int                     lead         = (lead_offset < 0x1F) ? lead_offset + 0x81 : lead_offset + 0xC1;
            int                     trail        = (trail_offset < 63) ? trail_offset + 0x40 : trail_offset + 0x41;
            shift_jis_encode_result r{};
            r.bytes[0] = static_cast<unsigned char>(lead);
            r.bytes[1] = static_cast<unsigned char>(trail);
            r.count    = 2;
            return r;
        }
    }

    return {{}, 0, true};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SHIFT_JIS_HPP
