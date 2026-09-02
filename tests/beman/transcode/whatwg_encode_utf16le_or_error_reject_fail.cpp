// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

#include <vector>

// WHATWG does not define a UTF-16LE encoder. This must fail to compile with
// the unsupported-encoder diagnostic.
using namespace beman::transcoding;

void use_removed_utf16le_or_error_encoder() {
    std::vector<char32_t> src{U'A'};
    auto                  view = src | whatwg_encode_or_error<codec::utf_16le>;
    (void)view;
}
