# Handoff: beman.transcode — Step 21 (UTF-16 decode + encode)

## Project

`beman.transcode` — C++20 header-only library for Unicode transcoding
using ranges and views. Beman Project incubation for a C++29 standard
proposal.

- GitHub: `https://github.com/steve-downey/transcode.git` (remote `origin`)
- Bloomberg: `bbgithub.dev.bloomberg.com:sdowney/transcode.git` (remote `bbgithub`)
- Working directory: `/home/sdowney/src/steve-downey/transcode/transcode`
- Two remotes; always push both: `git push origin <branch> && git push bbgithub <branch>`
- Merge to main: `git merge --no-ff`

## Current State

**128 C++ tests + 33 Python tests pass** (`make test`). Steps 0–20
complete. On `main`.

### What Step 20 Built

Step 20 added the UTF-8 encode direction and refactored the encode
iterator to a buffer design:

- **`include/beman/transcode/detail/utf8_encode.hpp`** — new file with
  `utf8_encode_result` and `constexpr utf8_encode_one(char32_t)`.
  Covers all 4 byte-length ranges plus surrogate and out-of-range errors.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — refactored
  both iterators from a single-value `value_` field to a buffer design
  (`buf_[4]`, `len_`, `pos_`). Added `codec::utf_8` dispatch arm in
  both `load()` functions. Single-byte arms now use `encode_single()`
  lambda, UTF-8 arm calls `utf8_encode_one()` directly.

- **`tests/beman/transcode/whatwg_encode_reject_char_range_fail.cpp`** —
  new negative compile test: `vector<char>` rejected as
  `unicode_scalar_range` (elements are `char`, not `char32_t`).

### Current iterator designs — IMPORTANT for step 21

**Decode side** (`whatwg_decode_view::iterator`):
```cpp
base_iter current_;
base_sent end_;
char32_t  value_{};   // single decoded codepoint
bool      done_{false};
```
`load()` consumes bytes and produces one `char32_t`. This design works
for UTF-16 too: consume 2 or 4 bytes, produce one `char32_t`. No buffer
refactor needed for the decode side.

**Encode side** (`whatwg_encode_view::iterator`) — already uses buffer:
```cpp
char buf_[4]{};
int  len_{0};
int  pos_{0};
bool done_{false};
```
UTF-16 produces exactly 2 bytes (BMP) or 4 bytes (surrogate pair) per
codepoint. The buffer design handles this. No refactor needed.

