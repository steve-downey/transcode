// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/null_term.hpp>
#include <beman/transcode/detail/concepts.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <string>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::constify;

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

constexpr bool null_term_pointer_constexpr() {
    const char* p = "hi";
    auto        v = p | views::null_term;
    int         n = 0;
    for ([[maybe_unused]] char c : v)
        ++n;
    return n == 2;
}

constexpr bool null_sentinel_compare_constexpr() {
    const char*     p = "abc";
    null_sentinel_t s = {};
    return !(p == s); // 'a' != 0, so not at sentinel
}

TEST_CASE("null_term is usable in consteval context", "[transcoding::null_term]") {
    CHECK(constify(null_term_pointer_constexpr()));
}

TEST_CASE("null_sentinel comparison is constexpr", "[transcoding::null_term]") {
    CHECK(constify(null_sentinel_compare_constexpr()));
}
