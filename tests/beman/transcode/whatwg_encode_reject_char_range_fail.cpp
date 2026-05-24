// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>

#include <vector>

// vector<char> is not a unicode_scalar_range (elements are char, not char32_t).
// This must fail to compile with a diagnostic about unicode_scalar_range.
using namespace beman::transcoding;
using view_t = whatwg_encode_view<codec::utf_8, std::vector<char>>;
