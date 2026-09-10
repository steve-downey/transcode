// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The eager bulk helpers: decode_to, encode_to, decode_into, encode_into,
// each in a WHATWG form taking the codec as a template parameter and a
// pluggable form taking it as an argument.
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

// A library without `ranges::to` cannot provide these, and says so by not
// declaring them.  The wording is generated from a toolchain that has it; a
// toolchain that does not would drop the whole clause, which `make
// wording-check` reports as a diff rather than passing over in silence.
#if BEMAN_TRANSCODE_HAS_RANGES_TO

    #include <beman/transcode/codec_concepts.hpp>
    #include <beman/transcode/concepts.hpp>
    #include <beman/transcode/decode_view.hpp>
    #include <beman/transcode/encode_view.hpp>
    #include <beman/transcode/whatwg_decode_view.hpp>
    #include <beman/transcode/whatwg_encode_view.hpp>

    #if !BEMAN_TRANSCODE_USE_MODULES()
        #include <algorithm>
        #include <ranges>
        #include <string>
        #include <vector>

    #endif
namespace beman::transcoding {

// \ref{transcode.bulk}, eager bulk conversion

template <codec C, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source);

template <codec C, typename Container = std::string, unicode_scalar_range R>
constexpr Container encode_to(R&& source);

template <codec C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output);

template <codec C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output);

template <decode_codec Codec, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(Codec codec, R&& source);

template <encode_codec Codec, typename Container = std::string, unicode_scalar_range R>
constexpr Container encode_to(Codec codec, R&& source);

template <decode_codec Codec, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(Codec codec, R&& source, Output output);

template <encode_codec Codec, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(Codec codec, R&& source, Output output);

// ---------------------------------------------------------------------------
// Out-of-line definitions: WHATWG codec, as a template parameter
// ---------------------------------------------------------------------------

//! \returns-equiv
//! \remarks Each of these is the pipeline it names, run to completion.  They
//! add no behaviour to it: the errors are the decoding view's, substituted
//! rather than reported \iref{transcode.whatwg.decode}, and a caller who wants
//! them reported writes the pipeline with `whatwg_decode_or_error` instead.
//! What they add is the name, for the case a caller wants the whole result
//! rather than a view over it.
template <codec C, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source) {
    return std::forward<R>(source) | whatwg_decode<C> | std::ranges::to<std::vector<char32_t>>();
}

//! \returns-equiv
//! \remarks `Container` is the caller's choice of result, defaulted to
//! `string` because that is what encoded bytes are usually wanted in.
template <codec C, typename Container, unicode_scalar_range R>
constexpr Container encode_to(R&& source) {
    return std::forward<R>(source) | whatwg_encode<C> | std::ranges::to<Container>();
}

//! \effects-equiv
template <codec C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | whatwg_decode<C>, output);
}

//! \effects-equiv
template <codec C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | whatwg_encode<C>, output);
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: pluggable codec, as an argument
// ---------------------------------------------------------------------------

//! \returns-equiv
//! \remarks The same four operations over a codec supplied as a value
//! \iref{transcode.custom.reqs}, for a codec chosen at run time or one the
//! `codec` enumeration does not name.
template <decode_codec Codec, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(Codec codec, R&& source) {
    return std::forward<R>(source) | decode(codec) | std::ranges::to<std::vector<char32_t>>();
}

//! \returns-equiv
template <encode_codec Codec, typename Container, unicode_scalar_range R>
constexpr Container encode_to(Codec codec, R&& source) {
    return std::forward<R>(source) | encode(codec) | std::ranges::to<Container>();
}

//! \effects-equiv
template <decode_codec Codec, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(Codec codec, R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | decode(codec), output);
}

//! \effects-equiv
template <encode_codec Codec, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(Codec codec, R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | encode(codec), output);
}

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO
#endif // INCLUDE_BEMAN_TRANSCODE_BULK_TRANSCODE_HPP
