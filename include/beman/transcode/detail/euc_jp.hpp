// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_JP_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_JP_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/euc_jp_jis0212.hpp>
#include <beman/transcode/detail/tables/shift_jis.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>

#endif
namespace beman::transcoding::detail {

struct euc_jp_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct euc_jp_encode_result {
    unsigned char bytes[3]{};
    int           count{0};
    bool          is_error{false};
};

// Decode one EUC-JP character from the range [current, end).
// ASCII bytes (0x00-0x7F) pass through directly (1 byte consumed).
// 0x8E (SS2) is followed by a half-width katakana byte (0xA1-0xDF), 2 bytes total.
// 0x8F (SS3) is followed by two JIS X 0212 bytes (0xA1-0xFE each), 3 bytes total.
// 0xA1-0xFE (JIS X 0208 lead) is followed by a trail byte (0xA1-0xFE), 2 bytes total.
// Advances current by the number of bytes consumed.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr euc_jp_decode_result euc_jp_decode_one(I& current, S end);

// Encode one Unicode codepoint as EUC-JP.
// ASCII (cp < 0x80) outputs 1 byte. U+00A5 maps to 0x5C; U+203E to 0x7E.
// U+FF61-U+FF9F (half-width katakana) encode as 0x8E + 1 byte.
// JIS X 0208 codepoints encode as 2 bytes (0xA1-0xFE range).
// JIS X 0212 codepoints encode as 3 bytes (0x8F + 0xA1-0xFE + 0xA1-0xFE).
// Returns is_error=true if the codepoint is not representable in EUC-JP.
constexpr euc_jp_encode_result euc_jp_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr euc_jp_decode_result euc_jp_decode_one(I& current, S end) {
    if (current == end)
        return {{}, whatwg_error::truncated_sequence, true};

    auto lead = static_cast<unsigned char>(*current);
    ++current;

    // ASCII passthrough
    if (lead < 0x80)
        return {static_cast<char32_t>(lead), {}, false};

    // SS2 prefix: half-width katakana
    if (lead == 0x8E) {
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto byte2 = static_cast<unsigned char>(*current);
        ++current;
        if (byte2 < 0xA1 || byte2 > 0xDF)
            return {{}, whatwg_error::invalid_byte, true};
        return {static_cast<char32_t>(0xFF61 + (byte2 - 0xA1)), {}, false};
    }

    // SS3 prefix: JIS X 0212 (3-byte sequence)
    if (lead == 0x8F) {
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto b1 = static_cast<unsigned char>(*current);
        ++current;
        if (b1 < 0xA1 || b1 > 0xFE)
            return {{}, whatwg_error::invalid_byte, true};
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto b2 = static_cast<unsigned char>(*current);
        ++current;
        if (b2 < 0xA1 || b2 > 0xFE)
            return {{}, whatwg_error::invalid_byte, true};
        int      pointer = (b1 - 0xA1) * 94 + (b2 - 0xA1);
        char32_t cp      = tables::euc_jp_jis0212[pointer];
        if (cp == 0)
            return {{}, whatwg_error::invalid_byte, true};
        return {cp, {}, false};
    }

    // JIS X 0208: lead 0xA1-0xFE followed by trail 0xA1-0xFE
    if (lead >= 0xA1 && lead <= 0xFE) {
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto trail = static_cast<unsigned char>(*current);
        ++current;
        if (trail < 0xA1 || trail > 0xFE)
            return {{}, whatwg_error::invalid_byte, true};
        int      pointer = (lead - 0xA1) * 94 + (trail - 0xA1);
        char32_t cp      = tables::shift_jis[pointer];
        if (cp == 0)
            return {{}, whatwg_error::invalid_byte, true};
        return {cp, {}, false};
    }

    // Any other byte is invalid
    return {{}, whatwg_error::invalid_byte, true};
}

constexpr euc_jp_encode_result euc_jp_encode_one(char32_t cp) {
    // ASCII passthrough
    if (cp < 0x80) {
        euc_jp_encode_result r{};
        r.bytes[0] = static_cast<unsigned char>(cp);
        r.count    = 1;
        return r;
    }

    // WHATWG special cases
    if (cp == 0x00A5) {
        euc_jp_encode_result r{};
        r.bytes[0] = 0x5C;
        r.count    = 1;
        return r;
    }
    if (cp == 0x203E) {
        euc_jp_encode_result r{};
        r.bytes[0] = 0x7E;
        r.count    = 1;
        return r;
    }

    // Half-width katakana (U+FF61-U+FF9F) -> SS2 prefix + byte
    if (cp >= 0xFF61 && cp <= 0xFF9F) {
        euc_jp_encode_result r{};
        r.bytes[0] = 0x8E;
        r.bytes[1] = static_cast<unsigned char>(0xA1 + (cp - 0xFF61));
        r.count    = 2;
        return r;
    }

    // Search JIS X 0208 table (first 8836 entries of shift_jis = EUC-JP jis0208 range)
    for (int i = 0; i < 8836; ++i) {
        if (tables::shift_jis[i] == cp) {
            euc_jp_encode_result r{};
            r.bytes[0] = static_cast<unsigned char>((i / 94) + 0xA1);
            r.bytes[1] = static_cast<unsigned char>((i % 94) + 0xA1);
            r.count    = 2;
            return r;
        }
    }

    // Search JIS X 0212 table
    for (int i = 0; i < 8836; ++i) {
        if (tables::euc_jp_jis0212[i] == cp) {
            euc_jp_encode_result r{};
            r.bytes[0] = 0x8F;
            r.bytes[1] = static_cast<unsigned char>((i / 94) + 0xA1);
            r.bytes[2] = static_cast<unsigned char>((i % 94) + 0xA1);
            r.count    = 3;
            return r;
        }
    }

    return {{}, 0, true};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_JP_HPP
