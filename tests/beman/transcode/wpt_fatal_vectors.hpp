// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py
//
// Source: docs/wpt/textdecoder-fatal.any.js
// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding
// License: W3C 3-Clause BSD License

#ifndef TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP
#define TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP

#include <cstdint>
#include <vector>

namespace beman::transcoding::tests::wpt {

struct WptFatalVector {
    std::vector<uint8_t> input;
    const char*          description;
};

// NOLINTBEGIN(cert-err58-cpp)
inline const WptFatalVector utf8_fatal_wpt_vectors[] = {
    {{0xFF}, "invalid code"},
    {{0xC0}, "ends early"},
    {{0xE0}, "ends early 2"},
    {{0xC0, 0x00}, "invalid trail"},
    {{0xC0, 0xC0}, "invalid trail 2"},
    {{0xE0, 0x00}, "invalid trail 3"},
    {{0xE0, 0xC0}, "invalid trail 4"},
    {{0xE0, 0x80, 0x00}, "invalid trail 5"},
    {{0xE0, 0x80, 0xC0}, "invalid trail 6"},
    {{0xFC, 0x80, 0x80, 0x80, 0x80, 0x80}, "> 0x10FFFF"},
    {{0xFE, 0x80, 0x80, 0x80, 0x80, 0x80}, "obsolete lead byte"},
    {{0xC0, 0x80}, "overlong U+0000 - 2 bytes"},
    {{0xE0, 0x80, 0x80}, "overlong U+0000 - 3 bytes"},
    {{0xF0, 0x80, 0x80, 0x80}, "overlong U+0000 - 4 bytes"},
    {{0xF8, 0x80, 0x80, 0x80, 0x80}, "overlong U+0000 - 5 bytes"},
    {{0xFC, 0x80, 0x80, 0x80, 0x80, 0x80}, "overlong U+0000 - 6 bytes"},
    {{0xC1, 0xBF}, "overlong U+007F - 2 bytes"},
    {{0xE0, 0x81, 0xBF}, "overlong U+007F - 3 bytes"},
    {{0xF0, 0x80, 0x81, 0xBF}, "overlong U+007F - 4 bytes"},
    {{0xF8, 0x80, 0x80, 0x81, 0xBF}, "overlong U+007F - 5 bytes"},
    {{0xFC, 0x80, 0x80, 0x80, 0x81, 0xBF}, "overlong U+007F - 6 bytes"},
    {{0xE0, 0x9F, 0xBF}, "overlong U+07FF - 3 bytes"},
    {{0xF0, 0x80, 0x9F, 0xBF}, "overlong U+07FF - 4 bytes"},
    {{0xF8, 0x80, 0x80, 0x9F, 0xBF}, "overlong U+07FF - 5 bytes"},
    {{0xFC, 0x80, 0x80, 0x80, 0x9F, 0xBF}, "overlong U+07FF - 6 bytes"},
    {{0xF0, 0x8F, 0xBF, 0xBF}, "overlong U+FFFF - 4 bytes"},
    {{0xF8, 0x80, 0x8F, 0xBF, 0xBF}, "overlong U+FFFF - 5 bytes"},
    {{0xFC, 0x80, 0x80, 0x8F, 0xBF, 0xBF}, "overlong U+FFFF - 6 bytes"},
    {{0xF8, 0x84, 0x8F, 0xBF, 0xBF}, "overlong U+10FFFF - 5 bytes"},
    {{0xFC, 0x80, 0x84, 0x8F, 0xBF, 0xBF}, "overlong U+10FFFF - 6 bytes"},
    {{0xED, 0xA0, 0x80}, "lead surrogate"},
    {{0xED, 0xB0, 0x80}, "trail surrogate"},
    {{0xED, 0xA0, 0x80, 0xED, 0xB0, 0x80}, "surrogate pair"},
};

inline const WptFatalVector utf16le_fatal_wpt_vectors[] = {
    {{0x00}, "truncated code unit"},
};
// NOLINTEND(cert-err58-cpp)

} // namespace beman::transcoding::tests::wpt

#endif // TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP
