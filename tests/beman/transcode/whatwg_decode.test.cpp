// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>

#include <ranges>
#include <vector>

namespace {
// Helper: collect all code points from a whatwg_decode view into a vector.
template <typename View>
std::vector<char32_t> collect(View&& v) {
    std::vector<char32_t> result;
    for (char32_t cp : v)
        result.push_back(cp);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_decode_view satisfies input_range", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    auto              view = bytes | whatwg_decode<codec::utf_8>;
    static_assert(std::ranges::input_range<decltype(view)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(view)>, char32_t>);
}

// All WHATWG codecs share the 7-bit ASCII base (U+0000–U+007F).
TEST_CASE("whatwg_decode 7-bit ASCII shared base", "[transcoding::whatwg_decode]") {
    std::vector<char> ascii{'H', 'e', 'l', 'l', 'o'};
    CHECK(collect(ascii | whatwg_decode<codec::utf_8>) ==
          std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
}

// Step 5: valid multi-byte UTF-8 sequences

TEST_CASE("whatwg_decode 2-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+00E9 é = 0xC3 0xA9
    std::vector<char> bytes{'\xC3', '\xA9'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'é'});
}

TEST_CASE("whatwg_decode 3-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+20AC € = 0xE2 0x82 0xAC
    std::vector<char> bytes{'\xE2', '\x82', '\xAC'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'€'});
}

TEST_CASE("whatwg_decode 4-byte UTF-8", "[transcoding::whatwg_decode]") {
    // U+1F600 😀 = 0xF0 0x9F 0x98 0x80
    std::vector<char> bytes{'\xF0', '\x9F', '\x98', '\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) == std::vector<char32_t>{U'\U0001F600'});
}

TEST_CASE("whatwg_decode mixed ASCII and multi-byte", "[transcoding::whatwg_decode]") {
    // "Hi" + é (0xC3 0xA9) + "!"
    std::vector<char> bytes{'H', 'i', '\xC3', '\xA9', '!'};
    CHECK(collect(bytes | whatwg_decode<codec::utf_8>) ==
          std::vector<char32_t>{U'H', U'i', U'é', U'!'});
}
