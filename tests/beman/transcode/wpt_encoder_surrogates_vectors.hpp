// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/textencoder-utf16-surrogates.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License

#ifndef TESTS_BEMAN_TRANSCODE_WPT_ENCODER_SURROGATES_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_ENCODER_SURROGATES_VECTORS_HPP

#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptEncoderSurrogatesVector {
    std::vector<char32_t> input;
    std::vector<char32_t> expected;
    const char*           name;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptEncoderSurrogatesVector wpt_encoder_surrogates_vectors[] = {
    {{0xD800}, {0xFFFD}, "lone surrogate lead"},
    {{0xDC00}, {0xFFFD}, "lone surrogate trail"},
    {{0xD800, 0x0000}, {0xFFFD, 0x0000}, "unmatched surrogate lead"},
    {{0xDC00, 0x0000}, {0xFFFD, 0x0000}, "unmatched surrogate trail"},
    {{0xDC00, 0xD800}, {0xFFFD, 0xFFFD}, "swapped surrogate pair"},
    {{0x1D11E}, {0x1D11E}, "properly encoded MUSICAL SYMBOL G CLEF (U+1D11E)"},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_ENCODER_SURROGATES_VECTORS_HPP
