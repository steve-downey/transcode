// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_TABLES_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_TABLES_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/whatwg_decode_view.hpp>

namespace beman::transcoding::detail {

template <codec C>
consteval const char32_t* get_decode_table() {
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
    else if constexpr (C == codec::replacement)
        return nullptr;
    else if constexpr (C == codec::x_user_defined)
        return nullptr;
}

template <codec C>
consteval const char32_t* get_encode_table() {
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
    else if constexpr (C == codec::replacement)
        return nullptr;
    else if constexpr (C == codec::x_user_defined)
        return nullptr;
}

constexpr char32_t decode_single_byte_at(unsigned char byte, const char32_t* table) {
    if (byte < 0x80)
        return static_cast<char32_t>(byte);
    if (table == nullptr)
        return static_cast<char32_t>(byte);
    char32_t cp = table[byte - 0x80];
    if (cp == 0)
        return U'\xFFFD';
    return cp;
}

constexpr char encode_single_byte_at(char32_t cp, const char32_t* table) {
    if (cp < 0x80)
        return static_cast<char>(cp);
    if (table == nullptr)
        return '?';
    for (int i = 0; i < 128; ++i) {
        if (table[i] == cp)
            return static_cast<char>(0x80 + i);
    }
    return '?';
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_TABLES_HPP
