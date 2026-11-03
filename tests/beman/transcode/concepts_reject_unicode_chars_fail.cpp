// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: Unicode character types must not satisfy legacy_byte_range.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
// char8_t, char16_t, char32_t, and wchar_t are deliberately excluded from
// legacy_byte_range so that transcoding views cannot be constructed from ranges
// whose element type already implies an encoding.
//
// EXPECT: "required for the satisfaction of [^a-z_]+legacy_byte_range<R>.+ \[with R = std::vector<char8_t,"
// EXPECT: "'std::vector<char8_t>[^']*' does not satisfy 'legacy_byte_range'"

#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <vector>

void test() {
    std::vector<char8_t> v8{u8'A'};
    // std::vector<char8_t> is not a legacy_byte_range — should fail to compile
    auto decoded = v8 | beman::transcoding::whatwg::decode<beman::transcoding::whatwg::encoding::utf_8>;
    (void)decoded;
}