### Current `codec` enum (in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8,
    replacement,
    x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    // utf_16be and utf_16le NOT YET HERE — step 21 adds them
};
```

## What To Do Next — Step 21

**Branch:** `step21-utf16`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step21 plan file yet. Use this handoff as the
authoritative spec.

### WHATWG UTF-16 decoder algorithm

UTF-16 decode (`codec::utf_16be` / `codec::utf_16le`):

1. Read 2 bytes; interpret as a 16-bit code unit (BE: `b0<<8 | b1`, LE: `b1<<8 | b0`).
2. If the code unit is in `0xD800–0xDBFF` (high surrogate):
   a. Read another 2 bytes as a low surrogate unit.
   b. If low surrogate is in `0xDC00–0xDFFF`, combine:
      `cp = 0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00)`
   c. Otherwise: emit U+FFFD (non-error variant) or `surrogate_code_point`
      error (or_error variant), and do NOT consume the second pair.
3. If the code unit is in `0xDC00–0xDFFF` (lone low surrogate):
   emit U+FFFD or error.
4. Otherwise: the code unit IS the codepoint.
5. If fewer than 2 bytes remain: emit U+FFFD or error.

### WHATWG UTF-16 encoder algorithm

UTF-16 encode (`codec::utf_16be` / `codec::utf_16le`):

1. If `cp` is a surrogate (`0xD800–0xDFFF`): error (`surrogate_code_point`).
2. If `cp <= 0xFFFF`: emit 2 bytes (the 16-bit value in BE or LE).
3. If `cp >= 0x10000`: emit 4 bytes (surrogate pair):
   ```
   high = 0xD800 + ((cp - 0x10000) >> 10)
   low  = 0xDC00 + ((cp - 0x10000) & 0x3FF)
   ```
   Output high then low (each as 2 bytes in the correct byte order).

### Implementation order

1. **Add enum values** to `codec` enum in `whatwg_decode_view.hpp`:
   ```cpp
   utf_16be,
   utf_16le,
   ```
   Add after `x_mac_cyrillic` (end of enum).

2. **Create `include/beman/transcode/detail/utf16.hpp`**:
   ```cpp
   // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
   #ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP
   #define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP
   #include <beman/transcode/detail/error.hpp>
   #include <iterator>

   namespace beman::transcoding::detail {

   struct utf16_decode_result {
       char32_t codepoint{0xFFFD};
       int      bytes_consumed{0};
       bool     is_error{false};
   };

   struct utf16_encode_result {
       char bytes[4]{};
       int  count{0};
       bool is_error{false};
   };

   template <typename I, typename S>
   constexpr utf16_decode_result utf16be_decode_one(I& current, S end);

   template <typename I, typename S>
   constexpr utf16_decode_result utf16le_decode_one(I& current, S end);

   constexpr utf16_encode_result utf16be_encode_one(char32_t cp);
   constexpr utf16_encode_result utf16le_encode_one(char32_t cp);

   // ... out-of-line definitions ...

   } // namespace beman::transcoding::detail
   #endif
   ```

   Decode functions take `I& current, S end` (like `utf8_decode_one`),
   advance `current` as they consume bytes, and return the result.
   Encode functions take a `char32_t` and return bytes.

3. **Add UTF-16 decode dispatch** in `whatwg_decode_view.hpp`
   `load()` functions. The decode side already has the right shape: it
   calls e.g. `auto r = detail::utf8_decode_one(current_, end_)` and
   sets `value_`, `done_`. Add after the `x_mac_cyrillic` arm:
   ```cpp
   } else if constexpr (C == codec::utf_16be) {
       auto r = detail::utf16be_decode_one(current_, end_);
       if (r.bytes_consumed == 0) { done_ = true; return; }
       value_ = r.codepoint;
   } else if constexpr (C == codec::utf_16le) {
       auto r = detail::utf16le_decode_one(current_, end_);
       if (r.bytes_consumed == 0) { done_ = true; return; }
       value_ = r.codepoint;
   }
   ```
   Same pattern in `whatwg_decode_or_error_view` `load()`.

4. **Add UTF-16 encode dispatch** in `whatwg_encode_view.hpp`
   `load()` functions. Same buffer-pattern as UTF-8:
   ```cpp
   } else if constexpr (C == codec::utf_16be) {
       auto r = detail::utf16be_encode_one(static_cast<char32_t>(*current_));
       ++current_;
       if (r.is_error) {
           // U+FFFD in UTF-16BE: 0xFF 0xFD
           buf_[0] = '\xFF'; buf_[1] = '\xFD'; len_ = 2;
       } else {
           for (int i = 0; i < r.count; ++i) buf_[i] = r.bytes[i];
           len_ = r.count;
       }
       pos_ = 0;
   ```

5. **Add `#include <beman/transcode/detail/utf16.hpp>`** to both
   `whatwg_decode_view.hpp` and `whatwg_encode_view.hpp`.

6. **Add `detail/utf16.hpp`** to `include/beman/transcode/CMakeLists.txt`
   HEADERS FILES list.

7. **Create test files** (new executables):
   - `tests/beman/transcode/utf16_decode.test.cpp`
   - `tests/beman/transcode/utf16_encode.test.cpp`

   Register in `tests/beman/transcode/CMakeLists.txt` (same pattern as
   `whatwg_encode` / `whatwg_decode`).

8. **Test cases to write:**

   Decode:
   - ASCII codepoint via UTF-16BE (`0x00 0x41` → `U'A'`)
   - ASCII codepoint via UTF-16LE (`0x41 0x00` → `U'A'`)
   - BMP non-ASCII via UTF-16BE/LE
   - Surrogate pair → supplementary codepoint (UTF-16BE and LE)
   - Lone surrogate → U+FFFD (non-error variant)
   - Lone surrogate → `unexpected(surrogate_code_point)` (or_error variant)
   - Input with odd byte count handled gracefully
   - Pipe syntax: `span<byte> | whatwg_decode<codec::utf_16be>`
   - Consteval test

   Encode:
   - BMP codepoint to UTF-16BE (2 bytes)
   - BMP codepoint to UTF-16LE (2 bytes, swapped)
   - Supplementary codepoint to UTF-16BE (4 bytes, surrogate pair)
   - Supplementary codepoint to UTF-16LE (4 bytes, swapped)
   - Surrogate → U+FFFD bytes (non-error variant)
   - Surrogate → `unexpected(surrogate_code_point)` (or_error variant)
   - Pipe syntax: `vector<char32_t> | whatwg_encode<codec::utf_16be>`
   - Consteval test

9. **Negative compile tests** (optional but preferred):
   - `utf16_decode_reject_char32_range_fail.cpp` — `vector<char32_t>`
     should fail as input to `whatwg_decode_view<codec::utf_16be>` (not
     a `legacy_byte_range`)

### Note on `whatwg_decode_or_error_view`

The `or_error` decode view lives in the same file (`whatwg_decode_view.hpp`).
Its `load()` also needs the UTF-16 dispatch arms, yielding
`std::unexpected(whatwg_error::surrogate_code_point)` for lone surrogates.

### Note on decode iterator byte-consumption design

Look at `detail/utf8.hpp` to see the pattern for `utf8_decode_one`:
it takes `I& current, S end` by reference, advances `current` by the
bytes consumed, returns the decoded result. UTF-16 decode should follow
the exact same pattern (advance `current` by 2 for BMP, 4 for surrogate
pairs).

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Includes: angle brackets only, full path from include root
- Test files: include the header under test **twice** (idempotent check)
- Functions: out-of-line in headers with full qualification
- `constexpr` everything that can be
- License: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
make mypy      # mypy type check only
```

## TDD Process

1. Branch: `git checkout -b step21-utf16`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): add enum values, `utf16.hpp`, dispatch arms,
   update CMakeLists files
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — `utf16.hpp` should have good coverage
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- Both byte-order paths (BE and LE) should be covered.
- Both BMP and surrogate-pair paths should be covered.
- The lone-surrogate error path should be covered (or_error variant test).
- Any truly defensive unreachable lines may be skipped; note them.
