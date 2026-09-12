# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# One list, both modes.
#
# Module mode used to install five files -- codec.hpp, config.hpp,
# transcode.hpp, todo.hpp and the generated config.  That is enough for a
# consumer who only `import beman.transcode;`, because transcode.hpp takes its
# `import` branch and needs nothing else.
#
# It is not enough for a consumer whose build system rebuilds the interface
# unit from the installed FILE_SET CXX_MODULES.  transcode.cppm defines
# BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT and includes transcode.hpp,
# which then takes the other branch and pulls the whole header graph -- none of
# which was installed.  Rebuilding the interface unit is a normal thing for a
# consumer to do, so the file set has to cover it.
set(BEMAN_TRANSCODE_HEADERS
    bulk_transcode.hpp
    codec.hpp
    config.hpp
    codec_concepts.hpp
    codec_result.hpp
    label.hpp
    concepts.hpp
    decode_view.hpp
    encode_view.hpp
    error.hpp
    iconv_bulk.hpp
    null_term.hpp
    sniff.hpp
    todo.hpp
    transcode.hpp
    transcode_string.hpp
    transcode_view.hpp
    detail/big5.hpp
    detail/euc_jp.hpp
    detail/euc_kr.hpp
    detail/labels.hpp
    detail/gb18030.hpp
    detail/range_traits.hpp
    detail/whatwg_decode_select.hpp
    detail/whatwg_encode_select.hpp
    detail/gbk.hpp
    detail/iso2022jp.hpp
    detail/shift_jis.hpp
    detail/single_byte.hpp
    detail/single_byte_decode_view.hpp
    detail/single_byte_encode_view.hpp
    detail/single_byte_tables.hpp
    detail/table_codec.hpp
    detail/utf8.hpp
    detail/utf8_encode.hpp
    detail/utf16.hpp
    detail/x_user_defined.hpp
    detail/tables/big5.hpp
    detail/tables/euc_jp_jis0212.hpp
    detail/tables/euc_kr.hpp
    detail/tables/shift_jis.hpp
    detail/tables/gb18030_ranges.hpp
    detail/tables/gbk.hpp
    detail/tables/ibm866.hpp
    detail/tables/iso_8859_2.hpp
    detail/tables/iso_8859_3.hpp
    detail/tables/iso_8859_4.hpp
    detail/tables/iso_8859_5.hpp
    detail/tables/iso_8859_6.hpp
    detail/tables/iso_8859_7.hpp
    detail/tables/iso_8859_8.hpp
    detail/tables/iso_8859_10.hpp
    detail/tables/iso_8859_13.hpp
    detail/tables/iso_8859_14.hpp
    detail/tables/iso_8859_15.hpp
    detail/tables/iso_8859_16.hpp
    detail/tables/koi8_r.hpp
    detail/tables/koi8_u.hpp
    detail/tables/macintosh.hpp
    detail/tables/windows_874.hpp
    detail/tables/windows_1250.hpp
    detail/tables/windows_1251.hpp
    detail/tables/windows_1252.hpp
    detail/tables/windows_1253.hpp
    detail/tables/windows_1254.hpp
    detail/tables/windows_1255.hpp
    detail/tables/windows_1256.hpp
    detail/tables/windows_1257.hpp
    detail/tables/windows_1258.hpp
    detail/tables/x_mac_cyrillic.hpp
    iconv_real.hpp
    iconv_transcode_view.hpp
    iconv_transcode_or_error_view.hpp
    whatwg_decode_view.hpp
    whatwg_encode_view.hpp
    "${PROJECT_BINARY_DIR}/include/beman/transcode/config_generated.hpp"
)

if(BEMAN_TRANSCODE_USE_MODULES)
    target_sources(
        beman.transcode
        PUBLIC
            FILE_SET CXX_MODULES FILES transcode.cppm
            FILE_SET HEADERS FILES ${BEMAN_TRANSCODE_HEADERS}
    )
else()
    target_sources(
        beman.transcode
        PUBLIC FILE_SET HEADERS FILES ${BEMAN_TRANSCODE_HEADERS}
    )
endif()
