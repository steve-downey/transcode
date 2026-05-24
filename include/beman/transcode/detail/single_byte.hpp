// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP

#include <beman/transcode/detail/error.hpp>

#include <iterator>

namespace beman::transcoding::detail {

struct single_byte_result {
    char32_t     code_point{};
    whatwg_error error{};
    bool         is_error{false};
};

// Decode one byte using a 128-entry lookup table for the upper half.
// table[i] maps byte (0x80 + i). A table entry of 0 means "unmapped" (error).
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr single_byte_result single_byte_decode_one(I& current, [[maybe_unused]] S end, const char32_t (&table)[128]) {
    auto byte = static_cast<unsigned char>(*current);
    ++current;
    if (byte < 0x80)
        return {static_cast<char32_t>(byte), {}, false};
    char32_t cp = table[byte - 0x80];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};
    return {cp, {}, false};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP
