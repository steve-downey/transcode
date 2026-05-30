// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_STRING_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_STRING_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/labels.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <optional>
    #include <span>
    #include <string>
    #include <string_view>

#endif
namespace beman::transcoding {

// Decode bytes from `from` codec and re-encode to `to` codec.
// Decode errors yield U+FFFD; unmapped encode codepoints yield '?'.
// Returns empty string when `to` has no encoder (replacement, x_user_defined).
std::string transcode_string(std::span<const char> src, codec from, codec to);

// Label-based overload: looks up WHATWG labels for `from_label` and `to_label`.
// Returns nullopt if either label is unknown.
std::optional<std::string>
transcode_string(std::span<const char> src, std::string_view from_label, std::string_view to_label);

// ---------------------------------------------------------------------------
// detail helpers
// ---------------------------------------------------------------------------

namespace detail {

template <codec C>
std::u32string transcode_decode_all(std::span<const char> src);

template <codec C>
std::string transcode_encode_all(std::u32string_view src);

} // namespace detail

// ---------------------------------------------------------------------------
// Out-of-line definitions: detail helpers
// ---------------------------------------------------------------------------

namespace detail {

template <codec C>
std::u32string transcode_decode_all(std::span<const char> src) {
    std::u32string result;
    for (char32_t cp : src | whatwg_decode<C>)
        result.push_back(cp);
    return result;
}

template <codec C>
std::string transcode_encode_all(std::u32string_view src) {
    std::string result;
    for (char b : src | whatwg_encode<C>)
        result.push_back(b);
    return result;
}

} // namespace detail

// ---------------------------------------------------------------------------
// Out-of-line definition: transcode_string
// ---------------------------------------------------------------------------

inline std::string transcode_string(std::span<const char> src, codec from, codec to) {
    std::u32string intermediate;
    switch (from) {
    case codec::utf_8:
        intermediate = detail::transcode_decode_all<codec::utf_8>(src);
        break;
    case codec::replacement:
        intermediate = detail::transcode_decode_all<codec::replacement>(src);
        break;
    case codec::x_user_defined:
        intermediate = detail::transcode_decode_all<codec::x_user_defined>(src);
        break;
    case codec::ibm866:
        intermediate = detail::transcode_decode_all<codec::ibm866>(src);
        break;
    case codec::iso_8859_2:
        intermediate = detail::transcode_decode_all<codec::iso_8859_2>(src);
        break;
    case codec::iso_8859_3:
        intermediate = detail::transcode_decode_all<codec::iso_8859_3>(src);
        break;
    case codec::iso_8859_4:
        intermediate = detail::transcode_decode_all<codec::iso_8859_4>(src);
        break;
    case codec::iso_8859_5:
        intermediate = detail::transcode_decode_all<codec::iso_8859_5>(src);
        break;
    case codec::iso_8859_6:
        intermediate = detail::transcode_decode_all<codec::iso_8859_6>(src);
        break;
    case codec::iso_8859_7:
        intermediate = detail::transcode_decode_all<codec::iso_8859_7>(src);
        break;
    case codec::iso_8859_8:
        intermediate = detail::transcode_decode_all<codec::iso_8859_8>(src);
        break;
    case codec::iso_8859_8_i:
        intermediate = detail::transcode_decode_all<codec::iso_8859_8_i>(src);
        break;
    case codec::iso_8859_10:
        intermediate = detail::transcode_decode_all<codec::iso_8859_10>(src);
        break;
    case codec::iso_8859_13:
        intermediate = detail::transcode_decode_all<codec::iso_8859_13>(src);
        break;
    case codec::iso_8859_14:
        intermediate = detail::transcode_decode_all<codec::iso_8859_14>(src);
        break;
    case codec::iso_8859_15:
        intermediate = detail::transcode_decode_all<codec::iso_8859_15>(src);
        break;
    case codec::iso_8859_16:
        intermediate = detail::transcode_decode_all<codec::iso_8859_16>(src);
        break;
    case codec::koi8_r:
        intermediate = detail::transcode_decode_all<codec::koi8_r>(src);
        break;
    case codec::koi8_u:
        intermediate = detail::transcode_decode_all<codec::koi8_u>(src);
        break;
    case codec::macintosh:
        intermediate = detail::transcode_decode_all<codec::macintosh>(src);
        break;
    case codec::windows_874:
        intermediate = detail::transcode_decode_all<codec::windows_874>(src);
        break;
    case codec::windows_1250:
        intermediate = detail::transcode_decode_all<codec::windows_1250>(src);
        break;
    case codec::windows_1251:
        intermediate = detail::transcode_decode_all<codec::windows_1251>(src);
        break;
    case codec::windows_1252:
        intermediate = detail::transcode_decode_all<codec::windows_1252>(src);
        break;
    case codec::windows_1253:
        intermediate = detail::transcode_decode_all<codec::windows_1253>(src);
        break;
    case codec::windows_1254:
        intermediate = detail::transcode_decode_all<codec::windows_1254>(src);
        break;
    case codec::windows_1255:
        intermediate = detail::transcode_decode_all<codec::windows_1255>(src);
        break;
    case codec::windows_1256:
        intermediate = detail::transcode_decode_all<codec::windows_1256>(src);
        break;
    case codec::windows_1257:
        intermediate = detail::transcode_decode_all<codec::windows_1257>(src);
        break;
    case codec::windows_1258:
        intermediate = detail::transcode_decode_all<codec::windows_1258>(src);
        break;
    case codec::x_mac_cyrillic:
        intermediate = detail::transcode_decode_all<codec::x_mac_cyrillic>(src);
        break;
    case codec::utf_16be:
        intermediate = detail::transcode_decode_all<codec::utf_16be>(src);
        break;
    case codec::utf_16le:
        intermediate = detail::transcode_decode_all<codec::utf_16le>(src);
        break;
    case codec::gbk:
        intermediate = detail::transcode_decode_all<codec::gbk>(src);
        break;
    case codec::gb18030:
        intermediate = detail::transcode_decode_all<codec::gb18030>(src);
        break;
    case codec::big5:
        intermediate = detail::transcode_decode_all<codec::big5>(src);
        break;
    case codec::shift_jis:
        intermediate = detail::transcode_decode_all<codec::shift_jis>(src);
        break;
    case codec::euc_jp:
        intermediate = detail::transcode_decode_all<codec::euc_jp>(src);
        break;
    case codec::iso_2022_jp:
        intermediate = detail::transcode_decode_all<codec::iso_2022_jp>(src);
        break;
    case codec::euc_kr:
        intermediate = detail::transcode_decode_all<codec::euc_kr>(src);
        break;
    }

    std::string         result;
    std::u32string_view u32v = intermediate;
    switch (to) {
    case codec::utf_8:
        result = detail::transcode_encode_all<codec::utf_8>(u32v);
        break;
    case codec::ibm866:
        result = detail::transcode_encode_all<codec::ibm866>(u32v);
        break;
    case codec::iso_8859_2:
        result = detail::transcode_encode_all<codec::iso_8859_2>(u32v);
        break;
    case codec::iso_8859_3:
        result = detail::transcode_encode_all<codec::iso_8859_3>(u32v);
        break;
    case codec::iso_8859_4:
        result = detail::transcode_encode_all<codec::iso_8859_4>(u32v);
        break;
    case codec::iso_8859_5:
        result = detail::transcode_encode_all<codec::iso_8859_5>(u32v);
        break;
    case codec::iso_8859_6:
        result = detail::transcode_encode_all<codec::iso_8859_6>(u32v);
        break;
    case codec::iso_8859_7:
        result = detail::transcode_encode_all<codec::iso_8859_7>(u32v);
        break;
    case codec::iso_8859_8:
        result = detail::transcode_encode_all<codec::iso_8859_8>(u32v);
        break;
    case codec::iso_8859_8_i:
        result = detail::transcode_encode_all<codec::iso_8859_8_i>(u32v);
        break;
    case codec::iso_8859_10:
        result = detail::transcode_encode_all<codec::iso_8859_10>(u32v);
        break;
    case codec::iso_8859_13:
        result = detail::transcode_encode_all<codec::iso_8859_13>(u32v);
        break;
    case codec::iso_8859_14:
        result = detail::transcode_encode_all<codec::iso_8859_14>(u32v);
        break;
    case codec::iso_8859_15:
        result = detail::transcode_encode_all<codec::iso_8859_15>(u32v);
        break;
    case codec::iso_8859_16:
        result = detail::transcode_encode_all<codec::iso_8859_16>(u32v);
        break;
    case codec::koi8_r:
        result = detail::transcode_encode_all<codec::koi8_r>(u32v);
        break;
    case codec::koi8_u:
        result = detail::transcode_encode_all<codec::koi8_u>(u32v);
        break;
    case codec::macintosh:
        result = detail::transcode_encode_all<codec::macintosh>(u32v);
        break;
    case codec::windows_874:
        result = detail::transcode_encode_all<codec::windows_874>(u32v);
        break;
    case codec::windows_1250:
        result = detail::transcode_encode_all<codec::windows_1250>(u32v);
        break;
    case codec::windows_1251:
        result = detail::transcode_encode_all<codec::windows_1251>(u32v);
        break;
    case codec::windows_1252:
        result = detail::transcode_encode_all<codec::windows_1252>(u32v);
        break;
    case codec::windows_1253:
        result = detail::transcode_encode_all<codec::windows_1253>(u32v);
        break;
    case codec::windows_1254:
        result = detail::transcode_encode_all<codec::windows_1254>(u32v);
        break;
    case codec::windows_1255:
        result = detail::transcode_encode_all<codec::windows_1255>(u32v);
        break;
    case codec::windows_1256:
        result = detail::transcode_encode_all<codec::windows_1256>(u32v);
        break;
    case codec::windows_1257:
        result = detail::transcode_encode_all<codec::windows_1257>(u32v);
        break;
    case codec::windows_1258:
        result = detail::transcode_encode_all<codec::windows_1258>(u32v);
        break;
    case codec::x_mac_cyrillic:
        result = detail::transcode_encode_all<codec::x_mac_cyrillic>(u32v);
        break;
    case codec::utf_16be:
        result = detail::transcode_encode_all<codec::utf_16be>(u32v);
        break;
    case codec::utf_16le:
        result = detail::transcode_encode_all<codec::utf_16le>(u32v);
        break;
    case codec::gbk:
        result = detail::transcode_encode_all<codec::gbk>(u32v);
        break;
    case codec::gb18030:
        result = detail::transcode_encode_all<codec::gb18030>(u32v);
        break;
    case codec::big5:
        result = detail::transcode_encode_all<codec::big5>(u32v);
        break;
    case codec::shift_jis:
        result = detail::transcode_encode_all<codec::shift_jis>(u32v);
        break;
    case codec::euc_jp:
        result = detail::transcode_encode_all<codec::euc_jp>(u32v);
        break;
    case codec::iso_2022_jp:
        result = detail::transcode_encode_all<codec::iso_2022_jp>(u32v);
        break;
    case codec::euc_kr:
        result = detail::transcode_encode_all<codec::euc_kr>(u32v);
        break;
    default:
        break; // replacement, x_user_defined: no encoder defined
    }

    return result;
}

inline std::optional<std::string>
transcode_string(std::span<const char> src, std::string_view from_label, std::string_view to_label) {
    auto from = get_encoding(from_label);
    auto to   = get_encoding(to_label);
    if (!from || !to)
        return std::nullopt;
    return transcode_string(src, *from, *to);
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_STRING_HPP
