// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/textdecoder-byte-order-marks.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License

#ifndef TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP

#include <cstdint>
#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptBomCase {
    std::vector<uint8_t>  bytes;
    std::vector<uint8_t>  bom;
    std::vector<char32_t> expected;
    const char*           encoding;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptBomCase wpt_bom_cases[] = {
    {{0x7A, 0xC2, 0xA2, 0xE6, 0xB0, 0xB4, 0xF0, 0x9D, 0x84, 0x9E, 0xF4, 0x8F, 0xBF, 0xBD},
     {0xEF, 0xBB, 0xBF},
     {0x007A, 0x00A2, 0x6C34, 0x1D11E, 0x10FFFD},
     "utf-8"},
    {{0x7A, 0x00, 0xA2, 0x00, 0x34, 0x6C, 0x34, 0xD8, 0x1E, 0xDD, 0xFF, 0xDB, 0xFD, 0xDF},
     {0xFF, 0xFE},
     {0x007A, 0x00A2, 0x6C34, 0x1D11E, 0x10FFFD},
     "utf-16le"},
    {{0x00, 0x7A, 0x00, 0xA2, 0x6C, 0x34, 0xD8, 0x34, 0xDD, 0x1E, 0xDB, 0xFF, 0xDF, 0xFD},
     {0xFE, 0xFF},
     {0x007A, 0x00A2, 0x6C34, 0x1D11E, 0x10FFFD},
     "utf-16be"},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP
