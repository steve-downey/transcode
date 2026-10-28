// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Which WHATWG codecs decode a byte at a time, and the table each one uses.
// Implementation detail: whether a decode is O(1) per element is a property of
// the view, not a second view, so none of this appears in the wording.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_DECODE_SELECT_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_DECODE_SELECT_HPP

#include <beman/transcode/whatwg/encoding.hpp>
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
#include <beman/transcode/detail/big5.hpp>
#include <beman/transcode/detail/gb18030.hpp>
#include <beman/transcode/detail/iso2022jp.hpp>
#include <beman/transcode/detail/utf16.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <type_traits>

#endif

namespace beman::transcoding::detail {

// What a decode of C has to remember between calls, which for most codecs is
// nothing: a decoder reading a self-delimiting sequence starts each call where
// the last one stopped and needs no state of its own.  The four that do keep
// it in their own headers, and the view's iterator holds one of these and
// compares it, so the iterator declares no codec-specific field.
struct no_decode_state {
    static constexpr bool at_end() { return true; }

    friend constexpr bool operator==(const no_decode_state&, const no_decode_state&) { return true; }
};

template <whatwg::encoding C>
struct decode_state_of {
    using type = no_decode_state;
};

template <>
struct decode_state_of<whatwg::encoding::iso_2022_jp> {
    using type = iso2022jp_decode_state;
};

template <>
struct decode_state_of<whatwg::encoding::utf_16be> {
    using type = utf16_decode_state;
};

template <>
struct decode_state_of<whatwg::encoding::utf_16le> {
    using type = utf16_decode_state;
};

template <>
struct decode_state_of<whatwg::encoding::gbk> {
    using type = gb18030_decode_state;
};

template <>
struct decode_state_of<whatwg::encoding::gb18030> {
    using type = gb18030_decode_state;
};

template <>
struct decode_state_of<whatwg::encoding::big5> {
    using type = big5_decode_state;
};

template <whatwg::encoding C>
using decode_state_t = decode_state_of<C>::type;

// Whether a decoder of C is still to drop a leading U+FEFF: the UTF encodings
// strip a byte order mark from the start of the text, and every other
// encoding keeps nothing for it.
template <whatwg::encoding C>
inline constexpr bool strips_leading_bom =
    C == whatwg::encoding::utf_8 || C == whatwg::encoding::utf_16be || C == whatwg::encoding::utf_16le;

struct leading_bom_state {
    bool first{true};

    friend constexpr bool operator==(const leading_bom_state&, const leading_bom_state&) = default;
};

template <whatwg::encoding C>
using leading_bom_state_t = std::conditional_t<strips_leading_bom<C>, leading_bom_state, no_decode_state>;

//! \expos
template <whatwg::encoding C>
concept random_access_decode_codec =
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
    C == whatwg::encoding::windows_1258 || C == whatwg::encoding::x_mac_cyrillic ||
    C == whatwg::encoding::x_user_defined;

template <whatwg::encoding C>
concept single_byte_codec = random_access_decode_codec<C> || C == whatwg::encoding::replacement;

template <whatwg::encoding C>
concept single_byte_table_codec = random_access_decode_codec<C> && C != whatwg::encoding::x_user_defined;

// The 128-entry table codec C decodes its high half with. Returned by
// reference so single_byte_decode_one, which takes the array, can be handed
// the result directly.
template <whatwg::encoding C>
    requires single_byte_table_codec<C>
consteval const char32_t (&single_byte_decode_table())[128] {
    if constexpr (C == whatwg::encoding::ibm866) {
        return tables::ibm866;
    } else if constexpr (C == whatwg::encoding::iso_8859_2) {
        return tables::iso_8859_2;
    } else if constexpr (C == whatwg::encoding::iso_8859_3) {
        return tables::iso_8859_3;
    } else if constexpr (C == whatwg::encoding::iso_8859_4) {
        return tables::iso_8859_4;
    } else if constexpr (C == whatwg::encoding::iso_8859_5) {
        return tables::iso_8859_5;
    } else if constexpr (C == whatwg::encoding::iso_8859_6) {
        return tables::iso_8859_6;
    } else if constexpr (C == whatwg::encoding::iso_8859_7) {
        return tables::iso_8859_7;
    } else if constexpr (C == whatwg::encoding::iso_8859_8 || C == whatwg::encoding::iso_8859_8_i) {
        return tables::iso_8859_8;
    } else if constexpr (C == whatwg::encoding::iso_8859_10) {
        return tables::iso_8859_10;
    } else if constexpr (C == whatwg::encoding::iso_8859_13) {
        return tables::iso_8859_13;
    } else if constexpr (C == whatwg::encoding::iso_8859_14) {
        return tables::iso_8859_14;
    } else if constexpr (C == whatwg::encoding::iso_8859_15) {
        return tables::iso_8859_15;
    } else if constexpr (C == whatwg::encoding::iso_8859_16) {
        return tables::iso_8859_16;
    } else if constexpr (C == whatwg::encoding::koi8_r) {
        return tables::koi8_r;
    } else if constexpr (C == whatwg::encoding::koi8_u) {
        return tables::koi8_u;
    } else if constexpr (C == whatwg::encoding::macintosh) {
        return tables::macintosh;
    } else if constexpr (C == whatwg::encoding::windows_874) {
        return tables::windows_874;
    } else if constexpr (C == whatwg::encoding::windows_1250) {
        return tables::windows_1250;
    } else if constexpr (C == whatwg::encoding::windows_1251) {
        return tables::windows_1251;
    } else if constexpr (C == whatwg::encoding::windows_1252) {
        return tables::windows_1252;
    } else if constexpr (C == whatwg::encoding::windows_1253) {
        return tables::windows_1253;
    } else if constexpr (C == whatwg::encoding::windows_1254) {
        return tables::windows_1254;
    } else if constexpr (C == whatwg::encoding::windows_1255) {
        return tables::windows_1255;
    } else if constexpr (C == whatwg::encoding::windows_1256) {
        return tables::windows_1256;
    } else if constexpr (C == whatwg::encoding::windows_1257) {
        return tables::windows_1257;
    } else if constexpr (C == whatwg::encoding::windows_1258) {
        return tables::windows_1258;
    } else {
        static_assert(C == whatwg::encoding::x_mac_cyrillic,
                      "single_byte_decode_table: codec has no single-byte table");
        return tables::x_mac_cyrillic;
    }
}

// The same table as a pointer, or null for a codec that decodes its high half
// algorithmically rather than from a table (x_user_defined).
template <whatwg::encoding C>
consteval const char32_t* random_access_decode_table() {
    if constexpr (single_byte_table_codec<C>)
        return single_byte_decode_table<C>();
    else
        return nullptr;
}

constexpr char32_t decode_random_access_code_unit(unsigned char byte, const char32_t* table) {
    if (byte < 0x80)
        return static_cast<char32_t>(byte);
    char32_t cp = table[byte - 0x80];
    return cp == 0 ? U'\xFFFD' : cp;
}

constexpr char32_t decode_x_user_defined_code_unit(unsigned char byte) {
    return byte < 0x80 ? static_cast<char32_t>(byte) : static_cast<char32_t>(0xF780 + (byte - 0x80));
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_DECODE_SELECT_HPP
