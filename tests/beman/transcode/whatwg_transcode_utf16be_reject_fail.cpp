// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/transcode_view.hpp>
#include <beman/transcode/whatwg/transcode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

// WHATWG does not define a UTF-16BE encoder, so it cannot be the target of
// whatwg::transcode. This must fail to compile with the unsupported-encoder
// diagnostic.
using namespace beman::transcoding;

void use_utf16be_target() {
    auto adaptor = whatwg::transcode<whatwg::encoding::utf_8, whatwg::encoding::utf_16be>;
    (void)adaptor;
}
