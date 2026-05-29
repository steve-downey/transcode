// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_ERROR_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_ERROR_HPP

#include <beman/transcode/config.hpp>

namespace beman::transcoding {

// whatwg_error — error categories defined by the WHATWG Encoding Standard.
// Used by whatwg_decode_or_error, whatwg_encode_or_error, and all pluggable
// codec _or_error views (decode_or_error_view, encode_or_error_view).
// Pluggable codecs adopt WHATWG error semantics as the common framework;
// custom codec decode_one() and encode_one() return these values directly.
enum class whatwg_error {
    invalid_byte,
    truncated_sequence,
    overlong_encoding,
    surrogate_code_point,
    out_of_range,
    unmapped_codepoint,
};

// iconv_error — error categories that map to POSIX iconv errno values.
// Used only by iconv_transcode_or_error_view and iconv_transcode_to_or_error.
// Kept separate from whatwg_error because iconv reports at the OS level:
// EILSEQ (invalid_sequence), EINVAL (incomplete_sequence), E2BIG (output_full).
// The OS cannot distinguish WHY a byte sequence is invalid, only that it is.
enum class iconv_error {
    invalid_sequence,
    incomplete_sequence,
    output_full,
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_ERROR_HPP
