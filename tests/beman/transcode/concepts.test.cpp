// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/concepts.hpp>
#include <catch2/catch_all.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using beman::transcoding::legacy_byte_range;

// Accepted types: char, signed char, unsigned char, std::byte
static_assert(legacy_byte_range<std::vector<char>>);
static_assert(legacy_byte_range<std::vector<unsigned char>>);
static_assert(legacy_byte_range<std::vector<signed char>>);
static_assert(legacy_byte_range<std::vector<std::byte>>);
static_assert(legacy_byte_range<std::span<char>>);
static_assert(legacy_byte_range<std::span<unsigned char>>);
static_assert(legacy_byte_range<std::span<std::byte>>);
static_assert(legacy_byte_range<std::string>);
static_assert(legacy_byte_range<std::string_view>);

// Rejected types: char8_t, char16_t, char32_t, wchar_t
static_assert(!legacy_byte_range<std::vector<char8_t>>);
static_assert(!legacy_byte_range<std::u8string>);
static_assert(!legacy_byte_range<std::u8string_view>);
static_assert(!legacy_byte_range<std::vector<char16_t>>);
static_assert(!legacy_byte_range<std::u16string_view>);
static_assert(!legacy_byte_range<std::vector<char32_t>>);
static_assert(!legacy_byte_range<std::u32string_view>);
static_assert(!legacy_byte_range<std::vector<wchar_t>>);
static_assert(!legacy_byte_range<std::wstring_view>);

// Rejected: non-byte types
static_assert(!legacy_byte_range<std::vector<int>>);
static_assert(!legacy_byte_range<std::vector<float>>);

// Rejected: arrays (decay ban)
static_assert(!legacy_byte_range<char[5]>);
static_assert(!legacy_byte_range<const char[5]>);
static_assert(!legacy_byte_range<unsigned char[5]>);

TEST_CASE("legacy_byte_range accepts runtime ranges", "[transcoding::concepts]") {
    std::vector<char>          vc{'a', 'b'};
    std::vector<unsigned char> vuc{1, 2};
    std::vector<std::byte>     vb{std::byte{0}, std::byte{1}};

    CHECK(std::ranges::range<decltype(vc)>);
    CHECK(std::ranges::range<decltype(vuc)>);
    CHECK(std::ranges::range<decltype(vb)>);
}
