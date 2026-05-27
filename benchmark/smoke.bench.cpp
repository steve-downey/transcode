// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <catch2/catch_all.hpp>

#include <ranges>
#include <span>
#include <string>
#include <vector>

namespace {

const std::string ascii_4k(4096, 'A');

} // namespace

TEST_CASE("smoke benchmark — UTF-8 decode ASCII 4 KiB", "[smoke][benchmark]") {
    using namespace beman::transcoding;

    BENCHMARK("whatwg_decode utf_8 4096-byte ASCII") {
        std::span<const char> input(ascii_4k);
        std::vector<char32_t> output;
        output.reserve(4096);
        for (char32_t cp : input | whatwg_decode<codec::utf_8>) {
            output.push_back(cp);
        }
        return output.size();
    };
}
