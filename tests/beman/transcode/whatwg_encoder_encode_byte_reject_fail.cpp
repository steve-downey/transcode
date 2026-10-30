// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::encoder<C>::encode_byte exists only for an
// encoding that encodes one scalar value to one byte; Shift_JIS does not.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    constexpr beman::transcoding::whatwg::encoder<beman::transcoding::whatwg::encoding::shift_jis> e{};
    (void)e.encode_byte(U'a');
}
