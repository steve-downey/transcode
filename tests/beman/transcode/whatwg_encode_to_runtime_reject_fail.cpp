// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::encode_to with an encoding chosen at run time
// takes a unicode_scalar_range; a range of char is not one.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
//
// EXPECT: "required for the satisfaction of [^a-z_]+unicode_scalar_range<R>.+ \[with R = std::vector<char"
// EXPECT: "'std::vector<char>[^']*' does not satisfy 'unicode_scalar_range'"

#include <beman/transcode/whatwg/bulk.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <vector>

void test() {
    std::vector<char> bytes{'a'};
    (void)beman::transcoding::whatwg::encode_to(bytes, beman::transcoding::whatwg::encoding::utf_8);
}
