// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_STRING_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_STRING_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <optional>
    #include <span>
    #include <string>
    #include <string_view>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.string}, eager transcoding

constexpr bool has_encoder(whatwg::encoding encoding) noexcept;

std::optional<std::string> transcode_string(std::span<const char> src, whatwg::encoding from, whatwg::encoding to);

std::optional<std::string>
transcode_string(std::span<const char> src, std::string_view from_label, std::string_view to_label);

// ---------------------------------------------------------------------------
// detail helpers
// ---------------------------------------------------------------------------

namespace detail {

//! \omit
template <whatwg::encoding C>
std::u32string transcode_decode_all(std::span<const char> src);

//! \omit
template <whatwg::encoding C>
std::string transcode_encode_all(std::u32string_view src);

} // namespace detail

// ---------------------------------------------------------------------------
// Out-of-line definitions: detail helpers
// ---------------------------------------------------------------------------

namespace detail {

template <whatwg::encoding C>
std::u32string transcode_decode_all(std::span<const char> src) {
    std::u32string result;
    for (char32_t cp : src | decode<C>)
        result.push_back(cp);
    return result;
}

template <whatwg::encoding C>
std::string transcode_encode_all(std::u32string_view src) {
    std::string result;
    for (char b : src | encode<C>)
        result.push_back(b);
    return result;
}

} // namespace detail

// ---------------------------------------------------------------------------
// Out-of-line definitions: public API
// ---------------------------------------------------------------------------

//! \returns `true` if the WHATWG Encoding Standard defines an encoder for
//! `encoding`, and `false` otherwise.
constexpr bool has_encoder(whatwg::encoding encoding) noexcept {
    return encoding != whatwg::encoding::utf_16be && encoding != whatwg::encoding::utf_16le &&
           encoding != whatwg::encoding::replacement && encoding != whatwg::encoding::x_user_defined;
}

