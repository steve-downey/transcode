// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP

#include <beman/transcode/config.hpp>

#if BEMAN_TRANSCODE_USE_MODULES() && !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

import beman.transcode;

#else

    #include <beman/transcode/codec.hpp>
    #include <beman/transcode/null_term.hpp>
    #include <beman/transcode/whatwg_decode_view.hpp>
    #include <beman/transcode/whatwg_encode_view.hpp>
    #include <beman/transcode/detail/single_byte_decode_view.hpp>
    #include <beman/transcode/detail/single_byte_encode_view.hpp>
    #include <beman/transcode/detail/bulk_transcode.hpp>
    #include <beman/transcode/iconv_transcode_view.hpp>
    #include <beman/transcode/iconv_transcode_or_error_view.hpp>
    #include <beman/transcode/iconv_real.hpp>
    #include <beman/transcode/iconv_bulk.hpp>
    #include <beman/transcode/detail/labels.hpp>
    #include <beman/transcode/sniff.hpp>
    #include <beman/transcode/transcode_string.hpp>
    #include <beman/transcode/transcode_view.hpp>
    #include <beman/transcode/codec_concepts.hpp>
    #include <beman/transcode/detail/table_codec.hpp>
    #include <beman/transcode/decode_view.hpp>
    #include <beman/transcode/encode_view.hpp>

#endif // BEMAN_TRANSCODE_USE_MODULES() &&
       // !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
