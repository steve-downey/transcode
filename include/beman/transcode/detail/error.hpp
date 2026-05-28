// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_ERROR_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_ERROR_HPP

#include <beman/transcode/config.hpp>

namespace beman::transcoding {

enum class whatwg_error {
    invalid_byte,
    truncated_sequence,
    overlong_encoding,
    surrogate_code_point,
    out_of_range,
    unmapped_codepoint,
};

enum class iconv_error {
    invalid_sequence,
    incomplete_sequence,
    output_full,
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_ERROR_HPP
