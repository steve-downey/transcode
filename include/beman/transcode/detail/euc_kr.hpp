// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_KR_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_KR_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/euc_kr.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>

#endif
namespace beman::transcoding::detail {

struct euc_kr_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct euc_kr_encode_result {
    unsigned char bytes[2]{};
    int           count{0};
    bool          is_error{false};
};

// Decode one EUC-KR character from the range [current, end).
// ASCII bytes (0x00-0x7F) pass through directly (1 byte consumed).
// Lead 0x81-0xFE followed by trail 0x41-0xFE (excluding 0x7F) yields 2 bytes.
// Advances current by the number of bytes consumed.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr euc_kr_decode_result euc_kr_decode_one(I& current, S end);

// Encode one Unicode codepoint as EUC-KR.
// ASCII (cp < 0x80) outputs 1 byte. Non-ASCII searches the decode table.
// Returns is_error=true if the codepoint is not representable in EUC-KR.
constexpr euc_kr_encode_result euc_kr_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr euc_kr_decode_result euc_kr_decode_one(I& current, S end) {
    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto lead = static_cast<unsigned char>(*current);
    ++current;

    if (lead < 0x80)
        return {static_cast<char32_t>(lead), {}, false};

    if (lead < 0x81 || lead > 0xFE)
        return {{}, whatwg_error::invalid_byte, true};

    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto trail = static_cast<unsigned char>(*current);
    ++current;

    if (trail < 0x41 || trail > 0xFE || trail == 0x7F)
        return {{}, whatwg_error::invalid_byte, true};

    int offset  = trail > 0x7F ? 1 : 0;
    int pointer = (lead - 0x81) * 190 + (trail - 0x41) - offset;

    char32_t cp = tables::euc_kr[pointer];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};
    return {cp, {}, false};
}

constexpr euc_kr_encode_result euc_kr_encode_one(char32_t cp) {
    if (cp < 0x80) {
        euc_kr_encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    for (int i = 0; i < 23940; ++i) {
        if (tables::euc_kr[i] == cp) {
            int trail = i % 190 + 0x41;
            if (trail >= 0x7F)
                ++trail;
            euc_kr_encode_result r{};
            r.bytes[0] = static_cast<unsigned char>(i / 190 + 0x81);
            r.bytes[1] = static_cast<unsigned char>(trail);
            r.count    = 2;
            return r;
        }
    }

    return {{}, 0, true};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_KR_HPP
