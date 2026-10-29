// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::decoder<C>::flush exists only for an encoding
// that carries decode state; windows-1252 does not.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    beman::transcoding::whatwg::decoder<beman::transcoding::whatwg::encoding::windows_1252> d{};
    (void)d.flush();
}
