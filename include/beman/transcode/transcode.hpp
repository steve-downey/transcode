// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP

#include <beman/transcode/config.hpp>

#if BEMAN_TRANSCODE_USE_MODULES() && !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

import beman.transcode;

#else

    #include <beman/transcode/detail/null_term.hpp>
    #include <beman/transcode/whatwg_decode_view.hpp>
    #include <beman/transcode/whatwg_encode_view.hpp>
    #include <beman/transcode/detail/labels.hpp>
    #include <beman/transcode/detail/sniff.hpp>
    #include <beman/transcode/detail/transcode_string.hpp>
    #include <beman/transcode/detail/transcode_view.hpp>

#endif // BEMAN_TRANSCODE_USE_MODULES() &&
       // !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
