// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Convenience helpers: decode_to, encode_to, decode_into, encode_into.
//
// These are thin wrappers over `view | ranges::to<Container>()` and
// `ranges::copy(view, output)`.  They are NOT proposed for standardization
// because the standard library already provides the necessary composition
// tools (ranges::to, ranges::copy) and the views compose naturally with
// them.  This header exists for practical convenience in codebases that
// prefer a named-function call style.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP

#include <beman/transcode/config.hpp>
#include <beman/transcode/config_generated.hpp>

#if BEMAN_TRANSCODE_HAS_RANGES_TO

    #include <beman/transcode/decode_view.hpp>
    #include <beman/transcode/detail/codec_concepts.hpp>
    #include <beman/transcode/detail/concepts.hpp>
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

// ---------------------------------------------------------------------------
// WHATWG codec bulk helpers (codec as NTTP enum)
// ---------------------------------------------------------------------------

template <codec C, legacy_byte_range R>
std::vector<char32_t> decode_to(R&& source) {
    return std::forward<R>(source) | whatwg_decode<C> | std::ranges::to<std::vector<char32_t>>();
}

template <codec C, typename Container = std::string, unicode_scalar_range R>
Container encode_to(R&& source) {
    return std::forward<R>(source) | whatwg_encode<C> | std::ranges::to<Container>();
}

template <codec C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | whatwg_decode<C>, output);
}

template <codec C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | whatwg_encode<C>, output);
}

// ---------------------------------------------------------------------------
// Pluggable codec bulk helpers (codec as first argument)
// ---------------------------------------------------------------------------

template <decode_codec Codec, legacy_byte_range R>
constexpr std::vector<char32_t> decode_to(Codec codec, R&& source) {
    return std::forward<R>(source) | decode(codec) | std::ranges::to<std::vector<char32_t>>();
}

template <encode_codec Codec, typename Container = std::string, unicode_scalar_range R>
constexpr Container encode_to(Codec codec, R&& source) {
    return std::forward<R>(source) | encode(codec) | std::ranges::to<Container>();
}

template <decode_codec Codec, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(Codec codec, R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | decode(codec), output);
}

template <encode_codec Codec, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(Codec codec, R&& source, Output output) {
    std::ranges::copy(std::forward<R>(source) | encode(codec), output);
}

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_HAS_RANGES_TO
#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
