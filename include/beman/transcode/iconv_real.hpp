// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/iconv_transcode_or_error_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <iconv.h>
#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <span>

#endif
namespace beman::transcoding {

//! \returns An `iconv_functions` whose members are the implementation's
//! `iconv_open`, `iconv` and `iconv_close`.
inline iconv_functions make_real_iconv_fns() noexcept { return {::iconv_open, ::iconv, ::iconv_close}; }

//! \seebelow
//! \returns A range adaptor object.  Given a subexpression `E` that models
//! `legacy_byte_range`, `iconv_transcode(from, to, buf)(E)` and
//! `E | iconv_transcode(from, to, buf)` are each expression-equivalent to an
//! `iconv_transcode_view` \iref{transcode.iconv} over `E`,
//! `make_real_iconv_fns()`, `from`, `to` and `buf`.
//! \remarks `buf` is the caller's, and every iterator the adaptor produces
//! converts into it.  Two ranges adapted with the same buffer must not be
//! iterated at the same time.
inline auto iconv_transcode(const char* from, const char* to, std::span<char> buf) {
    return iconv_transcode_closure<iconv_functions>{make_real_iconv_fns(), from, to, buf};
}

//! \seebelow
//! \returns `iconv_transcode(from, to, buf)` with the errors reported rather
//! than skipped: the view it adapts to has value type
//! `expected<char, iconv_error>`.
inline auto iconv_transcode_or_error(const char* from, const char* to, std::span<char> buf) {
    return iconv_transcode_or_error_closure<iconv_functions>{make_real_iconv_fns(), from, to, buf};
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_REAL_HPP
