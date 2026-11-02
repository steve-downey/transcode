// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::decoder<C>::flush exists only for an encoding
// that carries decode state; windows-1252 does not.
// This file must NOT compile. The EXPECT lines below name the diagnostic it must fail with.
//
// EXPECT: "no_decode_state.+ \[with C = .+encoding::windows_1252\].+ evaluated to .+false"
// EXPECT: "'!std::same_as<detail::decode_state_t<.+whatwg::encoding.+>, detail::no_decode_state>' evaluated to false"

#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    beman::transcoding::whatwg::decoder<beman::transcoding::whatwg::encoding::windows_1252> d{};
    (void)d.flush();
}
