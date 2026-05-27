// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_GBK_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_GBK_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/gbk.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
#include <iterator>

#endif
namespace beman::transcoding::detail {

struct gbk_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct gbk_encode_result {
    unsigned char bytes[2]{};
    int           count{0};
    bool          is_error{false};
};

// Decode one GBK character from the range [current, end).
// ASCII bytes (0x00–0x7F) are direct passthroughs (1 byte consumed).
// Lead bytes 0x81–0xFE are followed by a trail byte 0x40–0xFE (not 0x7F).
// Advances current_ by the number of bytes consumed.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr gbk_decode_result gbk_decode_one(I& current, S end);

// Encode one Unicode codepoint as GBK.
// ASCII (cp < 0x80) outputs 1 byte. Non-ASCII searches the decode table.
// Returns is_error=true if the codepoint is not representable in GBK.
constexpr gbk_encode_result gbk_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr gbk_decode_result gbk_decode_one(I& current, S end) {
    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto lead = static_cast<unsigned char>(*current);
    ++current;

    if (lead < 0x80)
        return {static_cast<char32_t>(lead), {}, false};

    if (lead == 0x80)
        return {char32_t(0x20AC), {}, false};

    if (lead < 0x81 || lead > 0xFE)
        return {{}, whatwg_error::invalid_byte, true};

    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto trail = static_cast<unsigned char>(*current);

    if (trail < 0x40 || trail > 0xFE || trail == 0x7F) {
        if (trail >= 0x80)
            ++current;
        return {{}, whatwg_error::invalid_byte, true};
    }

    ++current;

    int offset = trail > 0x7F ? 1 : 0;
    int index  = (lead - 0x81) * 190 + (trail - 0x40) - offset;

    char32_t cp = tables::gbk[index];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};

    return {cp, {}, false};
}

constexpr gbk_encode_result gbk_encode_one(char32_t cp) {
    if (cp < 0x80) {
        gbk_encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    for (int i = 0; i < 23940; ++i) {
        if (tables::gbk[i] == cp) {
            int lead  = i / 190 + 0x81;
            int trail = i % 190 + 0x40;
            if (trail >= 0x7F)
                ++trail;
            gbk_encode_result r{};
            r.bytes[0] = static_cast<unsigned char>(lead);
            r.bytes[1] = static_cast<unsigned char>(trail);
            r.count    = 2;
            return r;
        }
    }

    return {{}, 0, true};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_GBK_HPP
