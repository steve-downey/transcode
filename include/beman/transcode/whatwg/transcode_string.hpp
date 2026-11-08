// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_STRING_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_STRING_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/whatwg_dispatch.hpp>
#include <beman/transcode/detail/whatwg_encode_select.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <optional>
    #include <span>
    #include <string>
    #include <string_view>
    #include <type_traits>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.string}, eager transcoding

constexpr bool has_encoder(whatwg::encoding encoding) noexcept;

std::optional<std::string> transcode_string(std::span<const char> src, whatwg::encoding from, whatwg::encoding to);

std::optional<std::string>
transcode_string(std::span<const char> src, std::string_view from_label, std::string_view to_label);

// ---------------------------------------------------------------------------
// detail helpers
// ---------------------------------------------------------------------------

namespace detail {

//! \omit
template <whatwg::encoding C>
std::u32string transcode_decode_all(std::span<const char> src);

//! \omit
template <whatwg::encoding C>
std::string transcode_encode_all(std::u32string_view src);

} // namespace detail

// ---------------------------------------------------------------------------
// Out-of-line definitions: detail helpers
// ---------------------------------------------------------------------------

namespace detail {

template <whatwg::encoding C>
std::u32string transcode_decode_all(std::span<const char> src) {
    std::u32string result;
    for (char32_t cp : src | decode<C>)
        result.push_back(cp);
    return result;
}

template <whatwg::encoding C>
std::string transcode_encode_all(std::u32string_view src) {
    std::string result;
    for (char b : src | encode<C>)
        result.push_back(b);
    return result;
}

} // namespace detail

// ---------------------------------------------------------------------------
// Out-of-line definitions: public API
// ---------------------------------------------------------------------------

//! \returns `true` if the WHATWG Encoding Standard defines an encoder for
//! `encoding`, and `false` otherwise.
constexpr bool has_encoder(whatwg::encoding encoding) noexcept {
    return encoding != whatwg::encoding::utf_16be && encoding != whatwg::encoding::utf_16le &&
           encoding != whatwg::encoding::replacement;
}

//! \returns `nullopt` if the WHATWG Encoding Standard defines no encoder for
//! `to`; otherwise, the bytes of `src`, decoded as `from` and re-encoded as
//! `to`.
//! Errors are substituted rather than reported: a byte sequence `from` does
//! not allow decodes to U+FFFD, and a scalar value `to` cannot represent
//! encodes to `'?'`.
//! \remarks This is `transcode` \iref{transcode.pipeline} run to completion
//! into a `string`, which is what a caller who wants the whole result and not
//! a view writes.  `src | whatwg::transcode<From, To> | ranges::to<string>()` is the
//! same thing with the codecs known at compile time.
inline std::optional<std::string>
transcode_string(std::span<const char> src, whatwg::encoding from, whatwg::encoding to) {
    if (!has_encoder(to))
        return std::nullopt;

    // Each encoding is examined once, and the loop it selects is the one a
    // caller naming the encoding at compile time would get.
    const std::u32string intermediate = beman::transcoding::detail::with_encoding(
        from, [&]<whatwg::encoding E>(std::integral_constant<whatwg::encoding, E>) {
            return detail::transcode_decode_all<E>(src);
        });
    return beman::transcoding::detail::with_encoding(
        to, [&]<whatwg::encoding E>(std::integral_constant<whatwg::encoding, E>) -> std::optional<std::string> {
            if constexpr (beman::transcoding::detail::whatwg_encode_codec<E>)
                return detail::transcode_encode_all<E>(intermediate);
            else
                return std::nullopt;
        });
}

//! \returns `whatwg::transcode_string(src, *whatwg::get_encoding(from_label),
//! *whatwg::get_encoding(to_label))`, and `nullopt` if either label names no encoding
//! \iref{transcode.whatwg.label} or the target encoding has no WHATWG encoder.
//! \remarks This is the overload a program uses when the encodings are a
//! runtime choice -- a `Content-Type` header, a command-line option -- which
//! is what labels are for.
inline std::optional<std::string>
transcode_string(std::span<const char> src, std::string_view from_label, std::string_view to_label) {
    auto from = whatwg::get_encoding(from_label);
    auto to   = whatwg::get_encoding(to_label);
    if (!from || !to)
        return std::nullopt;
    return transcode_string(src, *from, *to);
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_TRANSCODE_STRING_HPP
