// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/whatwg_encode_select.hpp>
#include <beman/transcode/transcode_view.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

namespace beman::transcoding::detail {

// What `whatwg::transcode<From, To>` initializes from.  `whatwg::encoder<To>`
// is constrained to the encodings WHATWG defines an encoder for, so the check
// that names the reason comes first.
//! \omit
template <whatwg::encoding From, whatwg::encoding To>
constexpr auto make_whatwg_transcode_adaptor() {
    static_assert(whatwg_encode_codec<To>, "transcode: WHATWG does not define an encoder for this encoding");
    if constexpr (whatwg_encode_codec<To>)
        return transcoding::transcode(whatwg::decoder<From>{}, whatwg::encoder<To>{});
}

} // namespace beman::transcoding::detail

namespace beman::transcoding::whatwg {

// \ref{transcode.pipeline}, transcoding pipelines

//! \seebelow
//! \remarks `whatwg::transcode<From, To>` is
//! `transcode(whatwg::decoder<From>{}, whatwg::encoder<To>{})`
//! \iref{transcode.pipeline}, and so is `E | whatwg::decode<From> |
//! whatwg::encode<To>` for a subexpression `E` that models
//! `legacy_byte_range`.  An `E` of array type is ill-formed.
//!
//! The composition is the specification: the pipeline decodes and
//! re-encodes one element at a time, and it is lazy because its parts are.
//! A scalar value `To` cannot represent is an encoding error, reported as
//! `whatwg::encode` reports one; a byte sequence `From` does not allow is a
//! decoding error, and the U+FFFD it decodes to is what gets encoded.
//!
//! *Mandates:* The Encoding Standard defines an encoder for `To`.
template <encoding From, encoding To>
inline constexpr auto transcode = detail::make_whatwg_transcode_adaptor<From, To>();

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_VIEW_HPP
