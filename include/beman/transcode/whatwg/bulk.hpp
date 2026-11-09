// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The eager bulk helpers in their WHATWG form: decode_to, encode_to,
// decode_into and encode_into, each taking the encoding either as a template
// parameter or as a value chosen at run time, and the all-or-first-error forms
// decode_to_or_error and encode_to_or_error.  The codec-object forms are in
// <beman/transcode/bulk_transcode.hpp>.
//
// Every one is a one-line forward to the pipeline it names.  That is the whole
// of the implementation and it is also the whole of the specification: the
// wording says "expression-equivalent to" and shows the pipeline, so there is
// no second description of decoding to keep in step with the first.

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_BULK_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_BULK_HPP

#include <beman/transcode/config.hpp>
#include <beman/transcode/config_generated.hpp>

#include <beman/transcode/bulk_transcode.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/detail/whatwg_dispatch.hpp>
#include <beman/transcode/detail/whatwg_encode_select.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <algorithm>
    #include <expected>
    #include <iterator>
    #include <optional>
    #include <ranges>
    #include <string>
    #include <string_view>
    #include <type_traits>
    #include <utility>
    #include <vector>

#endif
namespace beman::transcoding::whatwg {

// A library without `ranges::to` cannot provide the collecting and copying
// helpers, and says so by not declaring them.  The wording is generated from
// a toolchain that has it; a toolchain that does not would drop them from the
// clause, which `make wording-check` reports as a diff rather than passing
// over in silence.  The _or_error forms collect with a loop of their own and
// are declared regardless.
#if BEMAN_TRANSCODE_HAS_RANGES_TO

// \ref{transcode.bulk}, eager bulk conversion

template <encoding C, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source);

template <encoding C, typename Container = std::string, unicode_scalar_range R>
constexpr Container encode_to(R&& source);

template <encoding C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output);

template <encoding C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output);

// \ref{transcode.bulk}, with the encoding chosen at run time

template <legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source, encoding e);

template <legacy_byte_range R>
constexpr std::optional<std::vector<char32_t>> decode_to(R&& source, std::string_view label);

template <typename Container = std::string, unicode_scalar_range R>
constexpr std::optional<Container> encode_to(R&& source, encoding e);

template <typename Container = std::string, unicode_scalar_range R>
constexpr std::optional<Container> encode_to(R&& source, std::string_view label);

template <legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, encoding e, Output output);

template <unicode_scalar_range R, std::output_iterator<char> Output>
constexpr bool encode_into(R&& source, encoding e, Output output);

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

// \ref{transcode.bulk}, or the first error

template <encoding C, legacy_byte_range R>
constexpr std::expected<std::vector<char32_t>, error> decode_to_or_error(R&& source);

template <encoding C, typename Container = std::string, unicode_scalar_range R>
constexpr std::expected<Container, error> encode_to_or_error(R&& source);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

#if BEMAN_TRANSCODE_HAS_RANGES_TO

//! \returns-expr
//! \remarks Each of these is the pipeline it names, run to completion.  They
//! add no behaviour to it: the errors are the decoding view's, substituted
//! rather than reported \iref{transcode.whatwg.decode}, and a caller who wants
//! them reported writes the pipeline with `whatwg::decode_or_error` instead.
template <encoding C, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source) {
    return std::forward<R>(source) | decode<C> | std::ranges::to<std::vector<char32_t>>();
}

//! \returns-expr
//! \remarks `Container` is the caller's choice of result, defaulted to
//! `string`.
template <encoding C, typename Container, unicode_scalar_range R>
constexpr Container encode_to(R&& source) {
    return std::forward<R>(source) | encode<C> | std::ranges::to<Container>();
}

//! \effects-equiv
template <encoding C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | decode<C>, output);
}

//! \effects-equiv
template <encoding C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | encode<C>, output);
}

//! \returns `whatwg::decode_to<E>(std::forward<R>(source))`, where `E` is the
//! enumerator equal to `e`.
//! \remarks The encoding is examined once, not once per element.
template <legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source, encoding e) {
    return beman::transcoding::detail::with_encoding(e, [&]<encoding E>(std::integral_constant<encoding, E>) {
        return whatwg::decode_to<E>(std::forward<R>(source));
    });
}

//! \returns `nullopt` if `whatwg::get_encoding(label)` \iref{transcode.whatwg.label}
//! is `nullopt`, and otherwise `whatwg::decode_to(std::forward<R>(source),
//! *whatwg::get_encoding(label))`.
template <legacy_byte_range R>
constexpr std::optional<std::vector<char32_t>> decode_to(R&& source, std::string_view label) {
    const auto e = whatwg::get_encoding(label);
    if (!e)
        return std::nullopt;
    return whatwg::decode_to(std::forward<R>(source), *e);
}

//! \returns `nullopt` if the Encoding Standard defines no encoder for `e`,
//! and otherwise `whatwg::encode_to<E, Container>(std::forward<R>(source))`,
//! where `E` is the enumerator equal to `e`.
//! \remarks The encoding is examined once, not once per element.
template <typename Container, unicode_scalar_range R>
constexpr std::optional<Container> encode_to(R&& source, encoding e) {
    return beman::transcoding::detail::with_encoding(
        e, [&]<encoding E>(std::integral_constant<encoding, E>) -> std::optional<Container> {
            if constexpr (beman::transcoding::detail::whatwg_encode_codec<E>)
                return whatwg::encode_to<E, Container>(std::forward<R>(source));
            else
                return std::nullopt;
        });
}

//! \returns `nullopt` if `whatwg::get_encoding(label)` \iref{transcode.whatwg.label}
//! is `nullopt`, and otherwise `whatwg::encode_to<Container>(std::forward<R>(source),
//! *whatwg::get_encoding(label))`.
template <typename Container, unicode_scalar_range R>
constexpr std::optional<Container> encode_to(R&& source, std::string_view label) {
    const auto e = whatwg::get_encoding(label);
    if (!e)
        return std::nullopt;
    return whatwg::encode_to<Container>(std::forward<R>(source), *e);
}

//! \effects Equivalent to `whatwg::decode_into<E>(std::forward<R>(source),
//! output)`, where `E` is the enumerator equal to `e`.
template <legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, encoding e, Output output) {
    beman::transcoding::detail::with_encoding(e, [&]<encoding E>(std::integral_constant<encoding, E>) {
        whatwg::decode_into<E>(std::forward<R>(source), output);
    });
}

//! \effects If the Encoding Standard defines an encoder for `e`, equivalent to
//! `whatwg::encode_into<E>(std::forward<R>(source), output)`, where `E` is the
//! enumerator equal to `e`; otherwise none.
//! \returns Whether the Encoding Standard defines an encoder for `e`.
template <unicode_scalar_range R, std::output_iterator<char> Output>
constexpr bool encode_into(R&& source, encoding e, Output output) {
    return beman::transcoding::detail::with_encoding(e, [&]<encoding E>(std::integral_constant<encoding, E>) {
        if constexpr (beman::transcoding::detail::whatwg_encode_codec<E>) {
            whatwg::encode_into<E>(std::forward<R>(source), output);
            return true;
        } else {
            return false;
        }
    });
}

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

//! \returns `decode_to_or_error(std::forward<R>(source), whatwg::decoder<C>{})`
//! \iref{transcode.bulk}.
//! \remarks `whatwg::decode_to<C>(source)` with the errors reported rather than
//! substituted: the whole text as scalar values, or the first error the
//! Encoding Standard's decoder for `C` reports in it.  For an encoding chosen
//! at run time, `decode_to_or_error(source, whatwg::dynamic_decoder(e))`
//! \iref{transcode.whatwg.dynamic}.
template <encoding C, legacy_byte_range R>
constexpr std::expected<std::vector<char32_t>, error> decode_to_or_error(R&& source) {
    return transcoding::decode_to_or_error(std::forward<R>(source), whatwg::decoder<C>{});
}

//! \returns `encode_to_or_error<Container>(std::forward<R>(source),
//! whatwg::encoder<C>{})` \iref{transcode.bulk}.
//! \remarks `whatwg::encode_to<C, Container>(source)` with the errors
//! reported rather than substituted: the whole text as bytes, or the first
//! scalar value that is not a Unicode scalar value or that `C` has no
//! representation for.
//!
//! *Mandates:* The Encoding Standard defines an encoder for `C`.
template <encoding C, typename Container, unicode_scalar_range R>
constexpr std::expected<Container, error> encode_to_or_error(R&& source) {
    return transcoding::encode_to_or_error<Container>(std::forward<R>(source), whatwg::encoder<C>{});
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_BULK_HPP
