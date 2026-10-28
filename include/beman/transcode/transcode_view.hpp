// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/encode_view.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <type_traits>
    #include <utility>

#endif
namespace beman::transcoding {

// \ref{transcode.pipeline}, transcoding pipelines

// ---------------------------------------------------------------------------
// pluggable_transcode_closure — pipe adaptor for pluggable codec pairs
// ---------------------------------------------------------------------------

//! \omit
template <text_decoder From, text_encoder To>
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
//! `transcode(from, to)(E)` and `E | transcode(from, to)`
//! are each expression-equivalent to `E | decode(from) | encode(to)`.
//! \remarks This is `whatwg::transcode` \iref{transcode.pipeline} with codecs
//! supplied as values rather than named by enumerators.
template <text_decoder From, text_encoder To>
constexpr pluggable_transcode_closure<From, To> transcode(From from, To to) {
    return {from, to};
}

// Out-of-line definition: pluggable_transcode_closure::operator()
template <text_decoder From, text_encoder To>
template <legacy_byte_range R>
constexpr auto pluggable_transcode_closure<From, To>::operator()(R&& r) const {
    return std::forward<R>(r) | decode(from_) | encode(to_);
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_VIEW_HPP
