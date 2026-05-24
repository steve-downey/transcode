# Handoff: beman.transcode — Step 24 (Big5 decode + encode)

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

**223 C++ tests + 56 Python tests pass** (`make test`). Steps 0–23
complete. On `main`.

### What Step 23 Built

Step 23 added GB18030 decode and encode:

- **`tools/generate_tables.py`** — extended with `parse_gb18030_ranges()`,
  `render_gb18030_ranges_hpp()`, and `generate_gb18030_ranges()`. The
  ranges table has 207 entries and is called from `main()`.

- **`include/beman/transcode/detail/tables/gb18030_ranges.hpp`** —
  generated struct array `gb18030_range { uint32_t pointer; char32_t codepoint; }`
  with 207 entries. Also exports `gb18030_ranges_count = 207`.

- **`include/beman/transcode/detail/gb18030.hpp`** — new file with
  `gb18030_decode_result`, `gb18030_encode_result`, `gb18030_decode_one()`,
  and `gb18030_encode_one()`. Binary search helpers `gb18030_ranges_decode()`
  and `gb18030_ranges_encode()` live in the `detail` namespace.
  ASCII passthrough, 0x80→U+20AC special case, 2-byte GBK, 4-byte range.

- **`include/beman/transcode/whatwg_decode_view.hpp`** — added
  `codec::gb18030` to the enum; added include and dispatch arm in both
  `load()` functions.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — added include and
  dispatch arm in both `load()` functions. GB18030 covers all Unicode so
  the non-error encode arm never sets `is_error`.

- **`tests/beman/transcode/gb18030_decode.test.cpp`** and
  **`tests/beman/transcode/gb18030_encode.test.cpp`** — new test files.

### Codec enum (current, in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8,
    replacement,
    x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    utf_16be,
    utf_16le,
    gbk,
    gb18030,
    // big5, shift_jis, euc_jp, euc_kr NOT YET — step 24+
};
```

### Current decode iterator design (IMPORTANT for step 24)

`whatwg_decode_view::iterator` (and `whatwg_decode_or_error_view::iterator`)
each hold:

```cpp
base_iter     current_;
base_sent     end_;
char32_t      value_{};
bool          done_{false};
bool          has_pending_{false};   // bytes for UTF-16 only
unsigned char pending_[2]{};        // bytes for UTF-16 only
```

The `has_pending_` / `pending_[2]` mechanism stores raw bytes for the
UTF-16 case where a bad low-surrogate causes us to re-read 2 bytes. It
is NOT a general pending-codepoint buffer.

**Big5 requires a pending-codepoint buffer** (see WHATWG 2-codepoint section
below). You will need to add to both decode iterators:

```cpp
char32_t pending_cp_{};
bool     has_pending_cp_{false};
```

At the top of `load()`, before any codec dispatch, add:

```cpp
if (has_pending_cp_) {
    value_ = pending_cp_;
    has_pending_cp_ = false;
    return;
}
```

(Same pattern for the or_error variant, assigning `value_ = pending_cp_;`.)

## What To Do Next — Step 24

**Branch:** `step24-big5`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step24 plan file yet. Use this handoff as the
authoritative spec.

### The Big5 index file

`docs/whatwg/index-big5.txt` contains 18,590 data entries for pointers
0–19781 (19782 total slots = 126 leads × 157 trails). Unmapped pointers
are absent from the file (represent null entries in the table).

Lead range: 0x81–0xFE (126 values).
Trail range: 0x40–0x7E (63 values) + 0xA1–0xFE (94 values) = 157 values.

Pointer formula:
```
offset = (trail < 0xA1) ? trail - 0x40 : trail - 0x62
pointer = (lead - 0x81) * 157 + offset
```

### WHATWG Big5 decoder algorithm

```
1. If byte < 0x80: output directly (ASCII passthrough).
2. If byte is in 0x81–0xFE: it is a lead byte. Read next byte (trail).
3. Compute offset:
   - trail 0x40–0x7E: offset = trail - 0x40
   - trail 0xA1–0xFE: offset = trail - 0x62
   - otherwise: error (return U+FFFD or invalid_byte)
4. pointer = (lead - 0x81) * 157 + offset
5. SPECIAL CASES (before table lookup):
   - pointer == 1133 → output U+00CA then U+0304 (two codepoints)
   - pointer == 1135 → output U+00CA then U+030C (two codepoints)
   - pointer == 1164 → output U+00EA then U+0304 (two codepoints)
   - pointer == 1165 → output U+00EA then U+030C (two codepoints)
