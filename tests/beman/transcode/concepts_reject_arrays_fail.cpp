// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: Arrays must not satisfy legacy_byte_range.
// This file must NOT compile — it is registered as a WILL_FAIL ctest target.
// The legacy_byte_range concept explicitly rejects array types via
// !std::is_array_v so that raw string literals and char arrays cannot
// silently decay to pointers and include the null terminator.

#include <beman/transcode/whatwg_decode_view.hpp>

void test() {
    char arr[5] = "test";
    // char[N] is not a legacy_byte_range — should fail to compile
    auto v = arr | beman::transcoding::whatwg_decode<beman::transcoding::codec::utf_8>;
    (void)v;
}
