// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <vector>

namespace {
template <typename View>
std::vector<char> collect(View&& v) {
    std::vector<char> result;
    for (char b : v)
        result.push_back(b);
    return result;
}
} // namespace

using namespace beman::transcoding;

TEST_CASE("whatwg_encode_view satisfies input_range", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  view = cps | whatwg_encode<codec::windows_1252>;
    static_assert(std::ranges::input_range<decltype(view)>);
    static_assert(std::ranges::forward_range<decltype(view)>);
    static_assert(std::ranges::common_range<decltype(view)>);
    static_assert(std::ranges::range<const decltype(view)>);
    static_assert(std::ranges::input_range<const decltype(view)>);
    static_assert(std::ranges::forward_range<const decltype(view)>);
    static_assert(std::ranges::common_range<const decltype(view)>);
    static_assert(std::ranges::borrowed_range<decltype(view)>);
    static_assert(!std::ranges::borrowed_range<decltype(std::vector<char32_t>{U'A'} | whatwg_encode<codec::utf_8>)>);
    static_assert(std::copyable<std::ranges::iterator_t<decltype(view)>>);
    static_assert(std::ranges::random_access_range<decltype(view)>);
    static_assert(std::ranges::random_access_range<const decltype(view)>);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(view)>, char>);
}

TEST_CASE("whatwg_encode_view const iteration also works for owning views", "[transcoding::whatwg_encode]") {
    auto view = std::vector<char32_t>{U'A'} | whatwg_encode<codec::utf_8>;
    static_assert(std::ranges::range<const decltype(view)>);
}

TEST_CASE("whatwg_encode iso_2022_jp is common when base is common", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'A'};
    auto                  view = cps | whatwg_encode<codec::iso_2022_jp>;
    static_assert(std::ranges::common_range<decltype(view)>);
}

TEST_CASE("whatwg_encode_view keeps UTF-8 as non-random-access", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'\x20AC'};
    auto                  view = cps | whatwg_encode<codec::utf_8>;
    static_assert(std::ranges::forward_range<decltype(view)>);
    static_assert(std::ranges::borrowed_range<decltype(view)>);
    static_assert(std::copyable<std::ranges::iterator_t<decltype(view)>>);
    static_assert(!std::ranges::random_access_range<decltype(view)>);
    CHECK(collect(view) == std::vector<char>{'\xE2', '\x82', '\xAC'});
}

TEST_CASE("whatwg_encode_view forward iterators are multipass for UTF-8", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'\x20AC'};
    auto                  view = cps | whatwg_encode<codec::utf_8>;
    auto first = view.begin();
    auto copy = first;
    CHECK(*first == '\xE2');
    CHECK(*copy == '\xE2');
    ++first;
    CHECK(*copy == '\xE2');
    CHECK(*first == '\x82');
}

TEST_CASE("whatwg_encode_view supports indexing for indexed codecs", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'A', U'\x20AC', U'B'};
    auto                  view = cps | whatwg_encode<codec::windows_1252>;
    REQUIRE(view.size() == 3);
    auto it = view.begin();
    CHECK(it[0] == 'A');
    CHECK(it[1] == '\x80');
    CHECK(it[2] == 'B');
    it += 1;
    CHECK(*it == '\x80');
}

TEST_CASE("whatwg_encode iterators outlive borrowed views", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'\x20AC'};
    using view_t = decltype(cps | whatwg_encode<codec::utf_8>);
    std::ranges::iterator_t<view_t> it;
    {
        auto view = cps | whatwg_encode<codec::utf_8>;
        it = view.begin();
    }
    CHECK(*it == '\xE2');
    ++it;
    CHECK(*it == '\x82');
}

TEST_CASE("whatwg_encode windows_1252 ASCII passthrough", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'H', U'i'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'H', 'i'});
}

TEST_CASE("whatwg_encode windows_1252 euro sign", "[transcoding::whatwg_encode]") {
    // U+20AC (Euro sign) -> 0x80 in windows-1252
    std::vector<char32_t> cps{U'\x20AC'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'\x80'});
}

TEST_CASE("whatwg_encode windows_1252 unmapped codepoint yields question mark", "[transcoding::whatwg_encode]") {
    // U+4E2D (CJK) is not in windows-1252
    std::vector<char32_t> cps{U'\x4E2D'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'?'});
}

TEST_CASE("whatwg_encode iso_8859_2 A-ogonek", "[transcoding::whatwg_encode]") {
    // U+0104 (Latin capital A with ogonek) -> 0xA1 in iso-8859-2
    std::vector<char32_t> cps{U'\x0104'};
    CHECK(collect(cps | whatwg_encode<codec::iso_8859_2>) == std::vector<char>{'\xA1'});
}

TEST_CASE("whatwg_encode koi8_r Cyrillic A", "[transcoding::whatwg_encode]") {
    // U+0410 (Cyrillic capital A) -> 0xE1 in KOI8-R
    std::vector<char32_t> cps{U'\x0410'};
    CHECK(collect(cps | whatwg_encode<codec::koi8_r>) == std::vector<char>{'\xE1'});
}

TEST_CASE("whatwg_encode windows_1252 mixed ASCII and high", "[transcoding::whatwg_encode]") {
    std::vector<char32_t> cps{U'A', U'\x20AC', U'B'};
    CHECK(collect(cps | whatwg_encode<codec::windows_1252>) == std::vector<char>{'A', '\x80', 'B'});
}

TEST_CASE("whatwg_encode windows_1252 consteval", "[transcoding::whatwg_encode]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_euro = []() consteval {
        constexpr char32_t        cps[] = {U'\x20AC'};
        std::span<const char32_t> sp(cps, 1);
        return *(sp | whatwg_encode<codec::windows_1252>).begin();
    };
    CHECK(constify(encode_euro()) == '\x80');
}

// ---------------------------------------------------------------------------
// UTF-8 encoder tests (step 20)
// ---------------------------------------------------------------------------

TEST_CASE("whatwg_encode UTF-8 ASCII single byte", "[transcoding::whatwg_encode][utf8]") {
    std::vector<char32_t> cps{U'A'};
    CHECK(collect(cps | whatwg_encode<codec::utf_8>) == std::vector<char>{'A'});
}

TEST_CASE("whatwg_encode UTF-8 2-byte sequence", "[transcoding::whatwg_encode][utf8]") {
    // U+00E9 (e acute) -> 0xC3 0xA9
    std::vector<char32_t> cps{U'\x00E9'};
    CHECK(collect(cps | whatwg_encode<codec::utf_8>) == std::vector<char>{'\xC3', '\xA9'});
}

TEST_CASE("whatwg_encode UTF-8 3-byte sequence", "[transcoding::whatwg_encode][utf8]") {
    // U+20AC (Euro sign) -> 0xE2 0x82 0xAC
    std::vector<char32_t> cps{U'\x20AC'};
    CHECK(collect(cps | whatwg_encode<codec::utf_8>) == std::vector<char>{'\xE2', '\x82', '\xAC'});
}

TEST_CASE("whatwg_encode UTF-8 4-byte sequence", "[transcoding::whatwg_encode][utf8]") {
    // U+1F600 (grinning face emoji) -> 0xF0 0x9F 0x98 0x80
    std::vector<char32_t> cps{U'\x1F600'};
    CHECK(collect(cps | whatwg_encode<codec::utf_8>) == std::vector<char>{'\xF0', '\x9F', '\x98', '\x80'});
}

TEST_CASE("whatwg_encode UTF-8 surrogate replaced with U+FFFD", "[transcoding::whatwg_encode][utf8]") {
    // U+D800 (surrogate) -> U+FFFD encoded as {0xEF, 0xBF, 0xBD}
    std::vector<char32_t> cps{static_cast<char32_t>(0xD800)};
    CHECK(collect(cps | whatwg_encode<codec::utf_8>) == std::vector<char>{'\xEF', '\xBF', '\xBD'});
}

TEST_CASE("whatwg_encode UTF-8 multi-codepoint string", "[transcoding::whatwg_encode][utf8]") {
    // "Hi" + Euro -> "Hi" + 3 bytes
    std::vector<char32_t> cps{U'H', U'i', U'\x20AC'};
    CHECK(collect(cps | whatwg_encode<codec::utf_8>) == std::vector<char>{'H', 'i', '\xE2', '\x82', '\xAC'});
}

TEST_CASE("whatwg_encode UTF-8 consteval", "[transcoding::whatwg_encode][utf8]") {
    using beman::transcoding::tests::constify;
    constexpr auto encode_euro = []() consteval {
        constexpr char32_t        cps[] = {U'\x20AC'};
        std::span<const char32_t> sp(cps, 1);
        auto                      it = (sp | whatwg_encode<codec::utf_8>).begin();
        char                      b0 = *it;
        ++it;
        char b1 = *it;
        ++it;
        char b2 = *it;
        return b0 == '\xE2' && b1 == '\x82' && b2 == '\xAC';
    };
    CHECK(constify(encode_euro()));
}
