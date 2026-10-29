// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: whatwg::encoder<C> exists only for an encoding the
// Encoding Standard defines an encoder for; UTF-16BE has none.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

void test() {
    constexpr beman::transcoding::whatwg::encoder<beman::transcoding::whatwg::encoding::utf_16be> e{};
    (void)e;
}
