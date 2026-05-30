// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_ENCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_ENCODE_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>

namespace beman::transcoding::detail {

struct utf8_encode_result {
    char         bytes[4]{};
    int          count{};
    whatwg_error error{};
    bool         is_error{false};
};

constexpr utf8_encode_result utf8_encode_one(char32_t cp);

constexpr utf8_encode_result utf8_encode_one(char32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, 0, whatwg_error::surrogate_code_point, true};
    if (cp > 0x10FFFF)
        return {{}, 0, whatwg_error::out_of_range, true};

    utf8_encode_result r{};
    if (cp < 0x80) {
        r.bytes[0] = static_cast<char>(cp);
        r.count    = 1;
    } else if (cp < 0x800) {
        r.bytes[0] = static_cast<char>(0xC0 | (cp >> 6));
        r.bytes[1] = static_cast<char>(0x80 | (cp & 0x3F));
        r.count    = 2;
    } else if (cp < 0x10000) {
        r.bytes[0] = static_cast<char>(0xE0 | (cp >> 12));
        r.bytes[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        r.bytes[2] = static_cast<char>(0x80 | (cp & 0x3F));
        r.count    = 3;
    } else {
        r.bytes[0] = static_cast<char>(0xF0 | (cp >> 18));
        r.bytes[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        r.bytes[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        r.bytes[3] = static_cast<char>(0x80 | (cp & 0x3F));
        r.count    = 4;
    }
    return r;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_ENCODE_HPP
