// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>

#include <vector>

// vector<char32_t> is not a legacy_byte_range (elements are char32_t, not char/byte).
// This must fail to compile with a diagnostic about legacy_byte_range.
using namespace beman::transcoding;
using view_t = whatwg_decode_view<codec::utf_16be, std::vector<char32_t>>;
