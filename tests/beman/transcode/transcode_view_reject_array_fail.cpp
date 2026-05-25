// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: Raw arrays must not satisfy transcode closure input.
// This file must NOT compile — it is registered as a WILL_FAIL ctest target.
// The transcode_closure explicitly rejects raw array types via a constrained
// operator| overload with a static_assert, so that char[N] cannot silently
// decay to a pointer and include the null terminator.

#include <beman/transcode/detail/transcode_view.hpp>

void test() {
    char arr[] = "hello";
    // char[N] is not a valid input — should fail to compile with the message:
    // "transcode: raw arrays are not valid input to transcode"
    auto v = arr | beman::transcoding::transcode<beman::transcoding::codec::utf_8,
                                                 beman::transcoding::codec::utf_8>;
    (void)v;
}
