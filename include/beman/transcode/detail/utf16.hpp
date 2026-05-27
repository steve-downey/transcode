// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
#include <iterator>

#endif
namespace beman::transcoding::detail {

struct utf16_decode_result {
    char32_t     codepoint{0xFFFD};
    int          bytes_consumed{0};
    bool         is_error{false};
    whatwg_error error{};
};

struct utf16_encode_result {
    char bytes[4]{};
    int  count{0};
    bool is_error{false};
};

template <typename I, typename S>
constexpr utf16_decode_result utf16be_decode_one(I& current, S end);

template <typename I, typename S>
constexpr utf16_decode_result utf16le_decode_one(I& current, S end);

constexpr utf16_encode_result utf16be_encode_one(char32_t cp);
constexpr utf16_encode_result utf16le_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <typename I, typename S>
constexpr utf16_decode_result utf16be_decode_one(I& current, S end) {
    if (current == end)
        return {0xFFFD, 0, true, whatwg_error::truncated_sequence};

    auto b0 = static_cast<unsigned char>(*current);
    ++current;
    if (current == end)
        return {0xFFFD, 1, true, whatwg_error::truncated_sequence};

    auto b1 = static_cast<unsigned char>(*current);
    ++current;

    auto unit = static_cast<char16_t>((b0 << 8) | b1);

    if (unit >= 0xD800 && unit <= 0xDBFF) {
        // High surrogate — need 2 more bytes for low surrogate
        if (current == end)
            return {0xFFFD, 2, true, whatwg_error::truncated_sequence};

        auto b2 = static_cast<unsigned char>(*current);
        ++current;
        if (current == end)
            return {0xFFFD, 3, true, whatwg_error::truncated_sequence};

        auto b3 = static_cast<unsigned char>(*current);
        ++current;

        auto low = static_cast<char16_t>((b2 << 8) | b3);
        if (low >= 0xDC00 && low <= 0xDFFF) {
            char32_t cp = 0x10000 + ((static_cast<char32_t>(unit - 0xD800) << 10) | (low - 0xDC00));
            return {cp, 4, false, {}};
        }
        // Bad low surrogate: consume only the high surrogate pair bytes,
        // leave b2/b3 unconsumed (back up 2)
        // We already advanced; to not consume the low unit we must back up.
        // Input iterators can't go back — instead, we consume all 4 bytes
        // but emit an error. The WHATWG spec says: emit U+FFFD for the high
        // surrogate, then re-process the low unit. Since input iterators
        // can't retreat, we emit U+FFFD for the pair and let the next call
        // see the low unit — but we've already consumed it. Emit two errors
        // isn't easy from here. For simplicity (and WHATWG "simple" mode):
        // emit U+FFFD, having consumed all 4 bytes. The caller can only
        // produce one value per call, so we flag is_error and emit U+FFFD.
        // (A second U+FFFD for the orphaned low unit would require a push-back
        // that input iterators don't support.)
        return {0xFFFD, 4, true, whatwg_error::surrogate_code_point};
    }

    if (unit >= 0xDC00 && unit <= 0xDFFF)
        return {0xFFFD, 2, true, whatwg_error::surrogate_code_point};

    return {static_cast<char32_t>(unit), 2, false, {}};
}

template <typename I, typename S>
constexpr utf16_decode_result utf16le_decode_one(I& current, S end) {
    if (current == end)
        return {0xFFFD, 0, true, whatwg_error::truncated_sequence};

    auto b0 = static_cast<unsigned char>(*current);
    ++current;
    if (current == end)
        return {0xFFFD, 1, true, whatwg_error::truncated_sequence};

    auto b1 = static_cast<unsigned char>(*current);
    ++current;

    auto unit = static_cast<char16_t>((b1 << 8) | b0);

    if (unit >= 0xD800 && unit <= 0xDBFF) {
        if (current == end)
            return {0xFFFD, 2, true, whatwg_error::truncated_sequence};

        auto b2 = static_cast<unsigned char>(*current);
        ++current;
        if (current == end)
            return {0xFFFD, 3, true, whatwg_error::truncated_sequence};

        auto b3 = static_cast<unsigned char>(*current);
        ++current;

        auto low = static_cast<char16_t>((b3 << 8) | b2);
        if (low >= 0xDC00 && low <= 0xDFFF) {
            char32_t cp = 0x10000 + ((static_cast<char32_t>(unit - 0xD800) << 10) | (low - 0xDC00));
            return {cp, 4, false, {}};
        }
        return {0xFFFD, 4, true, whatwg_error::surrogate_code_point};
    }

    if (unit >= 0xDC00 && unit <= 0xDFFF)
        return {0xFFFD, 2, true, whatwg_error::surrogate_code_point};

    return {static_cast<char32_t>(unit), 2, false, {}};
}

constexpr utf16_encode_result utf16be_encode_one(char32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, 0, true};
    if (cp <= 0xFFFF) {
        utf16_encode_result r{};
        r.bytes[0] = static_cast<char>((cp >> 8) & 0xFF);
        r.bytes[1] = static_cast<char>(cp & 0xFF);
        r.count    = 2;
        return r;
    }
    // Supplementary plane
    char32_t            offset = cp - 0x10000;
    auto                high   = static_cast<char16_t>(0xD800 + (offset >> 10));
    auto                low    = static_cast<char16_t>(0xDC00 + (offset & 0x3FF));
    utf16_encode_result r{};
    r.bytes[0] = static_cast<char>((high >> 8) & 0xFF);
    r.bytes[1] = static_cast<char>(high & 0xFF);
    r.bytes[2] = static_cast<char>((low >> 8) & 0xFF);
    r.bytes[3] = static_cast<char>(low & 0xFF);
    r.count    = 4;
    return r;
}

constexpr utf16_encode_result utf16le_encode_one(char32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, 0, true};
    if (cp <= 0xFFFF) {
        utf16_encode_result r{};
        r.bytes[0] = static_cast<char>(cp & 0xFF);
        r.bytes[1] = static_cast<char>((cp >> 8) & 0xFF);
        r.count    = 2;
        return r;
    }
    char32_t            offset = cp - 0x10000;
    auto                high   = static_cast<char16_t>(0xD800 + (offset >> 10));
    auto                low    = static_cast<char16_t>(0xDC00 + (offset & 0x3FF));
    utf16_encode_result r{};
    r.bytes[0] = static_cast<char>(high & 0xFF);
    r.bytes[1] = static_cast<char>((high >> 8) & 0xFF);
    r.bytes[2] = static_cast<char>(low & 0xFF);
    r.bytes[3] = static_cast<char>((low >> 8) & 0xFF);
    r.count    = 4;
    return r;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP
