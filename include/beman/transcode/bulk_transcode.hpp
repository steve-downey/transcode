// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The eager bulk helpers: decode_to, encode_to, decode_into, encode_into, and
// the all-or-first-error forms decode_to_or_error and encode_to_or_error, in
// the form that takes the codec as an argument.  The form that names a WHATWG
// encoding as a template parameter is in <beman/transcode/whatwg/bulk.hpp>.
//
// Every one is a one-line forward to the pipeline it names.  That is the whole
// of the implementation and it is also the whole of the specification: the
// wording says "expression-equivalent to" and shows the pipeline, so there is
// no second description of decoding to keep in step with the first.
//
// They are proposed.  `ranges::to` and `ranges::copy` already do the job, and
// a name that saves no work still saves the reader looking for it -- see
// papers/transcode-view.md, "Bulk conversion to owned storage".

#ifndef INCLUDE_BEMAN_TRANSCODE_BULK_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_BULK_TRANSCODE_HPP

#include <beman/transcode/config.hpp>
#include <beman/transcode/config_generated.hpp>

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/detail/collect_or_error.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/whatwg/error.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <algorithm>
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <string>
    #include <utility>
    #include <vector>

#endif
namespace beman::transcoding {

// A library without `ranges::to` cannot provide the four collecting and
// copying helpers, and says so by not declaring them.  The wording is
// generated from a toolchain that has it; a toolchain that does not would
// drop them from the clause, which `make wording-check` reports as a diff
// rather than passing over in silence.  The _or_error forms below collect
// with a loop of their own and are declared regardless.
#if BEMAN_TRANSCODE_HAS_RANGES_TO

// \ref{transcode.bulk}, eager bulk conversion

template <legacy_byte_range R, text_decoder Codec>
constexpr std::vector<char32_t> decode_to(R&& source, Codec codec);

template <typename Container = std::string, unicode_scalar_range R, text_encoder Codec>
constexpr Container encode_to(R&& source, Codec codec);

template <legacy_byte_range R, text_decoder Codec, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Codec codec, Output output);

template <unicode_scalar_range R, text_encoder Codec, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Codec codec, Output output);

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

// \ref{transcode.bulk}, or the first error

template <legacy_byte_range R, text_decoder Codec>
constexpr std::expected<std::vector<char32_t>, whatwg::error> decode_to_or_error(R&& source, Codec codec);

template <typename Container = std::string, unicode_scalar_range R, text_encoder Codec>
constexpr std::expected<Container, whatwg::error> encode_to_or_error(R&& source, Codec codec);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

#if BEMAN_TRANSCODE_HAS_RANGES_TO

//! \returns-expr
//! \remarks The same four operations as the `whatwg::` forms over a codec
//! supplied as a value \iref{transcode.codec.reqs}, for a codec chosen at run
//! time or one the `whatwg::encoding` enumeration does not name.  As in
//! every eager helper, the source comes first, then what to read or write it
//! as, then any output.
template <legacy_byte_range R, text_decoder Codec>
constexpr std::vector<char32_t> decode_to(R&& source, Codec codec) {
    return std::forward<R>(source) | decode(codec) | std::ranges::to<std::vector<char32_t>>();
}

//! \returns-expr
template <typename Container, unicode_scalar_range R, text_encoder Codec>
constexpr Container encode_to(R&& source, Codec codec) {
    return std::forward<R>(source) | encode(codec) | std::ranges::to<Container>();
}

//! \effects-equiv
template <legacy_byte_range R, text_decoder Codec, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Codec codec, Output output) {
    std::ranges::copy(std::forward<R>(source) | decode(codec), output);
}

//! \effects-equiv
template <unicode_scalar_range R, text_encoder Codec, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Codec codec, Output output) {
    std::ranges::copy(std::forward<R>(source) | encode(codec), output);
}

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

//! \returns Let `v` be `std::forward<R>(source) | decode_or_error(codec)`
//! \iref{transcode.codec.decode}.  If an element of `v` holds an error, the
//! error of the first such element; otherwise a `vector<char32_t>` holding the
//! value of each element of `v`, in order.
//! \remarks This is `decode_to(source, codec)` with the errors reported rather
//! than substituted: the whole text, or the first thing wrong with it.  The
//! elements of `v` after the first error are not examined.  A codec for an
//! encoding chosen at run time is `whatwg::dynamic_decoder`
//! \iref{transcode.whatwg.dynamic}.
template <legacy_byte_range R, text_decoder Codec>
constexpr std::expected<std::vector<char32_t>, whatwg::error> decode_to_or_error(R&& source, Codec codec) {
    return detail::collect_or_error<std::vector<char32_t>, whatwg::error>(std::forward<R>(source) |
                                                                          decode_or_error(codec));
}

//! \returns Let `v` be `std::forward<R>(source) | encode_or_error(codec)`
//! \iref{transcode.codec.encode}.  If an element of `v` holds an error, the
//! error of the first such element; otherwise a `Container` holding the value
//! of each element of `v`, in order.
//! \remarks This is `encode_to<Container>(source, codec)` with the errors
//! reported rather than substituted.  The elements of `v` after the first
//! error are not examined, and the encoder is not flushed past it.
template <typename Container, unicode_scalar_range R, text_encoder Codec>
constexpr std::expected<Container, whatwg::error> encode_to_or_error(R&& source, Codec codec) {
    return detail::collect_or_error<Container, whatwg::error>(std::forward<R>(source) | encode_or_error(codec));
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_BULK_TRANSCODE_HPP
