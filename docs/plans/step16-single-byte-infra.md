# Step 16: Single-Byte Table Infrastructure + `codec::windows_1252`

**Branch:** `step16-single-byte-infra`
**Depends on:** Step 14 (dispatch pattern). Step 15 is independent but should be done first for ordering.

---

## Goal

Introduce the generic table-driven single-byte decoder. All WHATWG single-byte codecs (windows-1252, ISO-8859-x, KOI8-x, etc.) share one algorithm with a different 128-entry lookup table for bytes 0x80–0xFF. `windows_1252` is the most commonly used legacy web encoding and the first concrete table, exercising both valid mappings and null (error) slots.

## WHATWG Specification — Single-Byte Decoder Algorithm

1. If byte is end-of-queue → finished.
2. If byte < 0x80 → return code point equal to byte value (ASCII identity).
3. Let code point = index[byte − 0x80].
4. If code point is null → return error.
5. Return code point.

**Table source:** WHATWG `index-windows-1252.txt` — 128 entries for bytes 0x80–0xFF. Five slots are null/unmapped: 0x81, 0x8D, 0x8F, 0x90, 0x9D.

## Files Created

### `include/beman/transcode/detail/single_byte.hpp`

Generic decode function shared by all single-byte codecs:

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP

#include <beman/transcode/detail/error.hpp>

#include <iterator>

namespace beman::transcoding::detail {

struct single_byte_result {
    char32_t     code_point{};
    whatwg_error error{};
    bool         is_error{false};
};

// Decode one byte using a 128-entry lookup table for the upper half.
// table[i] maps byte (0x80 + i). A table entry of 0 means "unmapped" (error).
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr single_byte_result single_byte_decode_one(
    I& current, [[maybe_unused]] S end, const char32_t (&table)[128]) {
    auto byte = static_cast<unsigned char>(*current);
    ++current;
    if (byte < 0x80)
        return {static_cast<char32_t>(byte), {}, false};
    char32_t cp = table[byte - 0x80];
    if (cp == 0)
        return {{}, whatwg_error::invalid_byte, true};
    return {cp, {}, false};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_HPP
```

### `include/beman/transcode/detail/tables/windows_1252.hpp`

The WHATWG-defined mapping for bytes 0x80–0xFF. The table has 128 entries; 0 = unmapped.

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_WINDOWS_1252_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_WINDOWS_1252_HPP

namespace beman::transcoding::detail::tables {

// WHATWG index-windows-1252: maps bytes 0x80–0xFF.
// Entry 0 = unmapped (error). Indexes: table[byte - 0x80].
// Source: https://encoding.spec.whatwg.org/index-windows-1252.txt
inline constexpr char32_t windows_1252[128] = {
    0x20AC, 0,      0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,  // 80–87
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0,      0x017D, 0,       // 88–8F
    0,      0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,  // 90–97
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0,      0x017E, 0x0178,  // 98–9F
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,  // A0–A7
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,  // A8–AF
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,  // B0–B7
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,  // B8–BF
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,  // C0–C7
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,  // C8–CF
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,  // D0–D7
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,  // D8–DF
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,  // E0–E7
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,  // E8–EF
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,  // F0–F7
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,  // F8–FF
};

} // namespace beman::transcoding::detail::tables

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_WINDOWS_1252_HPP
```

## Files Modified

### `include/beman/transcode/whatwg_decode_view.hpp`

1. Add includes:
   ```cpp
   #include <beman/transcode/detail/single_byte.hpp>
   #include <beman/transcode/detail/tables/windows_1252.hpp>
   ```

2. Extend `codec` enum:
   ```cpp
   enum class codec { utf_8, replacement, x_user_defined, windows_1252 };
   ```

3. Dispatch in `whatwg_decode_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::windows_1252) {
       auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1252);
       value_ = r.is_error ? U'\xFFFD' : r.code_point;
   }
   ```

4. Dispatch in `whatwg_decode_or_error_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::windows_1252) {
       auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1252);
       if (r.is_error)
           value_ = std::unexpected(r.error);
       else
           value_ = r.code_point;
   }
   ```

### `include/beman/transcode/CMakeLists.txt`

Add to HEADERS FILES list:
```cmake
detail/single_byte.hpp
detail/tables/windows_1252.hpp
```

### `tests/beman/transcode/whatwg_decode.test.cpp`

```cpp
TEST_CASE("whatwg_decode windows_1252 ASCII", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'A'});
}

TEST_CASE("whatwg_decode windows_1252 euro sign", "[transcoding::whatwg_decode]") {
    // 0x80 -> U+20AC (Euro sign) — the iconic windows-1252 mapping
    std::vector<char> bytes{'\x80'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'\x20AC'});
}

TEST_CASE("whatwg_decode windows_1252 smart quotes", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'\x93', '\x94'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) ==
          std::vector<char32_t>{U'\x201C', U'\x201D'});
}

TEST_CASE("whatwg_decode windows_1252 undefined byte", "[transcoding::whatwg_decode]") {
    // 0x81 is null/unmapped in windows-1252 per WHATWG
    std::vector<char> bytes{'\x81'};
    CHECK(collect(bytes | whatwg_decode<codec::windows_1252>) == std::vector<char32_t>{U'\xFFFD'});
}

TEST_CASE("whatwg_decode windows_1252 consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_euro = []() consteval {
        constexpr char        bytes[] = {'\x80'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::windows_1252>).begin();
    };
    CHECK(constify(decode_euro()) == char32_t(0x20AC));
}
```

### `tests/beman/transcode/whatwg_decode_or_error.test.cpp`

```cpp
TEST_CASE("whatwg_decode_or_error windows_1252 undefined byte", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\x81'};
    auto result = collect_or_error(bytes | whatwg_decode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}

TEST_CASE("whatwg_decode_or_error windows_1252 valid high byte", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\x80'};
    auto result = collect_or_error(bytes | whatwg_decode_or_error<codec::windows_1252>);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].has_value());
    CHECK(result[0].value() == char32_t(0x20AC));
}
```

## Verification

```bash
make test   # all previous + ~7 new tests pass
make lint   # clean
```
