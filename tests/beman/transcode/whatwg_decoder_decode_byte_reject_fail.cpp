// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::decoder<C>::decode_byte exists only for an
// encoding that decodes one byte to one scalar value; Shift_JIS does not.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    constexpr beman::transcoding::whatwg::decoder<beman::transcoding::whatwg::encoding::shift_jis> d{};
    (void)d.decode_byte(0x41);
}
