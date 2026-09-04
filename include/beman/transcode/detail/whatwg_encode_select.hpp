// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Which WHATWG codecs encode a code point at a time, the table each one uses,
// and the lookup itself. Implementation detail; see whatwg_decode_select.hpp.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_ENCODE_SELECT_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_ENCODE_SELECT_HPP

#include <beman/transcode/codec.hpp>
#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/tables/ibm866.hpp>
#include <beman/transcode/detail/tables/iso_8859_10.hpp>
#include <beman/transcode/detail/tables/iso_8859_13.hpp>
#include <beman/transcode/detail/tables/iso_8859_14.hpp>
#include <beman/transcode/detail/tables/iso_8859_15.hpp>
#include <beman/transcode/detail/tables/iso_8859_16.hpp>
#include <beman/transcode/detail/tables/iso_8859_2.hpp>
#include <beman/transcode/detail/tables/iso_8859_3.hpp>
#include <beman/transcode/detail/tables/iso_8859_4.hpp>
#include <beman/transcode/detail/tables/iso_8859_5.hpp>
#include <beman/transcode/detail/tables/iso_8859_6.hpp>
#include <beman/transcode/detail/tables/iso_8859_7.hpp>
#include <beman/transcode/detail/tables/iso_8859_8.hpp>
#include <beman/transcode/detail/tables/koi8_r.hpp>
#include <beman/transcode/detail/tables/koi8_u.hpp>
#include <beman/transcode/detail/tables/macintosh.hpp>
#include <beman/transcode/detail/tables/windows_1250.hpp>
#include <beman/transcode/detail/tables/windows_1251.hpp>
#include <beman/transcode/detail/tables/windows_1252.hpp>
#include <beman/transcode/detail/tables/windows_1253.hpp>
#include <beman/transcode/detail/tables/windows_1254.hpp>
#include <beman/transcode/detail/tables/windows_1255.hpp>
#include <beman/transcode/detail/tables/windows_1256.hpp>
#include <beman/transcode/detail/tables/windows_1257.hpp>
#include <beman/transcode/detail/tables/windows_1258.hpp>
#include <beman/transcode/detail/tables/windows_874.hpp>
#include <beman/transcode/detail/tables/x_mac_cyrillic.hpp>

namespace beman::transcoding::detail {

template <codec C>
concept random_access_encode_codec =
    C == codec::ibm866 || C == codec::iso_8859_2 || C == codec::iso_8859_3 || C == codec::iso_8859_4 ||
    C == codec::iso_8859_5 || C == codec::iso_8859_6 || C == codec::iso_8859_7 || C == codec::iso_8859_8 ||
    C == codec::iso_8859_8_i || C == codec::iso_8859_10 || C == codec::iso_8859_13 || C == codec::iso_8859_14 ||
    C == codec::iso_8859_15 || C == codec::iso_8859_16 || C == codec::koi8_r || C == codec::koi8_u ||
    C == codec::macintosh || C == codec::windows_874 || C == codec::windows_1250 || C == codec::windows_1251 ||
    C == codec::windows_1252 || C == codec::windows_1253 || C == codec::windows_1254 || C == codec::windows_1255 ||
    C == codec::windows_1256 || C == codec::windows_1257 || C == codec::windows_1258 || C == codec::x_mac_cyrillic;

template <codec C>
consteval const char32_t (&random_access_encode_table())[128] {
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
    else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i)
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
    else
        return tables::x_mac_cyrillic;
}

constexpr char encode_random_access_code_point(char32_t cp, const char32_t (&table)[128]) {
    if (cp < 0x80)
        return static_cast<char>(cp);
    for (int index = 0; index < 128; ++index) {
        if (table[index] == cp)
            return static_cast<char>(0x80 + index);
    }
    return '?';
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_ENCODE_SELECT_HPP
