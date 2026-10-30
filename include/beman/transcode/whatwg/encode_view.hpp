// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/whatwg_encode_select.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/error.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

namespace beman::transcoding::detail {

// What `whatwg::encode<C>` and `whatwg::encode_or_error<C>` initialize from.
// `whatwg::encoder<C>` is constrained to the encodings WHATWG defines an
// encoder for, so the check that names the reason comes first.
//! \omit
template <whatwg::encoding C, error_mode E>
constexpr auto make_whatwg_encode_adaptor() {
    static_assert(whatwg_encode_codec<C>, "transcode: WHATWG does not define an encoder for this encoding");
    if constexpr (whatwg_encode_codec<C>) {
        if constexpr (E == error_mode::report)
            return transcoding::encode_or_error(whatwg::encoder<C>{});
        else
            return transcoding::encode(whatwg::encoder<C>{});
    }
}

} // namespace beman::transcoding::detail

namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.encode}, encoding adaptors

//! \seebelow
//! \remarks `whatwg::encode<C>` is `encode(whatwg::encoder<C>{})`
//! \iref{transcode.codec.encode}.  It is the codec-object adaptor over the
//! library's encoder for the encoding `C`, the encoder as the Encoding
//! Standard defines it.  The value type, the reporting of encoding errors,
//! the laziness and the range properties of the view it adapts to are those
//! of that adaptor; the view is random access for the single-byte encodings,
//! which `single_byte_text_encoder` \iref{transcode.codec.reqs} provides.
//! An argument of array type is ill-formed.
//!
//! *Mandates:* The Encoding Standard defines an encoder for `C`.  It defines
//! none for `utf_16be`, `utf_16le`, `replacement` or `x_user_defined`.
template <whatwg::encoding C>
inline constexpr auto encode = detail::make_whatwg_encode_adaptor<C, error_mode::replacement>();

//! \seebelow
//! \remarks `whatwg::encode_or_error<C>` is
//! `encode_or_error(whatwg::encoder<C>{})` \iref{transcode.codec.encode}: it
//! is `whatwg::encode<C>` with `error_mode::report`.  The view it adapts to
//! has value type `expected<char, whatwg::error>`, and an encoding or UTF-32
//! validation error is the error rather than a replacement.
//!
//! *Mandates:* The Encoding Standard defines an encoder for `C`.
template <whatwg::encoding C>
inline constexpr auto encode_or_error = detail::make_whatwg_encode_adaptor<C, error_mode::report>();

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP
