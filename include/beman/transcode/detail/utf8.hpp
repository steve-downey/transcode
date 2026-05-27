// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
#include <iterator>

#endif
namespace beman::transcoding::detail {

struct utf8_result {
    char32_t     code_point{};
    whatwg_error error{};
    bool         is_error{false};
};

// Decode one UTF-8 codepoint from [current, end) following the WHATWG spec.
// Advances current past consumed bytes.  On a bad continuation byte, current
// is NOT advanced past the bad byte so the caller can re-process it.
// Returns a value with is_error=false on success, is_error=true on failure.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr utf8_result utf8_decode_one(I& current, S end) {
    auto byte = static_cast<unsigned char>(*current);
    ++current;

    if (byte < 0x80)
        return {static_cast<char32_t>(byte), {}, false};

    int      extra;
    char32_t cp;
    if (byte >= 0xC2 && byte <= 0xDF) {
        extra = 1;
        cp    = byte & 0x1F;
    } else if (byte >= 0xE0 && byte <= 0xEF) {
        extra = 2;
        cp    = byte & 0x0F;
    } else if (byte >= 0xF0 && byte <= 0xF4) {
        extra = 3;
        cp    = byte & 0x07;
    } else {
        // 0x80–0xBF: unexpected continuation; 0xC0–0xC1: always-overlong;
        // 0xF5–0xFF: out-of-range leads.
        return {{}, whatwg_error::invalid_byte, true};
    }

    for (int i = 0; i < extra; ++i) {
        if (current == end)
            return {{}, whatwg_error::truncated_sequence, true};
        auto cont = static_cast<unsigned char>(*current);
        if ((cont & 0xC0) != 0x80)
            return {{}, whatwg_error::invalid_byte, true}; // do NOT advance
        // WHATWG: validate first continuation byte ranges.
        if (i == 0) {
            if (byte == 0xE0 && cont < 0xA0)
                return {{}, whatwg_error::overlong_encoding, true};
            if (byte == 0xED && cont > 0x9F)
                return {{}, whatwg_error::surrogate_code_point, true};
            if (byte == 0xF0 && cont < 0x90)
                return {{}, whatwg_error::overlong_encoding, true};
            if (byte == 0xF4 && cont > 0x8F)
                return {{}, whatwg_error::out_of_range, true};
        }
        ++current;
        cp = (cp << 6) | (cont & 0x3F);
    }

    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, whatwg_error::surrogate_code_point, true};
    if (cp > 0x10FFFF)
        return {{}, whatwg_error::out_of_range, true};

    return {cp, {}, false};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_HPP
