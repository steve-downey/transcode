// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP

#include <beman/transcode/iconv_transcode_or_error_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <iconv.h>
#include <span>

namespace beman::transcoding {

inline iconv_functions make_real_iconv_fns() noexcept { return {::iconv_open, ::iconv, ::iconv_close}; }

// Pipe factory: input | iconv_transcode("UTF-8", "UTF-32LE", buf)
inline auto iconv_transcode(const char* from, const char* to, std::span<char> buf) {
    return iconv_transcode_closure<iconv_functions>{make_real_iconv_fns(), from, to, buf};
}

// Pipe factory: input | iconv_transcode_or_error("UTF-8", "UTF-32LE", buf)
inline auto iconv_transcode_or_error(const char* from, const char* to, std::span<char> buf) {
    return iconv_transcode_or_error_closure<iconv_functions>{make_real_iconv_fns(), from, to, buf};
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP
