// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/null_term.hpp>
#include <beman/transcode/detail/concepts.hpp>
#include <catch2/catch_all.hpp>

#include <ranges>
#include <string>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("null_term over string literal", "[transcoding::null_term]") {
    auto              v = "hello" | views::null_term;
    std::vector<char> result;
    for (char c : v)
        result.push_back(c);
    CHECK(result == std::vector<char>{'h', 'e', 'l', 'l', 'o'});
}

TEST_CASE("null_term over char pointer", "[transcoding::null_term]") {
    const char*       p = "world";
    auto              v = p | views::null_term;
    std::vector<char> result;
    for (char c : v)
        result.push_back(c);
    CHECK(result == std::vector<char>{'w', 'o', 'r', 'l', 'd'});
}

TEST_CASE("null_term over empty string", "[transcoding::null_term]") {
    auto v = "" | views::null_term;
    CHECK(std::ranges::begin(v) == std::ranges::end(v));
}

TEST_CASE("null_term result satisfies legacy_byte_range", "[transcoding::null_term]") {
    auto v = "test" | views::null_term;
    static_assert(legacy_byte_range<decltype(v)>);
}

TEST_CASE("null_term is a view", "[transcoding::null_term]") {
    auto v = "abc" | views::null_term;
    static_assert(std::ranges::view<decltype(v)>);
    static_assert(std::ranges::input_range<decltype(v)>);
}
