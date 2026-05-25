// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SNIFF_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SNIFF_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/null_term.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>

#include <cstdint>
#include <optional>
#include <ranges>

namespace beman::transcoding {

// Detects encoding by examining a BOM at the start of the byte range.
// WHATWG Encoding Standard §8.2 "Determining the fallback encoding".
// Returns:
//   codec::utf_8     if the range begins with EF BB BF
//   codec::utf_16be  if the range begins with FE FF
//   codec::utf_16le  if the range begins with FF FE
//   std::nullopt     otherwise
template <legacy_byte_range R>
constexpr std::optional<codec> sniff_encoding(R&& r) noexcept;

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

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SNIFF_HPP
