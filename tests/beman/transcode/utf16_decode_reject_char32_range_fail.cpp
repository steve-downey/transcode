// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// EXPECT: "required for the satisfaction of [^a-z_]+legacy_byte_range<R>.+ \[with R = std::vector<char32_t,"
// EXPECT: "'std::vector<char32_t>[^']*' does not satisfy 'legacy_byte_range'"

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <vector>

// vector<char32_t> is not a legacy_byte_range (elements are char32_t, not char/byte).
// This must fail to compile with a diagnostic about legacy_byte_range.
using namespace beman::transcoding;
using view_t = decode_view<whatwg::decoder<whatwg::encoding::utf_16be>, std::vector<char32_t>>;
