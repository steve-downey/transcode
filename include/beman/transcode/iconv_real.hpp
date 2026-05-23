// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP

#include <beman/transcode/iconv_transcode_view.hpp>

#include <iconv.h>

namespace beman::transcoding {

inline iconv_functions make_real_iconv_fns() noexcept {
    return {::iconv_open, ::iconv, ::iconv_close};
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP
