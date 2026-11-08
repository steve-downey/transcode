// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>
    #include <optional>

#endif
namespace beman::transcoding::detail {

// Decode one byte using the x-user-defined algorithm.
// Advances current past the consumed byte. Never errors.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr char32_t x_user_defined_decode_one(I& current, [[maybe_unused]] S end) {
    auto byte = static_cast<unsigned char>(*current);
    ++current;
    if (byte < 0x80)
        return static_cast<char32_t>(byte);
    return static_cast<char32_t>(0xF780 + (byte - 0x80));
}

// Encode one scalar value using the x-user-defined algorithm: an ASCII code
// point is its own byte, U+F780 to U+F7FF are 0x80 to 0xFF, and anything else
// has no representation.
constexpr std::optional<unsigned char> x_user_defined_encode_byte(char32_t scalar) {
    if (scalar < 0x80)
        return static_cast<unsigned char>(scalar);
    if (scalar >= 0xF780 && scalar <= 0xF7FF)
        return static_cast<unsigned char>(0x80 + (scalar - 0xF780));
    return std::nullopt;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP
