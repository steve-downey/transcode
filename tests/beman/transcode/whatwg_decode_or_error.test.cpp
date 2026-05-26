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
    static_assert(std::ranges::forward_range<decltype(view)>);
    static_assert(std::ranges::common_range<decltype(view)>);
    static_assert(std::ranges::range<const decltype(view)>);
    static_assert(std::ranges::input_range<const decltype(view)>);
    static_assert(std::ranges::forward_range<const decltype(view)>);
    static_assert(std::ranges::common_range<const decltype(view)>);
    static_assert(std::ranges::borrowed_range<decltype(view)>);
    static_assert(
        !std::ranges::borrowed_range<decltype(std::vector<char>{'A'} | whatwg_decode_or_error<codec::utf_8>)>);
    static_assert(std::copyable<std::ranges::iterator_t<decltype(view)>>);
    static_assert(!std::ranges::random_access_range<decltype(view)>);
    using val_t = std::ranges::range_value_t<decltype(view)>;
    static_assert(std::same_as<val_t, std::expected<char32_t, whatwg_error>>);
}

TEST_CASE("whatwg_decode_or_error const iteration also works for owning views",
          "[transcoding::whatwg_decode_or_error]") {
    auto view = std::vector<char>{'A'} | whatwg_decode_or_error<codec::utf_8>;
    static_assert(std::ranges::range<const decltype(view)>);
}

TEST_CASE("whatwg_decode_or_error iso_2022_jp is common when base is common",
          "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'A'};
    auto              view = bytes | whatwg_decode_or_error<codec::iso_2022_jp>;
    static_assert(std::ranges::common_range<decltype(view)>);
}

TEST_CASE("whatwg_decode_or_error forward iterators are multipass for UTF-8",
          "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'A', '\xFF'};
    auto              view  = bytes | whatwg_decode_or_error<codec::utf_8>;
    auto              first = view.begin();
    auto              copy  = first;
    REQUIRE((*first).has_value());
    REQUIRE((*copy).has_value());
    CHECK((*first).value() == U'A');
    ++first;
    CHECK((*copy).value() == U'A');
    CHECK(!(*first).has_value());
    CHECK((*first).error() == whatwg_error::invalid_byte);
}

TEST_CASE("whatwg_decode_or_error lifts to random_access_range for indexed codecs",
          "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'A', '\xA1', 'B'};
    auto              view = bytes | whatwg_decode_or_error<codec::iso_8859_6>;
    static_assert(std::ranges::random_access_range<decltype(view)>);
    static_assert(std::ranges::random_access_range<const decltype(view)>);
    static_assert(std::ranges::borrowed_range<decltype(view)>);
    REQUIRE(view.size() == 3);
    auto it = view.begin();
    CHECK(it[0].has_value());
    CHECK(it[0].value() == U'A');
    CHECK(!it[1].has_value());
    CHECK(it[1].error() == whatwg_error::invalid_byte);
    CHECK(it[2].has_value());
    CHECK(it[2].value() == U'B');
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
    // WHATWG: ED requires first continuation byte in 80-9F; A0 is rejected
    // early as surrogate_code_point without consuming it.  Then A0 and 80
    // are bare continuations → invalid_byte each.  3 errors total.
    std::vector<char> bytes{'\xED', '\xA0', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 3);
    CHECK(result[0].error() == whatwg_error::surrogate_code_point);
    CHECK(result[1].error() == whatwg_error::invalid_byte);
    CHECK(result[2].error() == whatwg_error::invalid_byte);
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

// Coverage: iso_8859_6 success path (valid high byte).
// 0xA0 -> U+00A0 (NO-BREAK SPACE).
TEST_CASE("whatwg_decode_or_error iso_8859_6 valid high byte", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\xA0'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_8859_6>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == char32_t(0x00A0));
}

// Coverage gap: single_byte_decode_one() error path (cp == 0 in table).
// iso-8859-6 has genuine null entries; byte 0xA1 is unmapped.
TEST_CASE("whatwg_decode_or_error iso_8859_6 unmapped byte yields error", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\xA1'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::iso_8859_6>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

// WHATWG: E0 requires first continuation byte in A0-BF; 80 is rejected early.
TEST_CASE("whatwg_decode_or_error overlong 3-byte sequence", "[transcoding::whatwg_decode_or_error]") {
    // E0 80 80: E0 with cont 80 < A0 → overlong (not consumed), then
    // two bare continuations → invalid_byte each.  3 errors total.
    std::vector<char> bytes{'\xE0', '\x80', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 3);
    CHECK(result[0].error() == whatwg_error::overlong_encoding);
    CHECK(result[1].error() == whatwg_error::invalid_byte);
    CHECK(result[2].error() == whatwg_error::invalid_byte);
}

// WHATWG: F0 requires first continuation byte in 90-BF; 80 is rejected early.
TEST_CASE("whatwg_decode_or_error overlong 4-byte sequence", "[transcoding::whatwg_decode_or_error]") {
    // F0 80 80 80: F0 with cont 80 < 90 → overlong (not consumed), then
    // three bare continuations → invalid_byte each.  4 errors total.
    std::vector<char> bytes{'\xF0', '\x80', '\x80', '\x80'};
    auto              result = collect_or_error(bytes | whatwg_decode_or_error<codec::utf_8>);
    REQUIRE(result.size() == 4);
    CHECK(result[0].error() == whatwg_error::overlong_encoding);
    CHECK(result[1].error() == whatwg_error::invalid_byte);
    CHECK(result[2].error() == whatwg_error::invalid_byte);
    CHECK(result[3].error() == whatwg_error::invalid_byte);
}
