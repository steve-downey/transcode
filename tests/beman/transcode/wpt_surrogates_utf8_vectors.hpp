// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/api-surrogates-utf8.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License

#ifndef TESTS_BEMAN_TRANSCODE_WPT_SURROGATES_UTF8_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_SURROGATES_UTF8_VECTORS_HPP

#include <cstdint>
#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptSurrogatesUtf8Vector {
    std::vector<char32_t> input;
    std::vector<uint8_t>  expected;
    std::vector<char32_t> decoded;
    const char*           name;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptSurrogatesUtf8Vector wpt_surrogates_utf8_vectors[] = {
    {{0x0061, 0x0062, 0x0063, 0x0031, 0x0032, 0x0033},
     {0x61, 0x62, 0x63, 0x31, 0x32, 0x33},
     {0x0061, 0x0062, 0x0063, 0x0031, 0x0032, 0x0033},
     "Sanity check"},
    {{0xD800}, {0xEF, 0xBF, 0xBD}, {0xFFFD}, "Surrogate half (low)"},
    {{0xDC00}, {0xEF, 0xBF, 0xBD}, {0xFFFD}, "Surrogate half (high)"},
    {{0x0061, 0x0062, 0x0063, 0xD800, 0x0031, 0x0032, 0x0033},
     {0x61, 0x62, 0x63, 0xEF, 0xBF, 0xBD, 0x31, 0x32, 0x33},
     {0x0061, 0x0062, 0x0063, 0xFFFD, 0x0031, 0x0032, 0x0033},
     "Surrogate half (low), in a string"},
    {{0x0061, 0x0062, 0x0063, 0xDC00, 0x0031, 0x0032, 0x0033},
     {0x61, 0x62, 0x63, 0xEF, 0xBF, 0xBD, 0x31, 0x32, 0x33},
     {0x0061, 0x0062, 0x0063, 0xFFFD, 0x0031, 0x0032, 0x0033},
     "Surrogate half (high), in a string"},
    {{0xDC00, 0xD800}, {0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD}, {0xFFFD, 0xFFFD}, "Wrong order"},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_SURROGATES_UTF8_VECTORS_HPP
