// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#include <cstddef>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

namespace beman::transcoding {

namespace detail {

template <codec C>
inline constexpr bool bulk_single_byte_codec = C == codec::ibm866 || C == codec::iso_8859_2 || C == codec::iso_8859_3 ||
                                               C == codec::iso_8859_4 || C == codec::iso_8859_5 || C == codec::iso_8859_6 ||
                                               C == codec::iso_8859_7 || C == codec::iso_8859_8 || C == codec::iso_8859_8_i ||
                                               C == codec::iso_8859_10 || C == codec::iso_8859_13 || C == codec::iso_8859_14 ||
                                               C == codec::iso_8859_15 || C == codec::iso_8859_16 || C == codec::koi8_r ||
                                               C == codec::koi8_u || C == codec::macintosh || C == codec::windows_874 ||
                                               C == codec::windows_1250 || C == codec::windows_1251 || C == codec::windows_1252 ||
                                               C == codec::windows_1253 || C == codec::windows_1254 || C == codec::windows_1255 ||
                                               C == codec::windows_1256 || C == codec::windows_1257 || C == codec::windows_1258 ||
                                               C == codec::x_mac_cyrillic;

template <codec C>
consteval const char32_t* get_single_byte_table() {
    if constexpr (C == codec::ibm866)
        return tables::ibm866;
    else if constexpr (C == codec::iso_8859_2)
        return tables::iso_8859_2;
    else if constexpr (C == codec::iso_8859_3)
        return tables::iso_8859_3;
    else if constexpr (C == codec::iso_8859_4)
        return tables::iso_8859_4;
    else if constexpr (C == codec::iso_8859_5)
        return tables::iso_8859_5;
    else if constexpr (C == codec::iso_8859_6)
        return tables::iso_8859_6;
    else if constexpr (C == codec::iso_8859_7)
        return tables::iso_8859_7;
    else if constexpr (C == codec::iso_8859_8)
        return tables::iso_8859_8;
    else if constexpr (C == codec::iso_8859_8_i)
        return tables::iso_8859_8;
    else if constexpr (C == codec::iso_8859_10)
        return tables::iso_8859_10;
    else if constexpr (C == codec::iso_8859_13)
        return tables::iso_8859_13;
    else if constexpr (C == codec::iso_8859_14)
        return tables::iso_8859_14;
    else if constexpr (C == codec::iso_8859_15)
        return tables::iso_8859_15;
    else if constexpr (C == codec::iso_8859_16)
        return tables::iso_8859_16;
    else if constexpr (C == codec::koi8_r)
        return tables::koi8_r;
    else if constexpr (C == codec::koi8_u)
        return tables::koi8_u;
    else if constexpr (C == codec::macintosh)
        return tables::macintosh;
    else if constexpr (C == codec::windows_874)
        return tables::windows_874;
    else if constexpr (C == codec::windows_1250)
        return tables::windows_1250;
    else if constexpr (C == codec::windows_1251)
        return tables::windows_1251;
    else if constexpr (C == codec::windows_1252)
        return tables::windows_1252;
    else if constexpr (C == codec::windows_1253)
        return tables::windows_1253;
    else if constexpr (C == codec::windows_1254)
        return tables::windows_1254;
    else if constexpr (C == codec::windows_1255)
        return tables::windows_1255;
    else if constexpr (C == codec::windows_1256)
        return tables::windows_1256;
    else if constexpr (C == codec::windows_1257)
        return tables::windows_1257;
    else if constexpr (C == codec::windows_1258)
        return tables::windows_1258;
    else if constexpr (C == codec::x_mac_cyrillic)
        return tables::x_mac_cyrillic;
}

template <typename T>
constexpr unsigned char to_unsigned_byte(T value) {
    if constexpr (std::same_as<std::remove_cv_t<T>, std::byte>)
        return std::to_integer<unsigned char>(value);
    else
        return static_cast<unsigned char>(value);
}

template <std::ranges::range R, typename Container>
constexpr void reserve_if_sized(Container& container, R&& range) {
    if constexpr (std::ranges::sized_range<R>)
        container.reserve(static_cast<std::size_t>(std::ranges::size(range)));
}

template <typename Container, typename Value>
concept bulk_output_container = requires(Container container, Value value) {
    Container{};
    container.push_back(value);
};

constexpr char32_t decode_single_byte(unsigned char byte, const char32_t* table) {
    if (byte < 0x80)
        return static_cast<char32_t>(byte);
    char32_t code_point = table[byte - 0x80];
    return code_point == 0 ? U'\xFFFD' : code_point;
}

constexpr char encode_single_byte(char32_t code_point, const char32_t* table) {
    if (code_point < 0x80)
        return static_cast<char>(code_point);
    for (int index = 0; index < 128; ++index) {
        if (table[index] == code_point)
            return static_cast<char>(0x80 + index);
    }
    return '?';
}

} // namespace detail

template <codec C, legacy_byte_range R>
std::vector<char32_t> decode_to(R&& source) {
    std::vector<char32_t> result;
    detail::reserve_if_sized(result, source);

    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_single_byte_table<C>();
        for (auto byte : source)
            result.push_back(detail::decode_single_byte(detail::to_unsigned_byte(byte), table));
    }
    else {
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
        const auto* table = detail::get_single_byte_table<C>();
        for (char32_t code_point : source)
            result.push_back(detail::encode_single_byte(code_point, table));
    }
    else {
        for (char byte : std::forward<R>(source) | whatwg_encode<C>)
            result.push_back(byte);
    }

    return result;
}

template <codec C, legacy_byte_range R, std::output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output) {
    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_single_byte_table<C>();
        for (auto byte : source)
            *output++ = detail::decode_single_byte(detail::to_unsigned_byte(byte), table);
    }
    else {
        for (char32_t code_point : std::forward<R>(source) | whatwg_decode<C>)
            *output++ = code_point;
    }
}

template <codec C, unicode_scalar_range R, std::output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output) {
    if constexpr (detail::bulk_single_byte_codec<C>) {
        const auto* table = detail::get_single_byte_table<C>();
        for (char32_t code_point : source)
            *output++ = detail::encode_single_byte(code_point, table);
    }
    else {
        for (char byte : std::forward<R>(source) | whatwg_encode<C>)
            *output++ = byte;
    }
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_BULK_TRANSCODE_HPP
