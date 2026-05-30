# Step 14: `codec::replacement` Decoder

**Branch:** `step14-replacement-codec`
**Depends on:** Steps 0–13 complete (53 tests, `main`)

---

## Goal

Introduce `if constexpr` dispatch on the `codec C` template parameter in both decode view `load()` methods. The `replacement` codec is the simplest WHATWG codec and serves as the vehicle for establishing the multi-codec dispatch pattern.

## WHATWG Specification

Per the WHATWG Encoding Standard, the replacement decoder:
1. If byte is end-of-queue → finished.
2. If "replacement error returned" flag is unset → set flag, return error.
3. Return finished.

**Effect:** Any non-empty input produces exactly ONE U+FFFD (or one error in the or_error variant). All remaining input is consumed without producing additional output.

## Files Modified

### `include/beman/transcode/whatwg_decode_view.hpp`

1. **Line 16:** Extend the `codec` enum:
   ```cpp
   enum class codec { utf_8, replacement };
   ```

2. **`whatwg_decode_view::iterator::load()` (around line 179):** Add `if constexpr` dispatch:
   ```cpp
   constexpr void whatwg_decode_view<C, R>::iterator::load() {
       if (current_ == end_) {
           done_ = true;
           return;
       }
       if constexpr (C == codec::replacement) {
           // Consume all remaining input; yield one replacement character.
           while (current_ != end_)
               ++current_;
           value_ = U'\xFFFD';
       } else if constexpr (C == codec::utf_8) {
           auto r = detail::utf8_decode_one(current_, end_);
           value_ = r.is_error ? U'\xFFFD' : r.code_point;
       }
   }
   ```

3. **`whatwg_decode_or_error_view::iterator::load()` (around line 250):** Same dispatch:
   ```cpp
   constexpr void whatwg_decode_or_error_view<C, R>::iterator::load() {
       if (current_ == end_) {
           done_ = true;
           return;
       }
       if constexpr (C == codec::replacement) {
           while (current_ != end_)
               ++current_;
           value_ = std::unexpected(whatwg_error::invalid_byte);
       } else if constexpr (C == codec::utf_8) {
           auto r = detail::utf8_decode_one(current_, end_);
           if (r.is_error)
               value_ = std::unexpected(r.error);
           else
               value_ = r.code_point;
       }
   }
   ```

### `tests/beman/transcode/whatwg_decode.test.cpp`

Add after existing tests:

```cpp
TEST_CASE("whatwg_decode replacement empty input", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{};
    CHECK(collect(bytes | whatwg_decode<codec::replacement>).empty());
}

TEST_CASE("whatwg_decode replacement single byte", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'X'};
    CHECK(collect(bytes | whatwg_decode<codec::replacement>) == std::vector<char32_t>{U'\xFFFD'});
}

TEST_CASE("whatwg_decode replacement multiple bytes yields one U+FFFD", "[transcoding::whatwg_decode]") {
    std::vector<char> bytes{'H', 'e', 'l', 'l', 'o'};
    auto result = collect(bytes | whatwg_decode<codec::replacement>);
    REQUIRE(result.size() == 1);
    CHECK(result[0] == U'\xFFFD');
}

TEST_CASE("whatwg_decode replacement consteval", "[transcoding::whatwg_decode]") {
    using beman::transcoding::tests::constify;
    constexpr auto decode_replacement = []() consteval {
        constexpr char        bytes[] = {'a', 'b', 'c'};
        std::span<const char> sp(bytes, 3);
        return *(sp | whatwg_decode<codec::replacement>).begin();
    };
    CHECK(constify(decode_replacement()) == U'\xFFFD');
}
```

### `tests/beman/transcode/whatwg_decode_or_error.test.cpp`

Add after existing tests:

```cpp
TEST_CASE("whatwg_decode_or_error replacement empty", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{};
    CHECK(collect_or_error(bytes | whatwg_decode_or_error<codec::replacement>).empty());
}

TEST_CASE("whatwg_decode_or_error replacement yields one error", "[transcoding::whatwg_decode_or_error]") {
    std::vector<char> bytes{'H', 'e', 'l', 'l', 'o'};
    auto result = collect_or_error(bytes | whatwg_decode_or_error<codec::replacement>);
    REQUIRE(result.size() == 1);
    CHECK(!result[0].has_value());
    CHECK(result[0].error() == whatwg_error::invalid_byte);
}
```

## No New Files Created

This step only modifies existing files. No new headers, no new test executables, no CMakeLists changes needed.

## No Negative Compile Tests

No new constraints or `static_assert`s are introduced. The `if constexpr` dispatch is an internal implementation detail.

## Verification

```bash
make test   # existing 53 + ~6 new tests all pass
make lint   # clean
```
