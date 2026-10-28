// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_TABLES_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_TABLES_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/detail/reverse_index.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

namespace beman::transcoding::detail {

template <whatwg::encoding C>
consteval const char32_t* get_decode_table() {
    if constexpr (C == whatwg::encoding::ibm866)
        return tables::ibm866;
    else if constexpr (C == whatwg::encoding::iso_8859_2)
        return tables::iso_8859_2;
    else if constexpr (C == whatwg::encoding::iso_8859_3)
        return tables::iso_8859_3;
    else if constexpr (C == whatwg::encoding::iso_8859_4)
        return tables::iso_8859_4;
    else if constexpr (C == whatwg::encoding::iso_8859_5)
        return tables::iso_8859_5;
    else if constexpr (C == whatwg::encoding::iso_8859_6)
        return tables::iso_8859_6;
    else if constexpr (C == whatwg::encoding::iso_8859_7)
        return tables::iso_8859_7;
    else if constexpr (C == whatwg::encoding::iso_8859_8)
        return tables::iso_8859_8;
    else if constexpr (C == whatwg::encoding::iso_8859_8_i)
        return tables::iso_8859_8;
    else if constexpr (C == whatwg::encoding::iso_8859_10)
        return tables::iso_8859_10;
    else if constexpr (C == whatwg::encoding::iso_8859_13)
        return tables::iso_8859_13;
    else if constexpr (C == whatwg::encoding::iso_8859_14)
        return tables::iso_8859_14;
    else if constexpr (C == whatwg::encoding::iso_8859_15)
        return tables::iso_8859_15;
    else if constexpr (C == whatwg::encoding::iso_8859_16)
        return tables::iso_8859_16;
    else if constexpr (C == whatwg::encoding::koi8_r)
        return tables::koi8_r;
    else if constexpr (C == whatwg::encoding::koi8_u)
        return tables::koi8_u;
    else if constexpr (C == whatwg::encoding::macintosh)
        return tables::macintosh;
    else if constexpr (C == whatwg::encoding::windows_874)
        return tables::windows_874;
    else if constexpr (C == whatwg::encoding::windows_1250)
        return tables::windows_1250;
    else if constexpr (C == whatwg::encoding::windows_1251)
        return tables::windows_1251;
    else if constexpr (C == whatwg::encoding::windows_1252)
        return tables::windows_1252;
    else if constexpr (C == whatwg::encoding::windows_1253)
        return tables::windows_1253;
    else if constexpr (C == whatwg::encoding::windows_1254)
        return tables::windows_1254;
    else if constexpr (C == whatwg::encoding::windows_1255)
        return tables::windows_1255;
    else if constexpr (C == whatwg::encoding::windows_1256)
        return tables::windows_1256;
    else if constexpr (C == whatwg::encoding::windows_1257)
        return tables::windows_1257;
    else if constexpr (C == whatwg::encoding::windows_1258)
        return tables::windows_1258;
    else if constexpr (C == whatwg::encoding::x_mac_cyrillic)
        return tables::x_mac_cyrillic;
    else if constexpr (C == whatwg::encoding::replacement)
        return nullptr;
    else if constexpr (C == whatwg::encoding::x_user_defined)
        return nullptr;
}

template <whatwg::encoding C>
consteval const char32_t* get_encode_table() {
    if constexpr (C == whatwg::encoding::ibm866)
        return tables::ibm866;
    else if constexpr (C == whatwg::encoding::iso_8859_2)
        return tables::iso_8859_2;
    else if constexpr (C == whatwg::encoding::iso_8859_3)
        return tables::iso_8859_3;
    else if constexpr (C == whatwg::encoding::iso_8859_4)
        return tables::iso_8859_4;
    else if constexpr (C == whatwg::encoding::iso_8859_5)
        return tables::iso_8859_5;
    else if constexpr (C == whatwg::encoding::iso_8859_6)
        return tables::iso_8859_6;
    else if constexpr (C == whatwg::encoding::iso_8859_7)
        return tables::iso_8859_7;
    else if constexpr (C == whatwg::encoding::iso_8859_8)
        return tables::iso_8859_8;
    else if constexpr (C == whatwg::encoding::iso_8859_8_i)
        return tables::iso_8859_8;
    else if constexpr (C == whatwg::encoding::iso_8859_10)
        return tables::iso_8859_10;
    else if constexpr (C == whatwg::encoding::iso_8859_13)
        return tables::iso_8859_13;
    else if constexpr (C == whatwg::encoding::iso_8859_14)
        return tables::iso_8859_14;
    else if constexpr (C == whatwg::encoding::iso_8859_15)
        return tables::iso_8859_15;
    else if constexpr (C == whatwg::encoding::iso_8859_16)
        return tables::iso_8859_16;
    else if constexpr (C == whatwg::encoding::koi8_r)
        return tables::koi8_r;
    else if constexpr (C == whatwg::encoding::koi8_u)
        return tables::koi8_u;
    else if constexpr (C == whatwg::encoding::macintosh)
        return tables::macintosh;
    else if constexpr (C == whatwg::encoding::windows_874)
        return tables::windows_874;
    else if constexpr (C == whatwg::encoding::windows_1250)
        return tables::windows_1250;
    else if constexpr (C == whatwg::encoding::windows_1251)
        return tables::windows_1251;
    else if constexpr (C == whatwg::encoding::windows_1252)
        return tables::windows_1252;
    else if constexpr (C == whatwg::encoding::windows_1253)
        return tables::windows_1253;
    else if constexpr (C == whatwg::encoding::windows_1254)
        return tables::windows_1254;
    else if constexpr (C == whatwg::encoding::windows_1255)
        return tables::windows_1255;
    else if constexpr (C == whatwg::encoding::windows_1256)
        return tables::windows_1256;
    else if constexpr (C == whatwg::encoding::windows_1257)
        return tables::windows_1257;
    else if constexpr (C == whatwg::encoding::windows_1258)
        return tables::windows_1258;
    else if constexpr (C == whatwg::encoding::x_mac_cyrillic)
        return tables::x_mac_cyrillic;
    else if constexpr (C == whatwg::encoding::replacement)
        return nullptr;
    else if constexpr (C == whatwg::encoding::x_user_defined)
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

// The byte for cp in codec C, or '?' when C cannot represent it.  Codecs
// without an encode table (replacement, x-user-defined) encode only ASCII.
template <whatwg::encoding C>
constexpr char encode_single_byte_at(char32_t cp) {
    if (cp < 0x80)
        return static_cast<char>(cp);
    if constexpr (get_encode_table<C>() == nullptr) {
        return '?';
    } else {
        int pointer = reverse_index_for<table_ref{get_encode_table<C>(), 128}>.find(cp);
        return pointer < 0 ? '?' : static_cast<char>(0x80 + pointer);
    }
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_TABLES_HPP
