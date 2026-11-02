// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::decoder<C>::decode_byte exists only for an
// encoding that decodes one byte to one scalar value; Shift_JIS does not.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
//
// EXPECT: "required for the satisfaction of .+random_access_decode_codec<C>.+ \[with C = .+encoding::shift_jis\]"
// EXPECT: "'[^']*whatwg::encoding[^']*' does not satisfy 'random_access_decode_codec'"

#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    constexpr beman::transcoding::whatwg::decoder<beman::transcoding::whatwg::encoding::shift_jis> d{};
    (void)d.decode_byte(0x41);
}
