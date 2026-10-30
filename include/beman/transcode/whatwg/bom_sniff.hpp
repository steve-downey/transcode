// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_BOM_SNIFF_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_BOM_SNIFF_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/concepts.hpp>
#include <beman/transcode/null_term.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <cstdint>
    #include <optional>
    #include <ranges>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.sniff}, byte order mark sniffing

template <legacy_byte_range R>
    requires std::ranges::forward_range<R>
constexpr std::optional<encoding> bom_sniff(R&& r) noexcept;

//! \returns The encoding `r` begins with a byte order mark for:
//! \item `whatwg::encoding::utf_8`, if `r` begins with `EF BB BF`;
//! \item `whatwg::encoding::utf_16be`, if `r` begins with `FE FF`;
//! \item `whatwg::encoding::utf_16le`, if `r` begins with `FF FE`;
//! \item `nullopt` otherwise.
//! \remarks The mark is not consumed: `r` can be passed on unchanged.
template <legacy_byte_range R>
    requires std::ranges::forward_range<R>
constexpr std::optional<encoding> bom_sniff(R&& r) noexcept {
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
        return whatwg::encoding::utf_16be;
    if (b0 == 0xFF && b1 == 0xFE)
        return whatwg::encoding::utf_16le;

    if (it == end)
        return std::nullopt;
    auto b2 = static_cast<std::uint8_t>(*it);

    if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF)
        return whatwg::encoding::utf_8;

    return std::nullopt;
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_BOM_SNIFF_HPP
