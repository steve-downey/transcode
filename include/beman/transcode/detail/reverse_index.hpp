// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_REVERSE_INDEX_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_REVERSE_INDEX_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <array>
    #include <cstddef>
    #include <cstdint>

#endif
namespace beman::transcoding::detail {

// A WHATWG index is an array from pointer to code point.  Decoding reads it
// directly; encoding needs the other direction.  WHATWG defines "the index
// pointer for code point" as the first pointer whose entry is that code
// point, and finding it by scanning the array costs O(N) per code point.
// A reverse_index answers the same question in two loads.
//
// The reverse index is paged on the high bits of the code point: page_of
// maps cp >> 8 to a page number, and each page maps the low 8 bits to
// pointer + 1, with 0 meaning the code point is not in the index.  Pages
// that would be entirely empty are not stored, so the size is proportional
// to the number of 256-code-point blocks the index touches, not to the code
// space.  Index jis0208 touches 98 of them: about 50 KiB.
//
// Some encoders exclude a range of pointers from the search.  Shift_JIS
// excludes 8272-8835, the NEC-selected IBM extensions, so that their code
// points encode as the IBM extensions proper at 8836 and up.  The excluded
// range is applied when the index is built, so find() needs no special case.
//
// Build one with reverse_index_for<table, excluded>.  The table must be a
// constexpr array with static storage duration, which every generated index
// in detail/tables/ is.

// An index by address and length, so that it can be a template argument.
// Converts implicitly from the array, so callers name the table itself.  The
// generated indexes are C arrays; the pluggable table codecs take std::array.
struct table_ref {
    const char32_t* data{nullptr};
    std::size_t     size{0};

    template <std::size_t N>
    constexpr table_ref(const char32_t (&table)[N]);

    template <std::size_t N>
    constexpr table_ref(const std::array<char32_t, N>& table);

    constexpr table_ref(const char32_t* table, std::size_t count);
};

// An inclusive range of pointers; empty when last < first.
struct pointer_range {
    int first{0};
    int last{-1};

    constexpr bool contains(int pointer) const;
};

// The two sizes a reverse_index needs before it can be a type: how many page
// slots the highest mapped code point reaches, and how many of those pages
// hold at least one code point.
struct reverse_index_shape {
    std::size_t page_slots{0};
    std::size_t page_count{0};
};

template <reverse_index_shape Shape>
struct reverse_index {
    static constexpr std::uint16_t no_page = 0xFFFF;

    std::array<std::uint16_t, Shape.page_slots>                 page_of{};
    std::array<std::array<std::uint16_t, 256>, Shape.page_count> pages{};

    // The lowest pointer whose entry is cp, or -1 when cp is not in the index.
    constexpr int find(char32_t cp) const;
};

// The shape of the reverse index of table, with the excluded pointers left out.
constexpr reverse_index_shape reverse_index_shape_of(table_ref table, pointer_range excluded = {});

// The reverse index of table, with the excluded pointers left out.  Shape must
// be reverse_index_shape_of(table, excluded); reverse_index_for supplies it.
template <reverse_index_shape Shape>
constexpr reverse_index<Shape> make_reverse_index(table_ref table, pointer_range excluded = {});

// Holds the reverse index of Table as a static member, so that each
// translation unit builds it once, at compile time.
template <table_ref Table, pointer_range Excluded = {}>
struct reverse_index_builder {
    static_assert(Table.size < 0xFFFF, "transcode: reverse_index stores pointer + 1 in 16 bits");

    static constexpr reverse_index_shape  shape = reverse_index_shape_of(Table, Excluded);
    static constexpr reverse_index<shape> value = make_reverse_index<shape>(Table, Excluded);
};

// The reverse index of Table: reverse_index_for<tables::shift_jis, pointer_range{8272, 8835}>.
template <table_ref Table, pointer_range Excluded = {}>
inline constexpr const auto& reverse_index_for = reverse_index_builder<Table, Excluded>::value;

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <std::size_t N>
constexpr table_ref::table_ref(const char32_t (&table)[N]) : data(table), size(N) {}

template <std::size_t N>
constexpr table_ref::table_ref(const std::array<char32_t, N>& table) : data(table.data()), size(N) {}

constexpr table_ref::table_ref(const char32_t* table, std::size_t count) : data(table), size(count) {}

constexpr bool pointer_range::contains(int pointer) const { return first <= pointer && pointer <= last; }

template <reverse_index_shape Shape>
constexpr int reverse_index<Shape>::find(char32_t cp) const {
    std::size_t hi = cp >> 8;
    if (hi >= Shape.page_slots)
        return -1;
    std::uint16_t page = page_of[hi];
    if (page == no_page)
        return -1;
    return static_cast<int>(pages[page][cp & 0xFF]) - 1;
}

constexpr reverse_index_shape reverse_index_shape_of(table_ref table, pointer_range excluded) {
    char32_t highest = 0;
    bool     any     = false;
    for (std::size_t p = 0; p < table.size; ++p) {
        char32_t cp = table.data[p];
        if (cp == 0 || excluded.contains(static_cast<int>(p)))
            continue;
        any     = true;
        highest = cp > highest ? cp : highest;
    }
    if (!any)
        return {};

    reverse_index_shape shape{};
    shape.page_slots = static_cast<std::size_t>(highest >> 8) + 1;

    // Count the distinct pages.  Unicode has 0x1100 blocks of 256 code points.
    bool seen[0x1100] = {};
    for (std::size_t p = 0; p < table.size; ++p) {
        char32_t cp = table.data[p];
        if (cp == 0 || excluded.contains(static_cast<int>(p)))
            continue;
        if (!seen[cp >> 8]) {
            seen[cp >> 8] = true;
            ++shape.page_count;
        }
    }
    return shape;
}

template <reverse_index_shape Shape>
constexpr reverse_index<Shape> make_reverse_index(table_ref table, pointer_range excluded) {
    reverse_index<Shape> index{};
    for (auto& slot : index.page_of)
        slot = reverse_index<Shape>::no_page;
    // Touch every entry once, in order, before the scattered writes below.
    // GCC keeps a constant-evaluated array sparse until an element is written,
    // and inserting elements out of order costs a shift of everything after
    // them; filling densely first makes each later write an in-place update
    // and cuts the build of a large index from about a second to a fifth of one.
    for (auto& page : index.pages)
        for (auto& entry : page)
            entry = 0;

    std::uint16_t next_page = 0;
    for (std::size_t p = 0; p < table.size; ++p) {
        char32_t cp = table.data[p];
        if (cp == 0 || excluded.contains(static_cast<int>(p)))
            continue;
        std::size_t hi = cp >> 8;
        if (index.page_of[hi] == reverse_index<Shape>::no_page)
            index.page_of[hi] = next_page++;
        auto& entry = index.pages[index.page_of[hi]][cp & 0xFF];
        if (entry == 0)
            entry = static_cast<std::uint16_t>(p + 1);
    }
    return index;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_REVERSE_INDEX_HPP
