// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP

#include <beman/transcode/config.hpp>
#include <beman/transcode/detail/reverse_index.hpp>
#include <beman/transcode/whatwg/error.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>

#endif
namespace beman::transcoding::detail {

struct single_byte_result {
    char32_t      code_point{};
    whatwg::error error{};
    bool          is_error{false};
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
        return {{}, whatwg::error::invalid_byte, true};
    return {cp, {}, false};
}

struct single_byte_encode_result {
    unsigned char byte{};
    bool          is_error{false};
};

// Encode one validated code point to a single legacy byte.
// Encode one code point against a 128-entry upper-half table.
// ASCII (cp < 0x80) is a direct passthrough.  For cp >= 0x80 the reverse
// index of Table gives the lowest i with Table[i] == cp; the byte is 0x80 + i.
template <table_ref Table>
constexpr single_byte_encode_result single_byte_encode_one(char32_t cp) {
    if (cp < 0x80)
        return {static_cast<unsigned char>(cp), false};
    int pointer = reverse_index_for<Table>.find(cp);
    if (pointer < 0)
        return {{}, true};
    return {static_cast<unsigned char>(0x80 + pointer), false};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP
