// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The eager bulk helpers in their WHATWG form: decode_to, encode_to,
// decode_into and encode_into, each taking the encoding as a template
// parameter.  The codec-object forms are in <beman/transcode/bulk_transcode.hpp>.
//
// Every one is a one-line forward to the pipeline it names.  That is the whole
// of the implementation and it is also the whole of the specification: the
// wording says "expression-equivalent to" and shows the pipeline, so there is
// no second description of decoding to keep in step with the first.

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_BULK_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_BULK_HPP

#include <beman/transcode/config.hpp>
#include <beman/transcode/config_generated.hpp>

// A library without `ranges::to` cannot provide these, and says so by not
// declaring them.  The wording is generated from a toolchain that has it; a
// toolchain that does not would drop the whole clause, which `make
// wording-check` reports as a diff rather than passing over in silence.
#if BEMAN_TRANSCODE_HAS_RANGES_TO

    #include <beman/transcode/concepts.hpp>
    #include <beman/transcode/whatwg/decode_view.hpp>
    #include <beman/transcode/whatwg/encode_view.hpp>
    #include <beman/transcode/whatwg/encoding.hpp>

    #if !BEMAN_TRANSCODE_USE_MODULES()
        #include <algorithm>
        #include <iterator>
        #include <ranges>
        #include <string>
        #include <utility>
        #include <vector>

    #endif
namespace beman::transcoding::whatwg {

// \ref{transcode.bulk}, eager bulk conversion

template <encoding C, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(R&& source);

template <encoding C, typename Container = std::string, unicode_scalar_range R>
constexpr Container encode_to(R&& source);

template <encoding C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output);

template <encoding C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

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

} // namespace beman::transcoding::whatwg

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO
#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_BULK_HPP
