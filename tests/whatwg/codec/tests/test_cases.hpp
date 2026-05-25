#include <cstdint>
#include <span>
#include <string_view>
#include <array>
#include <ranges>
#include <algorithm>

namespace whatwg::codec::tests {

/**
 * PROVENANCE AND LICENSE:
 * These test vectors are derived from the Web Platform Tests (WPT) project.
 * Repository: https://github.com/web-platform-tests/wpt/tree/master/encoding
 * License: W3C 3-Clause BSD License
 * Copyright (c) W3C and Web Platform Tests contributors.
 */

// Represents a compile-time test case for decoding a byte stream
struct DecodeTestCase {
    std::string_view         encoding_label;
    std::span<const uint8_t> input_bytes;
    std::u32string_view      expected_code_points;
    bool                     fatal_flag;
};

// Represents a compile-time test case for encoding Unicode code points
struct EncodeTestCase {
    std::string_view         encoding_label;
    std::u32string_view      input_code_points;
    std::span<const uint8_t> expected_bytes;
    bool                     fatal_flag;
};

} // namespace whatwg::codec::tests
