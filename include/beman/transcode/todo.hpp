// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TRANSCODE_TODO_HPP
#define BEMAN_TRANSCODE_TODO_HPP

#include <beman/transcode/config.hpp>

#if BEMAN_TRANSCODE_USE_MODULES() && !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

import beman.transcode;

#else

namespace beman::transcoding {

// TODO

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_USE_MODULES() &&
       // !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

#endif // BEMAN_TRANSCODE_TODO_HPP
