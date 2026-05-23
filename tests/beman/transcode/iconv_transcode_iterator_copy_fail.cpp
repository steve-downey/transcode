// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Negative compile test: iconv_transcode iterator must be move-only.
// iconv_t is an OS-managed opaque handle that cannot be copied.

#include <beman/transcode/iconv_transcode_view.hpp>

#include <concepts>
#include <ranges>
#include <vector>

using namespace beman::transcoding;

using view_t = iconv_transcode_view<iconv_functions, std::vector<char>>;
using iter_t = std::ranges::iterator_t<view_t>;

// This static_assert must fail to compile: the iterator is intentionally move-only.
static_assert(std::copy_constructible<iter_t>, "transcode: iconv_transcode iterator is move-only");
