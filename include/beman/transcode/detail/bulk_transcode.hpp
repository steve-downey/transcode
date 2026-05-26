// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP

#include <beman/transcode/detail/single_byte_tables.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#include <algorithm>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

namespace beman::transcoding {

// ---------------------------------------------------------------------------
// decode_to: Bulk decode legacy bytes to char32_t vector
// ---------------------------------------------------------------------------

template <codec C>
    requires detail::single_byte_codec<C>
std::vector<char32_t> decode_to(legacy_byte_range auto source) {
    std::vector<char32_t> result;
    result.reserve(std::ranges::size(source));
    const auto* table = detail::get_decode_table<C>();
    for (unsigned char byte : source) {
        result.push_back(detail::decode_single_byte_at(byte, table));
    }
    return result;
}

template <codec C>
    requires(!detail::single_byte_codec<C>)
std::vector<char32_t> decode_to(legacy_byte_range auto source) {
    std::vector<char32_t> result;
    for (char32_t cp : source | whatwg_decode<C>)
        result.push_back(cp);
    return result;
}

// ---------------------------------------------------------------------------
// encode_to: Bulk encode char32_t to string (byte string, not vector)
// ---------------------------------------------------------------------------

template <codec C>
    requires detail::single_byte_codec<C>
std::string encode_to(unicode_scalar_range auto source) {
    std::string result;
    result.reserve(std::ranges::size(source));
    const auto* table = detail::get_encode_table<C>();
    for (char32_t cp : source) {
        result.push_back(detail::encode_single_byte_at(cp, table));
    }
    return result;
}

template <codec C>
    requires(!detail::single_byte_codec<C>)
std::string encode_to(unicode_scalar_range auto source) {
    std::string result;
    for (char byte : source | whatwg_encode<C>)
        result.push_back(byte);
    return result;
}

// ---------------------------------------------------------------------------
// encode_to<Container>: Bulk encode char32_t to caller-selected container
// ---------------------------------------------------------------------------

template <typename Container>
concept char_output_container = std::ranges::range<Container> && std::same_as<std::ranges::range_value_t<Container>, char> &&
                                requires(Container& result) { std::back_inserter(result); };

template <typename Container, codec C>
    requires char_output_container<Container> && detail::single_byte_codec<C>
Container encode_to(unicode_scalar_range auto source) {
    Container result;
    if constexpr (std::ranges::sized_range<decltype(source)> && requires(Container& out, std::size_t size) { out.reserve(size); }) {
        result.reserve(static_cast<std::size_t>(std::ranges::size(source)));
    }
    const auto* table = detail::get_encode_table<C>();
    for (char32_t cp : source) {
        result.push_back(detail::encode_single_byte_at(cp, table));
    }
    return result;
}

template <typename Container, codec C>
    requires char_output_container<Container> && (!detail::single_byte_codec<C>)
Container encode_to(unicode_scalar_range auto source) {
    Container result;
    for (char byte : source | whatwg_encode<C>)
        result.push_back(byte);
    return result;
}

// ---------------------------------------------------------------------------
// decode_into: Decode legacy bytes into an output range
// ---------------------------------------------------------------------------

template <codec C>
constexpr void decode_into(legacy_byte_range auto source, std::output_iterator<char32_t> auto sink) {
    for (char32_t cp : source | whatwg_decode<C>)
        *sink++ = cp;
}

// ---------------------------------------------------------------------------
// encode_into: Encode char32_t into an output range
// ---------------------------------------------------------------------------

template <codec C>
constexpr void encode_into(unicode_scalar_range auto source, std::output_iterator<char> auto sink) {
    for (char byte : source | whatwg_encode<C>)
        *sink++ = byte;
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
