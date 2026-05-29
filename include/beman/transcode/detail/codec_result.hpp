// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_CODEC_RESULT_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_CODEC_RESULT_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/error.hpp>

namespace beman::transcoding {

struct decode_result {
    char32_t     code_point{};
    whatwg_error error{};
    bool         is_error{false};
};

struct encode_result {
    unsigned char bytes[8]{};
    int           count{0};
    bool          is_error{false};
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_CODEC_RESULT_HPP
