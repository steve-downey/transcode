// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP

#include <beman/transcode/config.hpp>

#if BEMAN_TRANSCODE_USE_MODULES() && !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

import beman.transcode;

#else

    #include <beman/transcode/todo.hpp>

#endif // BEMAN_TRANSCODE_USE_MODULES() &&
       // !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
