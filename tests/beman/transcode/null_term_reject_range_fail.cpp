// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: views::null_term only accepts pointers and arrays.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
// null_term is intentionally constrained to pointer/array types because it
// must scan for a null terminator — a general range has no null sentinel.
//
// EXPECT: "transcode: views::null_term requires a pointer or array type"

#include <beman/transcode/null_term.hpp>

#include <vector>

void test() {
    std::vector<char> v{'h', 'i', '\0'};
    // vector<char> is not a pointer/array — should fail to compile
    auto nt = v | beman::transcoding::views::null_term;
    (void)nt;
}
