// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/concepts.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <type_traits>
    #include <utility>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.pipeline}, transcoding pipelines

//! \omit
template <encoding From, encoding To>
struct transcode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const transcode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const transcode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to transcode; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

//! \seebelow
//! \remarks The name `whatwg::transcode<From, To>` denotes a range adaptor
//! object.  Given a subexpression `E` that models `legacy_byte_range`,
//! `whatwg::transcode<From, To>(E)` and `E | whatwg::transcode<From, To>` are
//! each expression-equivalent to `E | whatwg::decode<From> | whatwg::encode<To>`.
//! An `E` of array type is ill-formed.
//!
//! The composition is the specification: the pipeline decodes and
//! re-encodes one element at a time, and it is lazy because its parts are.
//! A scalar value `To` cannot represent is an encoding error, reported as
//! `whatwg::encode` reports one; a byte sequence `From` does not allow is a
//! decoding error, and the U+FFFD it decodes to is what gets encoded.
template <encoding From, encoding To>
inline constexpr auto transcode = transcode_closure<From, To>{};

// Out-of-line definition: transcode_closure::operator()
template <encoding From, encoding To>
template <legacy_byte_range R>
constexpr auto transcode_closure<From, To>::operator()(R&& r) const {
    return std::forward<R>(r) | decode<From> | encode<To>;
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_VIEW_HPP
