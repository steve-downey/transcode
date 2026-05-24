// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_GB18030_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_GB18030_HPP

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/gb18030_ranges.hpp>
#include <beman/transcode/detail/tables/gbk.hpp>

#include <cstdint>
#include <iterator>

namespace beman::transcoding::detail {

struct gb18030_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct gb18030_encode_result {
    unsigned char bytes[4]{};
    int           count{0};
    bool          is_error{false};
};

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------

// Decode one GB18030 character from [current, end).
// Handles ASCII passthrough, 0x80→U+20AC special case, 2-byte GBK, and 4-byte ranges.
// Advances current by bytes consumed.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr gb18030_decode_result gb18030_decode_one(I& current, S end);

// Encode one Unicode codepoint as GB18030.
// GB18030 covers all Unicode, so is_error is never true for valid scalar values.
constexpr gb18030_encode_result gb18030_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

// Binary search the ranges table to convert a linear 4-byte pointer to a codepoint.
constexpr char32_t gb18030_ranges_decode(std::uint32_t pointer) {
    const auto* table = tables::gb18030_ranges;
    int         n     = tables::gb18030_ranges_count;
    int         lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (mid + 1 < n && table[mid + 1].pointer <= pointer) {
            lo = mid + 1;
        } else if (table[mid].pointer > pointer) {
            hi = mid - 1;
        } else {
            return static_cast<char32_t>(table[mid].codepoint + (pointer - table[mid].pointer));
        }
    }
    return U'\xFFFD';
}

// Binary search the ranges table to convert a codepoint to a linear 4-byte pointer.
constexpr std::uint32_t gb18030_ranges_encode(char32_t cp) {
    const auto* table = tables::gb18030_ranges;
    int         n     = tables::gb18030_ranges_count;
    int         lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (mid + 1 < n && table[mid + 1].codepoint <= cp) {
            lo = mid + 1;
        } else if (table[mid].codepoint > cp) {
            hi = mid - 1;
        } else {
            return table[mid].pointer + (cp - table[mid].codepoint);
        }
    }
    return 0;
}

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr gb18030_decode_result gb18030_decode_one(I& current, S end) {
    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto lead = static_cast<unsigned char>(*current);
    ++current;

    // ASCII passthrough
    if (lead < 0x80)
        return {static_cast<char32_t>(lead), {}, false};

    // Special case: 0x80 -> U+20AC
    if (lead == 0x80)
        return {U'\x20AC', {}, false};

    // Lead must be 0x81–0xFE
    if (lead < 0x81 || lead > 0xFE)
        return {{}, whatwg_error::invalid_byte, true};

    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto second = static_cast<unsigned char>(*current);
    ++current;

    // 4-byte GB18030 sequence: second byte is a decimal digit 0x30–0x39
    if (second >= 0x30 && second <= 0x39) {
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto third = static_cast<unsigned char>(*current);
        ++current;
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto fourth = static_cast<unsigned char>(*current);
        ++current;

        if (third < 0x81 || third > 0xFE)
            return {{}, whatwg_error::invalid_byte, true};
        if (fourth < 0x30 || fourth > 0x39)
            return {{}, whatwg_error::invalid_byte, true};

        std::uint32_t pointer = ((lead - 0x81u) * 10u + (second - 0x30u)) * 126u * 10u +
                                (third - 0x81u) * 10u + (fourth - 0x30u);
        char32_t cp = gb18030_ranges_decode(pointer);
        return {cp, {}, false};
    }

    // 2-byte GBK sequence: trail byte 0x40–0xFE, not 0x7F
    if (second < 0x40 || second > 0xFE || second == 0x7F)
        return {{}, whatwg_error::invalid_byte, true};

    int offset = second > 0x7F ? 1 : 0;
    int index  = (lead - 0x81) * 190 + (second - 0x40) - offset;

    char32_t cp = tables::gbk[index];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};

    return {cp, {}, false};
}

constexpr gb18030_encode_result gb18030_encode_one(char32_t cp) {
    gb18030_encode_result r{};

    // ASCII passthrough
    if (cp < 0x80) {
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    // Search GBK 2-byte table
    for (int i = 0; i < 23940; ++i) {
        if (tables::gbk[i] == cp) {
            int lead  = i / 190 + 0x81;
            int trail = i % 190 + 0x40;
            if (trail >= 0x7F)
                ++trail;
            r.bytes[0] = static_cast<unsigned char>(lead);
            r.bytes[1] = static_cast<unsigned char>(trail);
            r.count    = 2;
            return r;
        }
    }

    // Encode via GB18030 4-byte ranges
    std::uint32_t pointer = gb18030_ranges_encode(cp);
    std::uint32_t p       = pointer;
    unsigned char b4      = static_cast<unsigned char>(p % 10 + 0x30);
    p /= 10;
    unsigned char b3 = static_cast<unsigned char>(p % 126 + 0x81);
    p /= 126;
    unsigned char b2 = static_cast<unsigned char>(p % 10 + 0x30);
    p /= 10;
    unsigned char b1 = static_cast<unsigned char>(p + 0x81);
    r.bytes[0]       = b1;
    r.bytes[1]       = b2;
    r.bytes[2]       = b3;
    r.bytes[3]       = b4;
    r.count          = 4;
    return r;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_GB18030_HPP
