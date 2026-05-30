// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/textdecoder-eof.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License
// Note: only non-streaming cases are included; streaming
//       (stream:true) cases require stateful API not provided
//       by this library's range-view interface.

#ifndef TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP

#include <cstdint>
#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptEofVector {
    const char*           encoding;
    std::vector<uint8_t>  input;
    std::vector<char32_t> expected;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptEofVector wpt_eof_vectors[] = {
    {"utf-8", {0xF0}, {0xFFFD}},
    {"utf-8", {0xF0, 0x9F}, {0xFFFD}},
    {"utf-8", {0xF0, 0x9F, 0x92}, {0xFFFD}},
    {"utf-8", {0xF0, 0x9F, 0x41}, {0xFFFD, 0x0041}},
    {"utf-8", {0xF0, 0x41, 0x42}, {0xFFFD, 0x0041, 0x0042}},
    {"utf-8", {0xF0, 0x41, 0xF0}, {0xFFFD, 0x0041, 0xFFFD}},
    {"utf-8", {0xF0, 0x8F, 0x92}, {0xFFFD, 0xFFFD, 0xFFFD}},
    {"Big5", {0x81, 0x40}, {0xFFFD, 0x0040}},
    {"Big5", {0x81, 0x81}, {0xFFFD}},
    {"Big5", {0x87, 0x87, 0x40}, {0xFFFD, 0x0040}},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP
