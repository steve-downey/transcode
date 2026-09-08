// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_SNIFF_HPP
#define INCLUDE_BEMAN_TRANSCODE_SNIFF_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/concepts.hpp>
#include <beman/transcode/null_term.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <cstdint>
    #include <optional>
    #include <ranges>

#endif
namespace beman::transcoding {

// \ref{transcode.codec.sniff}, byte order mark sniffing

template <legacy_byte_range R>
constexpr std::optional<codec> sniff_encoding(R&& r) noexcept;

//! \returns The encoding `r` begins with a byte order mark for:
//! \item `codec::utf_8`, if `r` begins with `EF BB BF`;
//! \item `codec::utf_16be`, if `r` begins with `FE FF`;
//! \item `codec::utf_16le`, if `r` begins with `FF FE`;
//! \item `nullopt` otherwise.
//! \remarks This is the byte order mark half of the Encoding Standard's
//! "decide the fallback encoding" step: what to do when there is no mark is a
//! question about the document and its transport, which a library cannot
//! answer.  The mark itself is not consumed -- a decode view strips a leading
//! U+FEFF for the UTF codecs on its own -- so the range can be passed on
//! unchanged.
template <legacy_byte_range R>
constexpr std::optional<codec> sniff_encoding(R&& r) noexcept {
    auto it  = std::ranges::begin(r);
    auto end = std::ranges::end(r);

    if (it == end)
        return std::nullopt;
    auto b0 = static_cast<std::uint8_t>(*it);
    ++it;

    if (it == end)
        return std::nullopt;
    auto b1 = static_cast<std::uint8_t>(*it);
    ++it;

    if (b0 == 0xFE && b1 == 0xFF)
        return codec::utf_16be;
    if (b0 == 0xFF && b1 == 0xFE)
        return codec::utf_16le;

    if (it == end)
        return std::nullopt;
    auto b2 = static_cast<std::uint8_t>(*it);

    if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF)
        return codec::utf_8;

    return std::nullopt;
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_SNIFF_HPP
