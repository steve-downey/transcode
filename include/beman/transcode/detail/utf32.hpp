// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF32_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF32_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/error.hpp>

namespace beman::transcoding::detail {

struct utf32_validation_result {
    char32_t     code_point{};
    whatwg_error error{};
    bool         is_error{false};
};

// Interpret one char32_t as UTF-32. Ill-formed code units become U+FFFD so
// replacement-mode callers can pass the scalar value through their codec;
// error-reporting callers retain the reason it was ill-formed.
constexpr utf32_validation_result validate_utf32(char32_t code_unit) {
    if (code_unit >= 0xD800 && code_unit <= 0xDFFF)
        return {U'\xFFFD', whatwg_error::surrogate_code_point, true};
    if (code_unit > 0x10FFFF)
        return {U'\xFFFD', whatwg_error::out_of_range, true};
    return {code_unit, {}, false};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF32_HPP