//! \returns `nullopt` if the WHATWG Encoding Standard defines no encoder for
//! `to`; otherwise, the bytes of `src`, decoded as `from` and re-encoded as
//! `to`.
//! Errors are substituted rather than reported: a byte sequence `from` does
//! not allow decodes to U+FFFD, and a scalar value `to` cannot represent
//! encodes to `'?'`.
//! \remarks This is `transcode` \iref{transcode.pipeline} run to completion
//! into a `string`, which is what a caller who wants the whole result and not
//! a view writes.  `src | whatwg::transcode<From, To> | ranges::to<string>()` is the
//! same thing with the codecs known at compile time.
inline std::optional<std::string>
transcode_string(std::span<const char> src, whatwg::encoding from, whatwg::encoding to) {
    if (!has_encoder(to))
        return std::nullopt;

    std::u32string intermediate;
    switch (from) {
    case whatwg::encoding::utf_8:
        intermediate = detail::transcode_decode_all<whatwg::encoding::utf_8>(src);
        break;
    case whatwg::encoding::replacement:
        intermediate = detail::transcode_decode_all<whatwg::encoding::replacement>(src);
        break;
    case whatwg::encoding::x_user_defined:
        intermediate = detail::transcode_decode_all<whatwg::encoding::x_user_defined>(src);
        break;
    case whatwg::encoding::ibm866:
        intermediate = detail::transcode_decode_all<whatwg::encoding::ibm866>(src);
        break;
    case whatwg::encoding::iso_8859_2:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_2>(src);
        break;
    case whatwg::encoding::iso_8859_3:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_3>(src);
        break;
    case whatwg::encoding::iso_8859_4:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_4>(src);
        break;
    case whatwg::encoding::iso_8859_5:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_5>(src);
        break;
    case whatwg::encoding::iso_8859_6:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_6>(src);
        break;
    case whatwg::encoding::iso_8859_7:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_7>(src);
        break;
    case whatwg::encoding::iso_8859_8:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_8>(src);
        break;
    case whatwg::encoding::iso_8859_8_i:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_8_i>(src);
        break;
    case whatwg::encoding::iso_8859_10:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_10>(src);
        break;
    case whatwg::encoding::iso_8859_13:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_13>(src);
        break;
    case whatwg::encoding::iso_8859_14:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_14>(src);
        break;
    case whatwg::encoding::iso_8859_15:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_15>(src);
        break;
    case whatwg::encoding::iso_8859_16:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_8859_16>(src);
        break;
    case whatwg::encoding::koi8_r:
        intermediate = detail::transcode_decode_all<whatwg::encoding::koi8_r>(src);
        break;
    case whatwg::encoding::koi8_u:
        intermediate = detail::transcode_decode_all<whatwg::encoding::koi8_u>(src);
        break;
    case whatwg::encoding::macintosh:
        intermediate = detail::transcode_decode_all<whatwg::encoding::macintosh>(src);
        break;
    case whatwg::encoding::windows_874:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_874>(src);
        break;
    case whatwg::encoding::windows_1250:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1250>(src);
        break;
    case whatwg::encoding::windows_1251:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1251>(src);
        break;
    case whatwg::encoding::windows_1252:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1252>(src);
        break;
    case whatwg::encoding::windows_1253:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1253>(src);
        break;
    case whatwg::encoding::windows_1254:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1254>(src);
        break;
    case whatwg::encoding::windows_1255:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1255>(src);
        break;
    case whatwg::encoding::windows_1256:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1256>(src);
        break;
    case whatwg::encoding::windows_1257:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1257>(src);
        break;
    case whatwg::encoding::windows_1258:
        intermediate = detail::transcode_decode_all<whatwg::encoding::windows_1258>(src);
        break;
    case whatwg::encoding::x_mac_cyrillic:
        intermediate = detail::transcode_decode_all<whatwg::encoding::x_mac_cyrillic>(src);
        break;
    case whatwg::encoding::utf_16be:
        intermediate = detail::transcode_decode_all<whatwg::encoding::utf_16be>(src);
        break;
    case whatwg::encoding::utf_16le:
        intermediate = detail::transcode_decode_all<whatwg::encoding::utf_16le>(src);
        break;
    case whatwg::encoding::gbk:
        intermediate = detail::transcode_decode_all<whatwg::encoding::gbk>(src);
        break;
    case whatwg::encoding::gb18030:
        intermediate = detail::transcode_decode_all<whatwg::encoding::gb18030>(src);
        break;
    case whatwg::encoding::big5:
        intermediate = detail::transcode_decode_all<whatwg::encoding::big5>(src);
        break;
    case whatwg::encoding::shift_jis:
        intermediate = detail::transcode_decode_all<whatwg::encoding::shift_jis>(src);
        break;
    case whatwg::encoding::euc_jp:
        intermediate = detail::transcode_decode_all<whatwg::encoding::euc_jp>(src);
        break;
    case whatwg::encoding::iso_2022_jp:
        intermediate = detail::transcode_decode_all<whatwg::encoding::iso_2022_jp>(src);
        break;
    case whatwg::encoding::euc_kr:
        intermediate = detail::transcode_decode_all<whatwg::encoding::euc_kr>(src);
        break;
    }

    std::string         result;
    std::u32string_view u32v = intermediate;
    switch (to) {
    case whatwg::encoding::utf_8:
        result = detail::transcode_encode_all<whatwg::encoding::utf_8>(u32v);
        break;
    case whatwg::encoding::ibm866:
        result = detail::transcode_encode_all<whatwg::encoding::ibm866>(u32v);
        break;
    case whatwg::encoding::iso_8859_2:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_2>(u32v);
        break;
    case whatwg::encoding::iso_8859_3:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_3>(u32v);
        break;
    case whatwg::encoding::iso_8859_4:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_4>(u32v);
        break;
    case whatwg::encoding::iso_8859_5:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_5>(u32v);
        break;
    case whatwg::encoding::iso_8859_6:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_6>(u32v);
        break;
    case whatwg::encoding::iso_8859_7:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_7>(u32v);
        break;
    case whatwg::encoding::iso_8859_8:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_8>(u32v);
        break;
    case whatwg::encoding::iso_8859_8_i:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_8_i>(u32v);
        break;
    case whatwg::encoding::iso_8859_10:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_10>(u32v);
        break;
    case whatwg::encoding::iso_8859_13:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_13>(u32v);
        break;
    case whatwg::encoding::iso_8859_14:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_14>(u32v);
        break;
    case whatwg::encoding::iso_8859_15:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_15>(u32v);
        break;
    case whatwg::encoding::iso_8859_16:
        result = detail::transcode_encode_all<whatwg::encoding::iso_8859_16>(u32v);
        break;
    case whatwg::encoding::koi8_r:
        result = detail::transcode_encode_all<whatwg::encoding::koi8_r>(u32v);
        break;
    case whatwg::encoding::koi8_u:
        result = detail::transcode_encode_all<whatwg::encoding::koi8_u>(u32v);
        break;
    case whatwg::encoding::macintosh:
        result = detail::transcode_encode_all<whatwg::encoding::macintosh>(u32v);
        break;
    case whatwg::encoding::windows_874:
        result = detail::transcode_encode_all<whatwg::encoding::windows_874>(u32v);
        break;
    case whatwg::encoding::windows_1250:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1250>(u32v);
        break;
    case whatwg::encoding::windows_1251:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1251>(u32v);
        break;
    case whatwg::encoding::windows_1252:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1252>(u32v);
        break;
    case whatwg::encoding::windows_1253:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1253>(u32v);
        break;
    case whatwg::encoding::windows_1254:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1254>(u32v);
        break;
    case whatwg::encoding::windows_1255:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1255>(u32v);
        break;
    case whatwg::encoding::windows_1256:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1256>(u32v);
        break;
    case whatwg::encoding::windows_1257:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1257>(u32v);
        break;
    case whatwg::encoding::windows_1258:
        result = detail::transcode_encode_all<whatwg::encoding::windows_1258>(u32v);
        break;
    case whatwg::encoding::x_mac_cyrillic:
        result = detail::transcode_encode_all<whatwg::encoding::x_mac_cyrillic>(u32v);
        break;
    case whatwg::encoding::gbk:
        result = detail::transcode_encode_all<whatwg::encoding::gbk>(u32v);
        break;
    case whatwg::encoding::gb18030:
        result = detail::transcode_encode_all<whatwg::encoding::gb18030>(u32v);
        break;
    case whatwg::encoding::big5:
        result = detail::transcode_encode_all<whatwg::encoding::big5>(u32v);
        break;
    case whatwg::encoding::shift_jis:
        result = detail::transcode_encode_all<whatwg::encoding::shift_jis>(u32v);
        break;
    case whatwg::encoding::euc_jp:
        result = detail::transcode_encode_all<whatwg::encoding::euc_jp>(u32v);
        break;
    case whatwg::encoding::iso_2022_jp:
        result = detail::transcode_encode_all<whatwg::encoding::iso_2022_jp>(u32v);
        break;
    case whatwg::encoding::euc_kr:
        result = detail::transcode_encode_all<whatwg::encoding::euc_kr>(u32v);
        break;
    // Unreachable after whatwg::has_encoder(to); retained to keep the switch exhaustive.
    case whatwg::encoding::utf_16be:
    case whatwg::encoding::utf_16le:
    case whatwg::encoding::replacement:
    case whatwg::encoding::x_user_defined:
        return std::nullopt;
    }

    return result;
}

//! \returns `whatwg::transcode_string(src, *whatwg::get_encoding(from_label),
//! *whatwg::get_encoding(to_label))`, and `nullopt` if either label names no encoding
//! \iref{transcode.whatwg.label} or the target encoding has no WHATWG encoder.
//! \remarks This is the overload a program uses when the encodings are a
//! runtime choice -- a `Content-Type` header, a command-line option -- which
//! is what labels are for.
inline std::optional<std::string>
transcode_string(std::span<const char> src, std::string_view from_label, std::string_view to_label) {
    auto from = whatwg::get_encoding(from_label);
    auto to   = whatwg::get_encoding(to_label);
    if (!from || !to)
        return std::nullopt;
    return transcode_string(src, *from, *to);
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_STRING_HPP
