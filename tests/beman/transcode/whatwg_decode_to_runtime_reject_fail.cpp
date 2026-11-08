// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::decode_to with an encoding chosen at run time
// takes a legacy_byte_range; a range of char32_t is not one.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
//
// EXPECT: "required for the satisfaction of [^a-z_]+legacy_byte_range<R>.+ \[with R = std::vector<char32_t"
// EXPECT: "'std::vector<char32_t>[^']*' does not satisfy 'legacy_byte_range'"

#include <beman/transcode/whatwg/bulk.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <vector>

void test() {
    std::vector<char32_t> scalars{U'a'};
    (void)beman::transcoding::whatwg::decode_to(scalars, beman::transcoding::whatwg::encoding::utf_8);
}
