// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_CODEC_RESULT_HPP
#define INCLUDE_BEMAN_TRANSCODE_CODEC_RESULT_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/error.hpp>

namespace beman::transcoding {

// \ref{transcode.custom.reqs}, codec requirements

//! \remarks What a codec's `decode_one` returns.  `is_error` says whether the
//! decode failed.  When it did, `error` says why and `code_point` is
//! U+FFFD REPLACEMENT CHARACTER; when it did not, `code_point` is the decoded
//! Unicode scalar value and `error` is unspecified.
struct decode_result {
    char32_t     code_point{};
    whatwg_error error{};
    bool         is_error{false};
};

//! \remarks What a codec's `encode_one` returns.  `is_error` says whether the
//! encode failed -- the encoding has no representation for that scalar value.
//! When it did not, `count` is the number of bytes written and `bytes[0]`
//! through `bytes[count - 1]` are the encoded form; when it did, `count` is
//! zero and the view substitutes or reports the error as its error kind says.
struct encode_result {
    unsigned char bytes[8]{};
    int           count{0};
    bool          is_error{false};
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_CODEC_RESULT_HPP
