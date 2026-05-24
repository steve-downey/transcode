// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <expected>
#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> collect_or_error(View&& v) {
    std::vector<std::expected<char32_t, beman::transcoding::whatwg_error>> result;
    for (auto&& r : v)
        result.push_back(r);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_decode_or_error satisfies input_range", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'A'};
    auto              view = bytes | whatwg_decode_or_error<codec::utf_8>;
    static_assert(std::ranges::input_range<decltype(view)>);
    using val_t = std::ranges::range_value_t<decltype(view)>;
    static_assert(std::same_as<val_t, std::expected<char32_t, whatwg_error>>);
}

TEST_CASE("whatwg_decode_or_error valid ASCII", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'A'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'A');
}

TEST_CASE("whatwg_decode_or_error valid 2-byte UTF-8", "[transcoding::whatwg_decode_or_error]") {
    // U+00E9 é = 0xC3 0xA9
    std::vector<char> bytes{'\xC3', '\xA9'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == U'é');
}

TEST_CASE("whatwg_decode_or_error invalid lead byte 0xFF", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\xFF', 'A'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
    CHECK(result[1].has_value());
    CHECK(result[1].value() == U'A');
}

TEST_CASE("whatwg_decode_or_error truncated sequence", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\xC3'}; // 2-byte lead with no continuation
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::truncated_sequence);
}

TEST_CASE("whatwg_decode_or_error surrogate codepoint", "[transcoding::whatwg_decode_or_error]") {
    // U+D800 = 0xED 0xA0 0x80
    std::vector<char> bytes{'\xED', '\xA0', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
}

TEST_CASE("whatwg_decode_or_error bad continuation re-processing", "[transcoding::whatwg_decode_or_error]") {
    // 0xC3 + '(' — bad continuation; '(' is re-processed as ASCII.
    std::vector<char> bytes{'\xC3', '\x28'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 2);
    CHECK(!result[0].has_value());
    CHECK(result[1].has_value());
    CHECK(result[1].value() == U'(');
}

TEST_CASE("whatwg_decode_or_error consteval ASCII", "[transcoding::whatwg_decode_or_error]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_ascii = []() consteval {
        constexpr char        bytes[] = {'A'};
        std::span<const char> sp(bytes, 1);
        return (*(sp | whatwg_decode_or_error<codec::utf_8>).begin()).value();
    };
    CHECK(constify(decode_ascii()) == U'A');
}

// Step 14: codec::replacement tests

TEST_CASE("whatwg_decode_or_error replacement empty", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{};
    CHECK(collect_or_error(bytes | whatwg_decode_or_error<codec::replacement>).empty());
}

TEST_CASE("whatwg_decode_or_error replacement yields one error", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'H', 'e', 'l', 'l', 'o'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::replacement>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// Step 15: codec::x_user_defined tests

TEST_CASE("whatwg_decode_or_error x_user_defined never errors", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\x80', '\xFF', 'A'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::x_user_defined>);
    REQUIRE(result.size() == 3);
    for (auto& r : result)
        CHECK(r.has_value());
    CHECK(result[0].value() == char32_t(0xF780));
    CHECK(result[1].value() == char32_t(0xF7FF));
    CHECK(result[2].value() == U'A');
}

// Step 16: codec::windows_1252 tests

TEST_CASE("whatwg_decode_or_error windows_1252 c1 control byte", "[transcoding::whatwg_decode_or_error]") {
    // WHATWG maps 0x81 to U+0081 (C1 control); it is a valid mapping, not an error.
    std::vector<char> bytes{'\x81'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == char32_t(0x0081));
}

TEST_CASE("whatwg_decode_or_error windows_1252 valid high byte", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == char32_t(0x20AC));
}
