// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_iso2022jp_vectors.hpp>

#include <cstdint>
#include <vector>

using namespace beman::transcoding;

namespace {

std::vector<char32_t> decode_iso2022jp(const std::vector<uint8_t>& input) {
    std::vector<char> bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<codec::iso_2022_jp>)
        result.push_back(cp);
    return result;
}

} // namespace

TEST_CASE("WPT ISO-2022-JP decode vectors", "[wpt::iso_2022_jp]") {
    for (const auto& v : beman::transcoding::tests::wpt::iso2022jp_wpt_decode_vectors) {
        INFO("Case: " << v.description);
        CHECK(decode_iso2022jp(v.input) == v.expected);
    }
}
