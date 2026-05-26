// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/single_byte_tables.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#include <cstddef>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

namespace beman::transcoding {

namespace detail {

template <codec C>
inline constexpr bool bulk_single_byte_codec =
    C == codec::ibm866 || C == codec::iso_8859_2 || C == codec::iso_8859_3 || C == codec::iso_8859_4 ||
    C == codec::iso_8859_5 || C == codec::iso_8859_6 || C == codec::iso_8859_7 || C == codec::iso_8859_8 ||
    C == codec::iso_8859_8_i || C == codec::iso_8859_10 || C == codec::iso_8859_13 || C == codec::iso_8859_14 ||
    C == codec::iso_8859_15 || C == codec::iso_8859_16 || C == codec::koi8_r || C == codec::koi8_u ||
    C == codec::macintosh || C == codec::windows_874 || C == codec::windows_1250 || C == codec::windows_1251 ||
    C == codec::windows_1252 || C == codec::windows_1253 || C == codec::windows_1254 || C == codec::windows_1255 ||
    C == codec::windows_1256 || C == codec::windows_1257 || C == codec::windows_1258 || C == codec::x_mac_cyrillic;

template <typename T>
constexpr unsigned char to_unsigned_byte(T value) {
    if constexpr (std::same_as<std::remove_cv_t<T>, std::byte>)
        return std::to_integer<unsigned char>(value);
    else
        return static_cast<unsigned char>(value);
}

template <std::ranges::range R, typename Container>
constexpr void reserve_if_sized(Container& container, R&& range) {
    if constexpr (std::ranges::sized_range<R> &&
                  requires(Container& output, std::size_t size) { output.reserve(size); })
        container.reserve(static_cast<std::size_t>(std::ranges::size(range)));
}

template <typename Container, typename Value>
concept bulk_output_container = requires(Container container, Value value) {
    Container{};
    container.push_back(value);
};

} // namespace detail

template <codec C, legacy_byte_range R>
std::vector<char32_t> decode_to(R&& source) {
    std::vector<char32_t> result;
    detail::reserve_if_sized(result, source);

    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_decode_table<C>();
        for (auto byte : source)
            result.push_back(detail::decode_single_byte_at(detail::to_unsigned_byte(byte), table));
    } else {
        for (char32_t code_point : std::forward<R>(source) | whatwg_decode<C>)
            result.push_back(code_point);
    }

    return result;
}

template <codec C, typename Container = std::string, unicode_scalar_range R>
    requires detail::bulk_output_container<Container, char>
Container encode_to(R&& source) {
    Container result;
    detail::reserve_if_sized(result, source);

    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_encode_table<C>();
        for (char32_t code_point : source)
            result.push_back(detail::encode_single_byte_at(code_point, table));
    } else {
        for (char byte : std::forward<R>(source) | whatwg_encode<C>)
            result.push_back(byte);
    }

    return result;
}

template <codec C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output) {
    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_decode_table<C>();
        for (auto byte : source)
            *output++ = detail::decode_single_byte_at(detail::to_unsigned_byte(byte), table);
    } else {
        for (char32_t code_point : std::forward<R>(source) | whatwg_decode<C>)
            *output++ = code_point;
    }
}

template <codec C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output) {
    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_encode_table<C>();
        for (char32_t code_point : source)
            *output++ = detail::encode_single_byte_at(code_point, table);
    } else {
        for (char byte : std::forward<R>(source) | whatwg_encode<C>)
            *output++ = byte;
    }
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
