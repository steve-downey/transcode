// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_gb18030_vectors.hpp>

#include <cstdint>
#include <vector>

using namespace beman::transcoding;

namespace {

template <codec C>
std::vector<char32_t> decode_bytes(const std::vector<uint8_t>& input) {
    std::vector<char> bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<C>)
        result.push_back(cp);
    return result;
}

} // namespace

TEST_CASE("WPT GB18030/GBK decode vectors: gbk codec", "[wpt::gbk]") {
    for (const auto& v : beman::transcoding::tests::wpt::gb18030_wpt_decode_vectors) {
        INFO("Case: " << v.description);
        CHECK(decode_bytes<codec::gbk>(v.input) == v.expected);
    }
}

TEST_CASE("WPT GB18030/GBK decode vectors: gb18030 codec", "[wpt::gb18030]") {
    for (const auto& v : beman::transcoding::tests::wpt::gb18030_wpt_decode_vectors) {
        INFO("Case: " << v.description);
        CHECK(decode_bytes<codec::gb18030>(v.input) == v.expected);
    }
}
