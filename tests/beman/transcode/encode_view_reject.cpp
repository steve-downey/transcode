// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Negative compile test: a type that does NOT satisfy encode_codec should fail
// to compile when passed to encode().  The compiler diagnostic must mention
// encode_codec (via a concept constraint failure).

#include <beman/transcode/encode_view.hpp>

#include <vector>

using namespace beman::transcoding;

struct not_an_encode_codec {
    // Missing encode_one — does not satisfy encode_codec
    int value;
};

void test() {
    std::vector<char32_t> src{U'A'};
    auto                  view = src | encode(not_an_encode_codec{});
    (void)view;
}
