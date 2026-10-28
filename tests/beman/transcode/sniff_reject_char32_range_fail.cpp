// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::bom_sniff requires a legacy_byte_range.
// This file must NOT compile — it is registered as a WILL_FAIL ctest target.
// vector<char32_t> is not a legacy_byte_range (element type is char32_t,
// not char/signed char/unsigned char/std::byte), so whatwg::bom_sniff must
// reject it with a diagnostic mentioning legacy_byte_range.

#include <beman/transcode/whatwg/bom_sniff.hpp>

#include <vector>

void test() {
    std::vector<char32_t> v = {0xEF, 0xBB, 0xBF};
    // char32_t is not a legacy byte type — must fail to compile
    auto result = beman::transcoding::whatwg::bom_sniff(v);
    (void)result;
}
