// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/textdecoder-mistakes.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License

#ifndef TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP

#include <cstdint>
#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptDecodeVector {
    std::vector<uint8_t>  input;
    std::vector<char32_t> expected;
    const char*           description;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptDecodeVector utf8_wpt_decode_vectors[] = {
    {{0x00, 0xFE, 0xFF}, {0x0000, 0xFFFD, 0xFFFD}, "bytes [0, 254, 255]"},
    {{0x80}, {0xFFFD}, "bytes [128]"},
    {{0xF0, 0x90, 0x80}, {0xFFFD}, "bytes [240, 144, 128]"},
    {{0xF0, 0x80, 0x80}, {0xFFFD, 0xFFFD, 0xFFFD}, "bytes [240, 128, 128]"},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP
