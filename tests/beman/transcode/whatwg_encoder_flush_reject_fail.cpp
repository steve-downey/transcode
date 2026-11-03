// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::encoder<C>::flush exists only for an encoding
// that owes output at the end of the input; Shift_JIS does not.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
//
// EXPECT: "the expression .+C == .+encoding::iso_2022_jp \[with C = .+encoding::shift_jis\].+ evaluated to .+false"
// EXPECT: "'[^']*whatwg::encoding[^']* == encoding::iso_2022_jp' evaluated to false"

#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    beman::transcoding::whatwg::encoder<beman::transcoding::whatwg::encoding::shift_jis> e{};
    (void)e.flush();
}
