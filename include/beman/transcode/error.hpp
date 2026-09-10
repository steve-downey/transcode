// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ERROR_HPP
#define INCLUDE_BEMAN_TRANSCODE_ERROR_HPP

#include <beman/transcode/config.hpp>

namespace beman::transcoding {

// \ref{transcode.errors}, error types

// whatwg_error — error categories defined by the WHATWG Encoding Standard.
// Used by whatwg_decode_or_error, whatwg_encode_or_error, and all pluggable
// codec _or_error views (decode_or_error_view, encode_or_error_view).
// Pluggable codecs adopt WHATWG error semantics as the common framework;
// custom codec decode_one() and encode_one() return these values directly.
//! \remarks An operation that fails to decode or to encode reports one of
//! these values.  The enumerators have the meanings in the following table.
//! \libtab2[transcode.errors.whatwg]{Enum class `whatwg_error`}
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
//! \cell the sequence encodes a surrogate code point, which is not a Unicode
//! scalar value.
//! \row `out_of_range`
//! \cell the sequence encodes a value greater than the largest Unicode scalar
//! value.
//! \row `unmapped_codepoint`
//! \cell the encoding has no representation for the Unicode scalar value
//! being encoded.
//! \endlibtab2
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
//! \remarks An `iconv` conversion that fails reports one of these values,
//! which are the three failures POSIX `iconv` distinguishes.  The enumerators
//! have the meanings in the following table.
//! \libtab2[transcode.errors.iconv]{Enum class `iconv_error`}
//! \column Constant
//! \column Meaning
//! \row `invalid_sequence`
//! \cell the input is not valid in the source encoding, or has no
//! representation in the destination encoding (`EILSEQ`).
//! \row `incomplete_sequence`
//! \cell the input ends in the middle of a multibyte sequence (`EINVAL`).
//! \row `output_full`
//! \cell the conversion has no room left to write its result (`E2BIG`).
//! \endlibtab2
enum class iconv_error {
    invalid_sequence,
    incomplete_sequence,
    output_full,
};

// transcode_error_kind — how a view reports a codec failure, as a template
// parameter rather than as a separate class template.  Modelled on P2728's
// `to_utf_view_error_kind` (see docs/p2728-alignment.md): an enum rather than a
// bool so that kinds beyond these two can be added, such as one that also
// surfaces the offending input bytes.
//
//   replacement — substitute U+FFFD on decode, '?' on encode
//   expected    — the value type becomes expected<T, whatwg_error>
//! \remarks A view's error kind says how it reports a failure of the codec it
//! drives.  The enumerators have the meanings in the following table.
//! \libtab2[transcode.errors.kind]{Enum class `transcode_error_kind`}
//! \column Constant
//! \column Meaning
//! \row `replacement`
//! \cell a failure to decode yields U+FFFD REPLACEMENT CHARACTER and a
//! failure to encode yields `'?'`, and the view's value type is the codec's
//! own.
//! \row `expected`
//! \cell the view's value type is `expected<T, whatwg_error>`, and a failure
//! yields an `unexpected` holding the error that occurred.
//! \endlibtab2
enum class transcode_error_kind {
    replacement,
    expected,
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ERROR_HPP
