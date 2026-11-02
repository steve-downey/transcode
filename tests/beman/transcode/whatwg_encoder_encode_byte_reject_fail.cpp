// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::encoder<C>::encode_byte exists only for an
// encoding that encodes one scalar value to one byte; Shift_JIS does not.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
//
// EXPECT: "required for the satisfaction of .+random_access_encode_codec<C>.+ \[with C = .+encoding::shift_jis\]"
// EXPECT: "encoding\)[0-9]+'? does not satisfy 'random_access_encode_codec'"

#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    constexpr beman::transcoding::whatwg::encoder<beman::transcoding::whatwg::encoding::shift_jis> e{};
    (void)e.encode_byte(U'a');
}
