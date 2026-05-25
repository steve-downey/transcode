// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_surrogates_utf8_vectors.hpp>

#include <cstdint>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::wpt::wpt_surrogates_utf8_vectors;

TEST_CASE("WPT surrogates UTF-8: encode produces correct bytes", "[wpt::surrogates_utf8]") {
    for (const auto& v : wpt_surrogates_utf8_vectors) {
        std::vector<char> got;
        for (char c : v.input | whatwg_encode<codec::utf_8>)
            got.push_back(c);
        std::vector<uint8_t> got_bytes(got.begin(), got.end());
        INFO("name=" << v.name);
        CHECK(got_bytes == v.expected);
    }
}

TEST_CASE("WPT surrogates UTF-8: decoded bytes match expected codepoints", "[wpt::surrogates_utf8]") {
    for (const auto& v : wpt_surrogates_utf8_vectors) {
        std::vector<char>     bytes(v.expected.begin(), v.expected.end());
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
            got.push_back(cp);
        INFO("name=" << v.name);
        CHECK(got == v.decoded);
    }
}
