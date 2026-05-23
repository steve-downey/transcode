// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_transcode_view.hpp>
#include <catch2/catch_all.hpp>

#include <concepts>
#include <ranges>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("iconv_transcode_view satisfies input_range", "[transcoding::iconv_transcode]") {
    using view_t = iconv_transcode_view<iconv_functions, std::vector<char>>;
    static_assert(std::ranges::input_range<view_t>);
    static_assert(std::same_as<std::ranges::range_value_t<view_t>, char>);

    using iter_t = std::ranges::iterator_t<view_t>;
    static_assert(!std::copy_constructible<iter_t>);
    static_assert(std::movable<iter_t>);
}
