// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP

#include <iterator>

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

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP
