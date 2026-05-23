// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>

#include <ranges>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("whatwg_decode_view satisfies input_range", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    auto              view = bytes | whatwg_decode<codec::utf_8>;
    static_assert(std::ranges::input_range<decltype(view)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(view)>, char32_t>);
}

TEST_CASE("whatwg_decode ASCII passthrough", "[transcoding::whatwg_decode]") {
    std::vector<char>     ascii{'H', 'e', 'l', 'l', 'o'};
    auto                  decoded = ascii | whatwg_decode<codec::utf_8>;
    std::vector<char32_t> result;
    for (char32_t cp : decoded)
        result.push_back(cp);
    CHECK(result == std::vector<char32_t>{U'H', U'e', U'l', U'l', U'o'});
}
