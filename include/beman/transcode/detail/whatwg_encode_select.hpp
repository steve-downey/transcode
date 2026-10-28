// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Which WHATWG codecs encode a code point at a time, the table each one uses,
// and the lookup itself. Implementation detail; see whatwg_decode_select.hpp.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_ENCODE_SELECT_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_ENCODE_SELECT_HPP

#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/iso2022jp.hpp>
#include <beman/transcode/concepts.hpp>
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

// What an encode of C has to remember between calls, which for every codec but
// ISO-2022-JP is nothing: an encoder that writes a self-delimiting sequence
// starts each call in the same place.  The view's iterator holds one of these
// and compares it, so the iterator declares no codec-specific field.
struct no_encode_state {
    friend constexpr bool operator==(const no_encode_state&, const no_encode_state&) { return true; }
};

template <whatwg::encoding C>
struct encode_state_of {
    using type = no_encode_state;
};

template <>
struct encode_state_of<whatwg::encoding::iso_2022_jp> {
    using type = iso2022jp_encode_state;
};

template <whatwg::encoding C>
using encode_state_t = encode_state_of<C>::type;

//! \expos
template <whatwg::encoding C>
concept random_access_encode_codec =
    C == whatwg::encoding::ibm866 || C == whatwg::encoding::iso_8859_2 || C == whatwg::encoding::iso_8859_3 ||
    C == whatwg::encoding::iso_8859_4 || C == whatwg::encoding::iso_8859_5 || C == whatwg::encoding::iso_8859_6 ||
    C == whatwg::encoding::iso_8859_7 || C == whatwg::encoding::iso_8859_8 || C == whatwg::encoding::iso_8859_8_i ||
    C == whatwg::encoding::iso_8859_10 || C == whatwg::encoding::iso_8859_13 || C == whatwg::encoding::iso_8859_14 ||
    C == whatwg::encoding::iso_8859_15 || C == whatwg::encoding::iso_8859_16 || C == whatwg::encoding::koi8_r ||
    C == whatwg::encoding::koi8_u || C == whatwg::encoding::macintosh || C == whatwg::encoding::windows_874 ||
    C == whatwg::encoding::windows_1250 || C == whatwg::encoding::windows_1251 ||
    C == whatwg::encoding::windows_1252 || C == whatwg::encoding::windows_1253 ||
    C == whatwg::encoding::windows_1254 || C == whatwg::encoding::windows_1255 ||
    C == whatwg::encoding::windows_1256 || C == whatwg::encoding::windows_1257 ||
    C == whatwg::encoding::windows_1258 || C == whatwg::encoding::x_mac_cyrillic;

// WHATWG defines a decoder for every codec but an encoder for only some of
// them: replacement, x-user-defined and the UTF-16 pair decode only. Naming one
// of those in an encode pipeline is a compile-time error, not a runtime one.
//! \expos
template <whatwg::encoding C>
concept whatwg_encode_codec =
    C == whatwg::encoding::utf_8 || random_access_encode_codec<C> || C == whatwg::encoding::gbk ||
    C == whatwg::encoding::gb18030 || C == whatwg::encoding::big5 || C == whatwg::encoding::shift_jis ||
    C == whatwg::encoding::euc_jp || C == whatwg::encoding::iso_2022_jp || C == whatwg::encoding::euc_kr;

//! \expos
template <whatwg::encoding C, typename R>
concept whatwg_encode_input = unicode_scalar_range<R> && whatwg_encode_codec<C>;

template <whatwg::encoding C>
consteval const char32_t (&random_access_encode_table())[128] {
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
    else if constexpr (C == whatwg::encoding::iso_8859_8 || C == whatwg::encoding::iso_8859_8_i)
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
