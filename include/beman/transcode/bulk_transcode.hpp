// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The eager bulk helpers: decode_to, encode_to, decode_into, encode_into,
// in the form that takes the codec as an argument.  The form that names a
// WHATWG encoding as a template parameter is in
// <beman/transcode/whatwg/bulk.hpp>.
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

    #if !BEMAN_TRANSCODE_USE_MODULES()
        #include <algorithm>
        #include <iterator>
        #include <ranges>
        #include <string>
        #include <utility>
        #include <vector>

    #endif
namespace beman::transcoding {

// \ref{transcode.bulk}, eager bulk conversion

template <text_decoder Codec, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(Codec codec, R&& source);

template <text_encoder Codec, typename Container = std::string, unicode_scalar_range R>
constexpr Container encode_to(Codec codec, R&& source);

template <text_decoder Codec, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(Codec codec, R&& source, Output output);

template <text_encoder Codec, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(Codec codec, R&& source, Output output);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

//! \returns-expr
//! \remarks The same four operations as the `whatwg::` forms over a codec
//! supplied as a value \iref{transcode.codec.reqs}, for a codec chosen at run
//! time or one the `whatwg::encoding` enumeration does not name.
template <text_decoder Codec, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(Codec codec, R&& source) {
    return std::forward<R>(source) | decode(codec) | std::ranges::to<std::vector<char32_t>>();
}

//! \returns-expr
template <text_encoder Codec, typename Container, unicode_scalar_range R>
constexpr Container encode_to(Codec codec, R&& source) {
    return std::forward<R>(source) | encode(codec) | std::ranges::to<Container>();
}

//! \effects-equiv
template <text_decoder Codec, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(Codec codec, R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | decode(codec), output);
}

//! \effects-equiv
template <text_encoder Codec, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(Codec codec, R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | encode(codec), output);
}

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO
#endif // INCLUDE_BEMAN_TRANSCODE_BULK_TRANSCODE_HPP
