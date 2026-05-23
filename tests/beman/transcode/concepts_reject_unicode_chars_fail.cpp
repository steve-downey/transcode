// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: Unicode character types must not satisfy legacy_byte_range.
// This file must NOT compile — it is registered as a WILL_FAIL ctest target.
// char8_t, char16_t, char32_t, and wchar_t are deliberately excluded from
// legacy_byte_range so that transcoding views cannot be constructed from ranges
// whose element type already implies an encoding.

#include <beman/transcode/whatwg_decode_view.hpp>

#include <vector>

void test() {
    std::vector<char8_t> v8{u8'A'};
    // std::vector<char8_t> is not a legacy_byte_range — should fail to compile
    auto decoded = v8 | beman::transcoding::whatwg_decode<beman::transcoding::codec::utf_8>;
    (void)decoded;
}
