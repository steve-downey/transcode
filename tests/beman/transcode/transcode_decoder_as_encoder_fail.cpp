// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// EXPECT: "required for the satisfaction of [^a-z_]+text_encoder<To>.+ \[with To = .+whatwg::decoder<"
// EXPECT: "'[^']*whatwg::decoder<[^']*' does not satisfy 'text_encoder'"

#include <beman/transcode/transcode_view.hpp>
#include <beman/transcode/transcode_view.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

// transcode(from, to) wants a text_decoder first and a text_encoder second.
// whatwg::decoder<...> is not an encoder, so this must fail to compile with
// the unsatisfied text_encoder constraint.
using namespace beman::transcoding;

void use_decoder_as_encoder() {
    auto adaptor = transcode(whatwg::decoder<whatwg::encoding::shift_jis>{}, whatwg::decoder<whatwg::encoding::gbk>{});
    (void)adaptor;
}
