// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.decode}, decoding adaptors

//! \seebelow
//! \remarks `whatwg::decode<C>` is `decode(whatwg::decoder<C>{})`
//! \iref{transcode.codec.decode}.  It is the codec-object adaptor over the
//! library's decoder for the encoding `C`, the decoder as the Encoding
//! Standard defines it.  The value type, the reporting of decoding errors,
//! the laziness and the range properties of the view it adapts to are those
//! of that adaptor.  An argument of array type is ill-formed: a string literal
//! carries a terminating null character that a decode would render, and
//! `views::null_term` \iref{null.term.adaptor} is how a null-terminated
//! sequence becomes a range.
template <whatwg::encoding C>
inline constexpr auto decode = transcoding::decode(whatwg::decoder<C>{});

//! \seebelow
//! \remarks `whatwg::decode_or_error<C>` is
//! `decode_or_error(whatwg::decoder<C>{})` \iref{transcode.codec.decode}: it
//! is `whatwg::decode<C>` with `error_mode::report`.  The view it adapts to
//! has value type `expected<char32_t, whatwg::error>`, and a decoding error
//! is the error rather than U+FFFD.
template <whatwg::encoding C>
inline constexpr auto decode_or_error = transcoding::decode_or_error(whatwg::decoder<C>{});

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
