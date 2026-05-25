# Step 18: UTF-8 Encoder — `whatwg_encode_view<codec::utf_8>`

**Branch:** `step18-utf8-encoder`
**Depends on:** Step 14 (established dispatch), but conceptually independent of steps 15–17

---

## Goal

Add the encoding direction: `whatwg_encode_view<codec::utf_8, R>` takes a `char32_t` input range and yields `char` output bytes. This is the symmetric counterpart to the existing `whatwg_decode_view`. Includes an or_error variant, pipe closures, and a new `unicode_scalar_range` concept.

## WHATWG Specification — UTF-8 Encoder

1. If code point is end-of-queue → finished.
2. If code point is in U+0000–U+007F → return that byte.
3. If code point is in U+0080–U+07FF → return 2 bytes.
4. If code point is in U+0800–U+FFFF (excluding surrogates U+D800–U+DFFF) → return 3 bytes.
5. If code point is in U+10000–U+10FFFF → return 4 bytes.
6. If code point is a surrogate or > U+10FFFF → error.

**Error handling (non-error variant):** Surrogates/out-of-range → emit U+FFFD encoded as UTF-8: {0xEF, 0xBF, 0xBD}.
**Error handling (or_error variant):** Surrogates → `unexpected(whatwg_error::surrogate_code_point)`. Out-of-range → `unexpected(whatwg_error::out_of_range)`.

## Files Created

### `include/beman/transcode/detail/utf8_encode.hpp`

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_ENCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_ENCODE_HPP

#include <beman/transcode/detail/error.hpp>

namespace beman::transcoding::detail {

struct utf8_encode_result {
    char         bytes[4]{};
    int          count{};
    whatwg_error error{};
    bool         is_error{false};
};

constexpr utf8_encode_result utf8_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definition
// ---------------------------------------------------------------------------

constexpr utf8_encode_result utf8_encode_one(char32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, 0, whatwg_error::surrogate_code_point, true};
    if (cp > 0x10FFFF)
        return {{}, 0, whatwg_error::out_of_range, true};

    utf8_encode_result r{};
    if (cp < 0x80) {
        r.bytes[0] = static_cast<char>(cp);
        r.count    = 1;
    } else if (cp < 0x800) {
        r.bytes[0] = static_cast<char>(0xC0 | (cp >> 6));
        r.bytes[1] = static_cast<char>(0x80 | (cp & 0x3F));
        r.count    = 2;
    } else if (cp < 0x10000) {
        r.bytes[0] = static_cast<char>(0xE0 | (cp >> 12));
        r.bytes[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        r.bytes[2] = static_cast<char>(0x80 | (cp & 0x3F));
        r.count    = 3;
    } else {
        r.bytes[0] = static_cast<char>(0xF0 | (cp >> 18));
        r.bytes[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        r.bytes[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        r.bytes[3] = static_cast<char>(0x80 | (cp & 0x3F));
        r.count    = 4;
    }
    return r;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF8_ENCODE_HPP
```

### `include/beman/transcode/whatwg_encode_view.hpp`

New header with:
- `whatwg_encode_view<C, R>` — view yielding `char`, constexpr
- `whatwg_encode_or_error_view<C, R>` — view yielding `expected<char, whatwg_error>`, constexpr
- `whatwg_encode_closure<C>` — pipe adapter
- `whatwg_encode_or_error_closure<C>` — pipe adapter
- Global variables: `whatwg_encode<C>`, `whatwg_encode_or_error<C>`

**Iterator design:**
```
class iterator {
    base_iter current_;
    base_sent end_;
    char      buf_[4]{};    // encoded bytes for current codepoint
    int       len_{0};      // how many bytes in buf_
    int       pos_{0};      // which byte to yield next
    bool      done_{false};

    constexpr void load();  // encode next codepoint into buf_
};
```

`load()` reads one `char32_t` from `current_`, calls `detail::utf8_encode_one()`, fills `buf_/len_/pos_`.

For the non-error variant: on encoder error, substitute U+FFFD → 3 bytes {0xEF, 0xBF, 0xBD}.
For the or_error variant: the iterator needs additional `has_error_`/`error_value_` state (similar to `iconv_transcode_or_error_view`).

**Input concept:** Requires `unicode_scalar_range<R>` (see below).

**Include guard:** `INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP`

**Array rejection:** Add a `static_assert` overload of `operator|` that rejects `char32_t[N]` arrays with message `"transcode: raw arrays are not valid input to whatwg_encode"`.

### `tests/beman/transcode/whatwg_encode.test.cpp`

New test executable. Include `whatwg_encode_view.hpp` twice. Tests:

```cpp
TEST_CASE("whatwg_encode_view satisfies input_range") { /* static_asserts */ }
TEST_CASE("whatwg_encode UTF-8 single ASCII") { /* U+0041 -> {0x41} */ }
TEST_CASE("whatwg_encode UTF-8 2-byte") { /* U+00E9 -> {0xC3, 0xA9} */ }
TEST_CASE("whatwg_encode UTF-8 3-byte") { /* U+20AC -> {0xE2, 0x82, 0xAC} */ }
TEST_CASE("whatwg_encode UTF-8 4-byte") { /* U+1F600 -> {0xF0, 0x9F, 0x98, 0x80} */ }
TEST_CASE("whatwg_encode UTF-8 surrogate replaced") { /* U+D800 -> {0xEF, 0xBF, 0xBD} */ }
TEST_CASE("whatwg_encode UTF-8 pipe syntax") { /* codepoints | whatwg_encode<codec::utf_8> */ }
TEST_CASE("whatwg_encode UTF-8 consteval") { /* compile-time encode check */ }
```

### `tests/beman/transcode/whatwg_encode_or_error.test.cpp`

New test executable. Tests:

```cpp
TEST_CASE("whatwg_encode_or_error surrogate") { /* unexpected(surrogate_code_point) */ }
TEST_CASE("whatwg_encode_or_error out_of_range") { /* unexpected(out_of_range) */ }
TEST_CASE("whatwg_encode_or_error valid codepoint") { /* has_value(), correct bytes */ }
```

### Negative compile test: `whatwg_encode_reject_char_range_fail.cpp`

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <beman/transcode/whatwg_encode_view.hpp>
#include <vector>
using namespace beman::transcoding;
using view_t = whatwg_encode_view<codec::utf_8, std::vector<char>>;
// This must fail: vector<char> is not unicode_scalar_range
static_assert(std::ranges::input_range<view_t>,
              "transcode: whatwg_encode requires a unicode_scalar_range (char32_t elements)");
```

## Files Modified

### `include/beman/transcode/detail/concepts.hpp`

Add after `legacy_byte_range`:

```cpp
template <typename R>
concept unicode_scalar_range = std::ranges::input_range<R> &&
    std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, char32_t>;
```

### `include/beman/transcode/CMakeLists.txt`

Add to HEADERS FILES list:
```cmake
detail/utf8_encode.hpp
whatwg_encode_view.hpp
```

### `tests/beman/transcode/CMakeLists.txt`

Add two new test executables (whatwg_encode, whatwg_encode_or_error) + catch_discover_tests. Add negative compile test target + PASS_REGULAR_EXPRESSION.

## Verification

```bash
make test   # all previous + ~11 new tests + 1 negative compile test pass
make lint   # clean
```
