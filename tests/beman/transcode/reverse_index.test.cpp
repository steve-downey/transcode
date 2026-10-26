// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/reverse_index.hpp>
#include <beman/transcode/detail/reverse_index.hpp>
#include <beman/transcode/detail/tables/shift_jis.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <cstddef>

namespace beman::transcoding::tests {

// A small index with the shapes that matter: an unmapped pointer, code points
// that appear twice, a code point outside the BMP, and one in the last BMP
// block.  Pointer -> code point:
//   0 -> U+3000   1 -> (none)   2 -> U+00E9   3 -> U+3000 (again)
//   4 -> U+20000  5 -> U+00E9 (again)   6 -> U+FFFD   7 -> U+0041
inline constexpr char32_t small[8] = {U'\x3000', 0, U'\x00E9', U'\x3000', U'\x20000', U'\x00E9', U'\xFFFD', U'A'};

} // namespace beman::transcoding::tests

using namespace beman::transcoding::detail;
using beman::transcoding::tests::constify;
using beman::transcoding::tests::small;

namespace {

// The definition the reverse index must agree with: the first pointer whose
// entry is cp, skipping the excluded range.
template <std::size_t N>
int reference_pointer(const char32_t (&table)[N], char32_t cp, pointer_range excluded = {}) {
    for (std::size_t p = 0; p < N; ++p)
        if (table[p] == cp && !excluded.contains(static_cast<int>(p)))
            return static_cast<int>(p);
    return -1;
}

} // namespace

TEST_CASE("reverse_index shape counts page slots and non-empty pages", "[transcoding::reverse_index]") {
    constexpr auto shape = reverse_index_shape_of(small);
    // Highest code point is U+20000, so the slots run to page 0x200 inclusive.
    CHECK(shape.page_slots == 0x201);
    // Pages 0x00 (U+00E9, U+0041), 0x30, 0x200, and 0xFF are populated.
    CHECK(shape.page_count == 4);
}

TEST_CASE("reverse_index shape of an empty table is empty", "[transcoding::reverse_index]") {
    static constexpr char32_t none[3] = {};
    constexpr auto            shape   = reverse_index_shape_of(none);
    CHECK(shape.page_slots == 0);
    CHECK(shape.page_count == 0);
    CHECK(reverse_index_for<none>.find(U'A') == -1);
}

TEST_CASE("reverse_index finds the lowest pointer for a duplicated code point", "[transcoding::reverse_index]") {
    const auto& index = reverse_index_for<small>;
    CHECK(index.find(U'\x3000') == 0);
    CHECK(index.find(U'\x00E9') == 2);
    CHECK(index.find(U'\x20000') == 4);
    CHECK(index.find(U'\xFFFD') == 6);
    CHECK(index.find(U'A') == 7);
}

TEST_CASE("reverse_index reports code points that are not in the index", "[transcoding::reverse_index]") {
    const auto& index = reverse_index_for<small>;
    // Same page as a mapped code point, different low byte.
    CHECK(index.find(U'\x3001') == -1);
    // A page with nothing in it.
    CHECK(index.find(U'\x1234') == -1);
    // Beyond the last page slot.
    CHECK(index.find(U'\x30000') == -1);
    CHECK(index.find(U'\x10FFFF') == -1);
    // Zero marks an unmapped pointer in the table; it is never a mapping.
    CHECK(index.find(0) == -1);
}

TEST_CASE("reverse_index leaves the excluded pointers out", "[transcoding::reverse_index]") {
    // Excluding pointer 0 moves U+3000 to its second occurrence.
    CHECK(reverse_index_for<small, pointer_range{0, 0}>.find(U'\x3000') == 3);
    // Excluding 2-5 removes U+00E9 and U+20000 entirely, and U+3000 keeps pointer 0.
    const auto& index = reverse_index_for<small, pointer_range{2, 5}>;
    CHECK(index.find(U'\x00E9') == -1);
    CHECK(index.find(U'\x20000') == -1);
    CHECK(index.find(U'\x3000') == 0);
    CHECK(index.find(U'A') == 7);
}

TEST_CASE("pointer_range contains its endpoints and nothing outside", "[transcoding::reverse_index]") {
    constexpr pointer_range r{8272, 8835};
    CHECK(r.contains(8272));
    CHECK(r.contains(8835));
    CHECK(!r.contains(8271));
    CHECK(!r.contains(8836));
    constexpr pointer_range none{};
    CHECK(!none.contains(0));
    CHECK(!none.contains(-1));
}

TEST_CASE("reverse_index consteval", "[transcoding::reverse_index]") {
    CHECK(constify(reverse_index_for<small>.find(U'\x00E9')) == 2);
    CHECK(constify(reverse_index_for<small, pointer_range{2, 2}>.find(U'\x00E9')) == 5);
    CHECK(constify(reverse_index_for<small>.find(U'\x1234')) == -1);
    CHECK(constify(pointer_range{1, 3}.contains(2)));
}

TEST_CASE("reverse_index of index jis0208 agrees with a linear scan for every code point",
          "[transcoding::reverse_index]") {
    const auto& index = reverse_index_for<tables::shift_jis>;
    for (char32_t cp : tables::shift_jis) {
        if (cp == 0)
            continue;
        CHECK(index.find(cp) == reference_pointer(tables::shift_jis, cp));
    }
}

TEST_CASE("reverse_index of index jis0208 with the Shift_JIS exclusion agrees with a linear scan",
          "[transcoding::reverse_index]") {
    constexpr pointer_range excluded{8272, 8835};
    const auto&             index = reverse_index_for<tables::shift_jis, excluded>;
    for (char32_t cp : tables::shift_jis) {
        if (cp == 0)
            continue;
        int pointer = index.find(cp);
        CHECK(pointer == reference_pointer(tables::shift_jis, cp, excluded));
        CHECK(!excluded.contains(pointer));
    }
}
