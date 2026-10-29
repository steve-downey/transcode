// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::decoder<C>::decode_first exists only for an
// encoding that strips a leading byte order mark; Shift_JIS does not.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    beman::transcoding::whatwg::decoder<beman::transcoding::whatwg::encoding::shift_jis> d{};
    const unsigned char                                                                  bytes[] = {0x41};
    const unsigned char*                                                                 it      = bytes;
    (void)d.decode_first(it, bytes + 1);
}
