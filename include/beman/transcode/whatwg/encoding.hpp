// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODING_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODING_HPP

#include <beman/transcode/config.hpp>

namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.encoding}, encodings

//! \remarks Each enumerator denotes the encoding that the WHATWG Encoding
//! Standard names correspondingly.
enum class encoding {
    utf_8,
    replacement,
    x_user_defined,
    ibm866,
    iso_8859_2,
    iso_8859_3,
    iso_8859_4,
    iso_8859_5,
    iso_8859_6,
    iso_8859_7,
    iso_8859_8,
    iso_8859_8_i,
    iso_8859_10,
    iso_8859_13,
    iso_8859_14,
    iso_8859_15,
    iso_8859_16,
    koi8_r,
    koi8_u,
    macintosh,
    windows_874,
    windows_1250,
    windows_1251,
    windows_1252,
    windows_1253,
    windows_1254,
    windows_1255,
    windows_1256,
    windows_1257,
    windows_1258,
    x_mac_cyrillic,
    utf_16be,
    utf_16le,
    gbk,
    gb18030,
    big5,
    shift_jis,
    euc_jp,
    iso_2022_jp,
    euc_kr,
};

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODING_HPP