6. Look up tables::big5[pointer]. If null (== 0): error (U+FFFD or error).
7. Otherwise: output the codepoint.
```

The 2-codepoint cases (step 5) require the pending-codepoint buffer
described above. Store the first codepoint in `value_`, the second in
`pending_cp_`, and set `has_pending_cp_ = true`.

For the or_error variant, 2-codepoint cases always succeed (no error).

### WHATWG Big5 encoder algorithm

Big5 encode is stateful in the full WHATWG spec (it peeks ahead for the
combining-mark following U+00CA / U+00EA). **For our one-at-a-time design,
use a simplified approach**: search the table for the codepoint, use the
lowest-pointer match. If not found, return is_error=true.

The 4 special 2-codepoint sequences are not individually encodable via the
table (U+0304, U+030C are not in the Big5 table). In the non-error variant,
emit `'?'` for unmapped codepoints. In the or_error variant, emit
`unmapped_codepoint` error.

Note: U+00CA and U+00EA ARE in the Big5 table as individual codepoints.
Encode them normally.

The table search for encode must pick the LOWEST pointer for each codepoint
(some codepoints appear more than once). This mirrors what GBK does: linear
scan, pick first match.

### Implementation order

1. **Extend `tools/generate_tables.py`**:
   - Add `parse_big5_index(path)` → list of 19782 `int` entries (0 = null).
     Similar to `parse_gbk_index()` but with 19782 slots. Entries in the
     file may have a tab-separated third column (character) — ignore it.
   - Add `render_big5_hpp(table)` → C++ header
     `include/beman/transcode/detail/tables/big5.hpp`.
     Guard: `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_BIG5_HPP`.
     Array: `inline constexpr char32_t big5[19782] = { ... };`
   - Add `generate_big5(in_dir, out_dir)` and call from `main()`.
   - Add Python tests for the new functions.

2. **Create `include/beman/transcode/detail/big5.hpp`**:
   ```cpp
   struct big5_decode_result {
       char32_t     code_point{0xFFFD};
       char32_t     code_point2{0};  // non-zero for 2-codepoint results
       whatwg_error error{};
       bool         is_error{false};
   };
   struct big5_encode_result {
       unsigned char bytes[2]{};
       int           count{0};
       bool          is_error{false};
   };
   template <std::input_iterator I, std::sentinel_for<I> S>
   constexpr big5_decode_result big5_decode_one(I& current, S end);
   constexpr big5_encode_result big5_encode_one(char32_t cp);
   ```
   The decode function returns `code_point2 != 0` to signal 2 codepoints.

3. **Add `codec::big5`** to the enum in `whatwg_decode_view.hpp` (after `gb18030`).

4. **Add pending-codepoint fields** to both decode iterators (see above):
   ```cpp
   char32_t pending_cp_{};
   bool     has_pending_cp_{false};
   ```
   Add at-top-of-`load()` check (before any `if constexpr` dispatch).

5. **Add Big5 dispatch arms** in `whatwg_decode_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::big5) {
       auto r = detail::big5_decode_one(current_, end_);
       if (r.is_error) {
           value_ = U'\xFFFD';
       } else {
           value_ = r.code_point;
           if (r.code_point2 != 0) {
               pending_cp_      = r.code_point2;
               has_pending_cp_  = true;
           }
       }
   }
   ```
   And in `whatwg_decode_or_error_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::big5) {
       auto r = detail::big5_decode_one(current_, end_);
       if (r.is_error) {
           value_ = std::unexpected(r.error);
       } else {
           value_ = r.code_point;
           if (r.code_point2 != 0) {
               pending_cp_     = r.code_point2;
               has_pending_cp_ = true;
           }
       }
   }
   ```

6. **Add Big5 encode dispatch arms** in both encode views (same pattern as GBK).

7. **Add `#include <beman/transcode/detail/big5.hpp>`** to both view headers.

8. **Update `include/beman/transcode/CMakeLists.txt`**: add `detail/big5.hpp`
   and `detail/tables/big5.hpp`.

9. **Create test files**:
   - `tests/beman/transcode/big5_decode.test.cpp`
   - `tests/beman/transcode/big5_encode.test.cpp`
   - Register in `tests/beman/transcode/CMakeLists.txt`.

10. **Test cases to write**:

    Decode:
    - ASCII passthrough `0x41` → U'A'
    - Valid 2-byte `0xA4 0x40` → U+4E00 (一)
    - 2-codepoint special: `0xA2 0xCC` (pointer 1133) → U+00CA + U+0304
    - 2-codepoint special: `0xA2 0xCE` (pointer 1135) → U+00CA + U+030C
    - Null table entry → U+FFFD
    - Bad trail byte → U+FFFD
    - or_error: valid decode → value
    - or_error: error → `unexpected(invalid_byte)`
    - or_error: 2-codepoint special → two successive values
    - Pipe syntax + consteval

    Encode:
    - ASCII U'A' → `0x41`
    - U+4E00 → Big5 bytes
    - Unmapped → '?' (non-error) or `unexpected(unmapped_codepoint)` (or_error)
    - Pipe syntax + consteval

### Verifying the pointer formula for known entries

`0xA4 0x40`: lead=0xA4, trail=0x40 → offset = 0x40-0x40 = 0 →
pointer = (0xA4-0x81)*157 + 0 = 35*157 = 5495. Look up index 5495 in table.

Verify: `grep "^[[:space:]]*5495[[:space:]]" docs/whatwg/index-big5.txt`
should give U+4E00 (一).

### Pointer-to-bytes inverse (for encode)

```
pointer = (lead - 0x81) * 157 + offset
lead    = pointer / 157 + 0x81
offset  = pointer % 157
trail   = (offset < 63) ? offset + 0x40 : offset + 0x62
```

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

1. Branch: `git checkout -b step24-big5`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): table generation + big5.hpp + pending-codepoint buffer +
   dispatch arms + CMakeLists updates
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — ASCII, 2-byte, null-entry, 2-codepoint paths all covered
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- ASCII passthrough path should be covered.
- Valid 2-byte decode path should be covered.
- Null table entry error path should be covered.
- 2-codepoint special case (all 4 pointers or at least 1133 and 1164) should be covered.
- Encode: valid table lookup and unmapped paths should be covered.
