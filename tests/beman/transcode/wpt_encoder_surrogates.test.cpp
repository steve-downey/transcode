// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_encoder_surrogates_vectors.hpp>

#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::wpt::wpt_encoder_surrogates_vectors;

TEST_CASE("WPT encoder surrogates: UTF-8 round-trip handles surrogates", "[wpt::encoder_surrogates]") {
    for (const auto& v : wpt_encoder_surrogates_vectors) {
        std::vector<char> bytes;
        for (char c : v.input | whatwg_encode<codec::utf_8>)
            bytes.push_back(c);
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
            got.push_back(cp);
        INFO("name=" << v.name);
        CHECK(got == v.expected);
    }
}
