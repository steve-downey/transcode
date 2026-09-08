// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

namespace beman::transcoding {

// \ref{transcode.pipeline}, transcoding pipelines

//! \omit
template <codec From, codec To>
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
//! \remarks The name `transcode<From, To>` denotes a range adaptor object.
//! Given a subexpression `E` that models `legacy_byte_range`,
//! `transcode<From, To>(E)` and `E | transcode<From, To>` are each
//! expression-equivalent to `E | whatwg_decode<From> | whatwg_encode<To>`.
//! An `E` of array type is ill-formed.
//!
//! The composition is the specification: the pipeline decodes and
//! re-encodes one element at a time, and it is lazy because its parts are.
//! A scalar value `To` cannot represent is an encoding error, reported as
//! `whatwg_encode` reports one; a byte sequence `From` does not allow is a
//! decoding error, and the U+FFFD it decodes to is what gets encoded.
template <codec From, codec To>
inline constexpr auto transcode = transcode_closure<From, To>{};

// Out-of-line definition: transcode_closure::operator()
template <codec From, codec To>
template <legacy_byte_range R>
constexpr auto transcode_closure<From, To>::operator()(R&& r) const {
    return std::forward<R>(r) | whatwg_decode<From> | whatwg_encode<To>;
}

// ---------------------------------------------------------------------------
// pluggable_transcode_closure — pipe adaptor for pluggable codec pairs
// ---------------------------------------------------------------------------

//! \omit
template <decode_codec From, encode_codec To>
struct pluggable_transcode_closure {
    From from_;
    To   to_;

    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const pluggable_transcode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const pluggable_transcode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to transcode; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

//! \seebelow
//! \returns A range adaptor object over `from` and `to`.  Given a
//! subexpression `E` that models `legacy_byte_range`,
//! `pluggable_transcode(from, to)(E)` and `E | pluggable_transcode(from, to)`
//! are each expression-equivalent to `E | decode(from) | encode(to)`.
//! \remarks This is `transcode` \iref{transcode.pipeline} with codecs supplied
//! as values rather than named by enumerators.
template <decode_codec From, encode_codec To>
constexpr pluggable_transcode_closure<From, To> pluggable_transcode(From from, To to) {
    return {from, to};
}

// Out-of-line definition: pluggable_transcode_closure::operator()
template <decode_codec From, encode_codec To>
template <legacy_byte_range R>
constexpr auto pluggable_transcode_closure<From, To>::operator()(R&& r) const {
    return std::forward<R>(r) | decode(from_) | encode(to_);
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_VIEW_HPP
