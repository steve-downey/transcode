// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_eof_vectors.hpp>

#include <cstdint>
#include <string_view>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::wpt::wpt_eof_vectors;

TEST_CASE("WPT EOF: UTF-8 truncated sequences", "[wpt::eof]") {
    for (const auto& v : wpt_eof_vectors) {
        if (std::string_view(v.encoding) != "utf-8")
            continue;
        std::vector<char>     bytes(v.input.begin(), v.input.end());
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
            got.push_back(cp);
        INFO("encoding=" << v.encoding << " input_size=" << v.input.size());
        CHECK(got == v.expected);
    }
}

TEST_CASE("WPT EOF: Big5 truncated sequences", "[wpt::eof]") {
    for (const auto& v : wpt_eof_vectors) {
        if (std::string_view(v.encoding) != "Big5")
            continue;
        std::vector<char>     bytes(v.input.begin(), v.input.end());
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::big5>)
            got.push_back(cp);
        INFO("encoding=" << v.encoding << " input_size=" << v.input.size());
        CHECK(got == v.expected);
    }
}
