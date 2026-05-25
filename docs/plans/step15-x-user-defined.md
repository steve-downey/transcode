# Step 15: `codec::x_user_defined` Decoder

**Branch:** `step15-x-user-defined`
**Depends on:** Step 14 (dispatch pattern established)

---

## Goal

Add the `x_user_defined` codec — a simple algorithmic decoder that never errors. Validates that the `if constexpr` dispatch works with a codec that has real per-byte computation (unlike `replacement` which has trivial behavior).

## WHATWG Specification

The x-user-defined decoder:
1. If byte is end-of-queue → finished.
2. If byte is an ASCII byte (< 0x80) → return code point equal to byte value.
3. Return code point = 0xF780 + byte − 0x80.

**Key property:** This codec NEVER returns an error. Every byte maps to a valid codepoint.

## Files Created

### `include/beman/transcode/detail/x_user_defined.hpp`

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP

#include <iterator>

namespace beman::transcoding::detail {

// Decode one byte using the x-user-defined algorithm.
// Advances current past the consumed byte. Never errors.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr char32_t x_user_defined_decode_one(I& current, [[maybe_unused]] S end) {
    auto byte = static_cast<unsigned char>(*current);
    ++current;
    if (byte < 0x80)
        return static_cast<char32_t>(byte);
    return static_cast<char32_t>(0xF780 + (byte - 0x80));
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_X_USER_DEFINED_HPP
```

## Files Modified

### `include/beman/transcode/whatwg_decode_view.hpp`

1. Add include (after the existing `#include <beman/transcode/detail/utf8.hpp>`):
   ```cpp
   #include <beman/transcode/detail/x_user_defined.hpp>
   ```

2. Extend the `codec` enum:
   ```cpp
   enum class codec { utf_8, replacement, x_user_defined };
   ```

3. Add dispatch branch in `whatwg_decode_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::x_user_defined) {
       value_ = detail::x_user_defined_decode_one(current_, end_);
   }
   ```

4. Add dispatch branch in `whatwg_decode_or_error_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::x_user_defined) {
       value_ = detail::x_user_defined_decode_one(current_, end_);
   }
   ```
   (Always a valid value — never `unexpected`.)

### `include/beman/transcode/CMakeLists.txt`

Add to the HEADERS FILES list:
```cmake
detail/x_user_defined.hpp
```

### `tests/beman/transcode/whatwg_decode.test.cpp`

```cpp
TEST_CASE("whatwg_decode x_user_defined ASCII", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'A', 'z'};
    CHECK(collect(bytes | whatwg_decode<codec::x_user_defined>) == std::vector<char32_t>{U'A', U'z'});
}

TEST_CASE("whatwg_decode x_user_defined high bytes", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'\x80', '\xFF'};
    auto result = collect(bytes | whatwg_decode<codec::x_user_defined>);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == char32_t(0xF780));
    CHECK(result[1] == char32_t(0xF7FF));
}

TEST_CASE("whatwg_decode x_user_defined consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_x = []() consteval {
        constexpr char        bytes[] = {'\x80'};
        std::span<const char> sp(bytes, 1);
        return *(sp | whatwg_decode<codec::x_user_defined>).begin();
    };
    CHECK(constify(decode_x()) == char32_t(0xF780));
}
```

### `tests/beman/transcode/whatwg_decode_or_error.test.cpp`

```cpp
TEST_CASE("whatwg_decode_or_error x_user_defined never errors", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'\x80', '\xFF', 'A'};
    auto result = collect_or_error(bytes | whatwg_decode_or_error<codec::x_user_defined>);
    REQUIRE(result.size() == 3);
    for (auto& r : result)
        CHECK(r.has_value());
    CHECK(result[0].value() == char32_t(0xF780));
    CHECK(result[1].value() == char32_t(0xF7FF));
    CHECK(result[2].value() == U'A');
}
```

## Verification

```bash
make test   # all previous + ~4 new tests pass
make lint   # clean
```
