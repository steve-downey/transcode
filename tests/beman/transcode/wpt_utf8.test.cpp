// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_utf8_vectors.hpp>

#include <cstdint>
#include <vector>

using namespace beman::transcoding;

namespace {

std::vector<char32_t> decode_utf8(const std::vector<uint8_t>& input) {
    std::vector<char>     bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
        result.push_back(cp);
    return result;
}

} // namespace

TEST_CASE("WPT UTF-8 decode: invalid byte sequences produce U+FFFD", "[wpt::utf_8]") {
    for (const auto& v : beman::transcoding::tests::wpt::utf8_wpt_decode_vectors) {
        INFO("Case: " << v.description);
        CHECK(decode_utf8(v.input) == v.expected);
    }
}
