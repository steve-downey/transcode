// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/textdecoder-utf16-surrogates.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License

#ifndef TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP

#include <cstdint>
#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptDecodeVector {
    std::vector<uint8_t>  input;
    std::vector<char32_t> expected;
    const char*           description;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptDecodeVector utf16le_surrogate_wpt_decode_vectors[] = {
    {{0x00, 0xD8}, {0xFFFD}, "lone surrogate lead"},
    {{0x00, 0xDC}, {0xFFFD}, "lone surrogate trail"},
    {{0x00, 0xD8, 0x00, 0x00}, {0xFFFD, 0x0000}, "unmatched surrogate lead"},
    {{0x00, 0xDC, 0x00, 0x00}, {0xFFFD, 0x0000}, "unmatched surrogate trail"},
    {{0x00, 0xDC, 0x00, 0xD8}, {0xFFFD, 0xFFFD}, "swapped surrogate pair"},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP
