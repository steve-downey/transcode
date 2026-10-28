// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ERROR_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ERROR_HPP

#include <beman/transcode/config.hpp>

namespace beman::transcoding::whatwg {

// \ref{transcode.errors}, error types

// error — error categories defined by the WHATWG Encoding Standard.
// Used by whatwg_decode_or_error, whatwg_encode_or_error, and all pluggable
// codec _or_error views (decode_or_error_view, encode_or_error_view).
// Pluggable codecs adopt WHATWG error semantics as the common framework;
// custom codec decode_one() and encode_one() return these values directly.
//! \remarks An operation that fails to decode or to encode reports one of
//! these values.  The enumerators have the meanings in the following table.
//! \libtab2[transcode.errors.whatwg]{Enum class `whatwg::error`}
//! \column Constant
//! \column Meaning
//! \row `invalid_byte`
//! \cell the input holds a byte the encoding does not allow in that
//! position.
//! \row `truncated_sequence`
//! \cell the input ends in the middle of a sequence.
//! \row `overlong_encoding`
//! \cell the sequence encodes a value that a shorter sequence also encodes.
//! \row `surrogate_code_point`
//! \cell a byte sequence decodes to, or a UTF-32 input code unit holds, a
//! surrogate code point, which is not a Unicode scalar value.
//! \row `out_of_range`
//! \cell a byte sequence decodes to, or a UTF-32 input code unit holds, a value
//! greater than the largest Unicode scalar value.
//! \row `unmapped_codepoint`
//! \cell the encoding has no representation for the Unicode scalar value
//! being encoded.
//! \endlibtab2
enum class error {
    invalid_byte,
    truncated_sequence,
    overlong_encoding,
    surrogate_code_point,
    out_of_range,
    unmapped_codepoint,
};

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ERROR_HPP
