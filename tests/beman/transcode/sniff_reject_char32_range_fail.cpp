// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::bom_sniff requires a legacy_byte_range.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
// vector<char32_t> is not a legacy_byte_range (element type is char32_t,
// not char/signed char/unsigned char/std::byte), so whatwg::bom_sniff must
// reject it with a diagnostic mentioning legacy_byte_range.
//
// EXPECT: "required for the satisfaction of [^a-z_]+legacy_byte_range<R>.+ \[with R = std::vector<char32_t,"
// EXPECT: "'std::vector<char32_t>[^']*' does not satisfy 'legacy_byte_range'"

#include <beman/transcode/whatwg/bom_sniff.hpp>

#include <vector>

void test() {
    std::vector<char32_t> v = {0xEF, 0xBB, 0xBF};
    // char32_t is not a legacy byte type — must fail to compile
    auto result = beman::transcoding::whatwg::bom_sniff(v);
    (void)result;
}
