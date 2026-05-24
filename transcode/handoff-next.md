# Handoff: beman.transcode — Step 26 (EUC-JP decode + encode)

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

**289 C++ tests + 80 Python tests pass** (`make test`). Steps 0–25
complete. On `main`.

### What Step 25 Built

Step 25 added Shift_JIS decode and encode:

- **`tools/generate_tables.py`** — extended with `SHIFTJIS_POINTER_COUNT`,
  `SHIFTJIS_LEAD_COUNT`, `SHIFTJIS_TRAIL_COUNT`, `parse_shift_jis_index()`,
  `render_shift_jis_hpp()`, and `generate_shift_jis()`. Called from `main()`.

- **`include/beman/transcode/detail/tables/shift_jis.hpp`** — generated
  `inline constexpr char32_t shift_jis[11280]`. Source: `index-jis0208.txt`.
  Pointer formula:
  ```
  lead bytes: 0x81-0x9F (offset 0-30) + 0xE0-0xFC (offset 31-59) = 60 values
  trail bytes: 0x40-0x7E (offset 0-62) + 0x80-0xFC (offset 63-187) = 188 values
  pointer = lead_offset * 188 + trail_offset
  ```

- **`include/beman/transcode/detail/shift_jis.hpp`** — `shift_jis_decode_result`,
  `shift_jis_encode_result`, `shift_jis_decode_one()`, `shift_jis_encode_one()`.
  Special cases: ASCII (0x00-0x7F) passthrough, half-width katakana (0xA1-0xDF
  → U+FF61-U+FF9F), U+00A5 → 0x5C, U+203E → 0x7E.

- **`include/beman/transcode/whatwg_decode_view.hpp`** — added `codec::shift_jis`
  to enum; added Shift_JIS dispatch arms in both `load()` functions.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — added Shift_JIS
  dispatch arms in both `load()` functions.

### Current codec enum (in `whatwg_decode_view.hpp`)

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
    big5,
    shift_jis,
    // euc_jp, euc_kr NOT YET — step 26+
};
```

### IMPORTANT: The `index-jis0208.txt` table is SHARED

The Shift_JIS table (`tables::shift_jis[11280]`) was generated from
`index-jis0208.txt` using the Shift_JIS pointer formula. **EUC-JP also
uses `index-jis0208.txt`** — but with a different pointer formula:

- **EUC-JP jis0208 pointer** = `(lead - 0xA1) * 94 + (trail - 0xA1)`
  - Lead: 0xA1-0xFE (94 values), Trail: 0xA1-0xFE (94 values)
  - Max pointer: 93 × 94 + 93 = 8835 (so table size 8836)

The codepoints at each pointer are the **same** in both tables. The
Shift_JIS table (`shift_jis[11280]`) already contains all pointers
0–8835 (EUC-JP jis0208 range) as its first 8836 entries. Therefore:

**EUC-JP decode/encode for jis0208 can reuse `tables::shift_jis`** —
just use the EUC-JP pointer formula and index into `tables::shift_jis`.

Verified:
- jis0208 pointer 0 → U+3000 → EUC-JP bytes: 0xA1, 0xA1
- jis0208 pointer 1485 → U+4E00 (一) → EUC-JP bytes: 0xB0, 0xEC

### EUC-JP also needs a NEW table: jis0212

EUC-JP's 3-byte sequence (0x8F lead) uses `index-jis0212.txt`:
- Lead: 0x8F (prefix), then 0xA1-0xFE, then 0xA1-0xFE
- JIS X 0212 pointer = `(b1 - 0xA1) * 94 + (b2 - 0xA1)`
- Max pointer from data: 7210; table size: 8836 entries
- Non-null entries start at pointer 108

Known entry: pointer 108 → U+02D8 (BREVE) → EUC-JP: 0x8F, 0xA2, 0xAF

## What To Do Next — Step 26

**Branch:** `step26-euc-jp`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step26 plan file. Use this handoff as the
authoritative spec.

### WHATWG EUC-JP decode algorithm

Source: https://encoding.spec.whatwg.org/#euc-jp-decoder

EUC-JP byte sequences:
1. `0x00–0x7F`: emit byte as codepoint (ASCII passthrough, 1 byte).
2. `0x8E` (SS2 prefix):
   - Read next byte. If in `0xA1–0xDF`: emit `U+FF61 + byte - 0xA1`
     (half-width katakana, 2 bytes consumed).
   - Otherwise: error (U+FFFD non-error, or `invalid_byte`).
3. `0x8F` (SS3 prefix, JIS X 0212):
   - Read two more bytes (b1, b2). If both in `0xA1–0xFE`:
     pointer = `(b1 - 0xA1) * 94 + (b2 - 0xA1)`; look up `tables::euc_jp_jis0212`.
   - If table entry is 0: error.
   - Otherwise: emit codepoint (3 bytes consumed).
   - If b1 or b2 out of range: error.
4. `0xA1–0xFE` (JIS X 0208):
   - Read next byte. If in `0xA1–0xFE`:
     pointer = `(lead - 0xA1) * 94 + (trail - 0xA1)`; look up `tables::shift_jis[pointer]`.
   - If table entry is 0: error.
   - Otherwise: emit codepoint (2 bytes consumed).
   - If trail out of range: error.
5. Any other byte: error (U+FFFD / `invalid_byte`).

### WHATWG EUC-JP encode algorithm

1. If `cp < 0x80`: emit 1 byte (ASCII).
2. If `cp == U+00A5`: emit 0x5C (1 byte).
3. If `cp == U+203E`: emit 0x7E (1 byte).
4. If `cp` in `U+FF61–U+FF9F`: emit 2 bytes: `0x8E, 0xA1 + cp - U+FF61`.
5. Search `tables::shift_jis[0..8835]` for first pointer P where
   `tables::shift_jis[P] == cp`:
   - `lead = (P / 94) + 0xA1`
   - `trail = (P % 94) + 0xA1`
   - Emit 2 bytes: lead, trail.
6. Search `tables::euc_jp_jis0212[0..8835]` for first pointer P where
   `tables::euc_jp_jis0212[P] == cp`:
   - `b1 = (P / 94) + 0xA1`
   - `b2 = (P % 94) + 0xA1`
   - Emit 3 bytes: 0x8F, b1, b2.
7. If not found: error (`'?'` or `unmapped_codepoint`).

### Table structure

**jis0208 for EUC-JP:** reuse `tables::shift_jis`. The EUC-JP pointer
formula accesses only indices 0–8835 — all within the existing array.

**jis0212:** needs a new generated table:
- Index file: `docs/whatwg/index-jis0212.txt`
- Non-null entries: pointer 108 → 7210 (6067 entries), rest are 0
- Table size: 8836 entries (pointer = (b1-0xA1)*94 + (b2-0xA1))
- Guard: `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_JP_JIS0212_HPP`
- Array: `inline constexpr char32_t euc_jp_jis0212[8836] = { ... };`
- Output: `include/beman/transcode/detail/tables/euc_jp_jis0212.hpp`

### Implementation order

1. **Extend `tools/generate_tables.py`**:
   - Add `EUC_JP_JIS0212_POINTER_COUNT = 8836` constant.
   - Add `parse_euc_jp_jis0212_index(path)` → 8836-entry list.
     Parse `index-jis0212.txt` (same format as jis0208).
   - Add `render_euc_jp_jis0212_hpp(table)` → header with guard
     `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_JP_JIS0212_HPP` and
     array `euc_jp_jis0212[8836]`.
   - Add `generate_euc_jp_jis0212(in_dir, out_dir)` called from `main()`.
   - Add Python tests for the new functions.

2. **Create `include/beman/transcode/detail/euc_jp.hpp`**:
   ```cpp
   struct euc_jp_decode_result {
       char32_t     code_point{0xFFFD};
       whatwg_error error{};
       bool         is_error{false};
   };
   struct euc_jp_encode_result {
       unsigned char bytes[3]{};  // up to 3 bytes (0x8F + two trail bytes)
       int           count{0};
       bool          is_error{false};
   };
   template <std::input_iterator I, std::sentinel_for<I> S>
   constexpr euc_jp_decode_result euc_jp_decode_one(I& current, S end);
   constexpr euc_jp_encode_result euc_jp_encode_one(char32_t cp);
   ```
   Include `<beman/transcode/detail/tables/shift_jis.hpp>` and
   `<beman/transcode/detail/tables/euc_jp_jis0212.hpp>`.

3. **Add `codec::euc_jp`** to the enum in `whatwg_decode_view.hpp`
   (after `shift_jis`).

4. **Add EUC-JP dispatch arms** in both decode `load()` functions and
   both encode `load()` functions.

5. **Add `#include <beman/transcode/detail/euc_jp.hpp>`** to both
   view headers.

6. **Update `include/beman/transcode/CMakeLists.txt`**: add
   `detail/euc_jp.hpp` and `detail/tables/euc_jp_jis0212.hpp`.

7. **Create test files**:
   - `tests/beman/transcode/euc_jp_decode.test.cpp`
   - `tests/beman/transcode/euc_jp_encode.test.cpp`
   - Register in `tests/beman/transcode/CMakeLists.txt`.

8. **Test cases to write**:

   Decode:
   - ASCII passthrough `0x41` → U'A'
   - JIS X 0208 multi-byte: `0xA1 0xA1` → U+3000 (pointer 0)
   - JIS X 0208 multi-byte: `0xB0 0xEC` → U+4E00 (一)
   - Half-width katakana: `0x8E 0xA1` → U+FF61
   - JIS X 0212 three-byte: `0x8F 0xA2 0xAF` → U+02D8 (pointer 108)
   - Invalid lead byte → U+FFFD
   - Bad trail byte → U+FFFD
   - Truncated sequences → U+FFFD / error
   - or_error variants
   - Pipe syntax + consteval

   Encode:
   - ASCII → 1 byte
   - U+3000 → 0xA1 0xA1 (2 bytes, jis0208)
   - U+4E00 (一) → 0xB0 0xEC (2 bytes, jis0208)
   - U+FF61 → 0x8E 0xA1 (2 bytes, half-width katakana)
   - U+02D8 (BREVE) → 0x8F 0xA2 0xAF (3 bytes, jis0212)
   - Unmapped → `'?'` / `unmapped_codepoint`
   - Pipe syntax + consteval

### Note on `euc_jp_encode_one` buffer size

The `euc_jp_encode_result.bytes` field must be at least 3 bytes to hold
the JIS X 0212 sequence (0x8F + b1 + b2). The `whatwg_encode_view`
buffer `buf_[4]` is already large enough.

### Note on reusing `tables::shift_jis` for jis0208 lookup

When searching for a codepoint to encode (step 5 above), limit the
search to indices 0–8835 of `tables::shift_jis`, not 0–11279. This
avoids producing byte sequences that are only valid in Shift_JIS but
not in EUC-JP.

```cpp
for (int i = 0; i < 8836; ++i) {
    if (tables::shift_jis[i] == cp) {
        // found in jis0208 range
    }
}
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

1. Branch: `git checkout -b step26-euc-jp`
2. Write failing tests (RED): tests reference `codec::euc_jp` (compile
   failure since enum value missing) + Python tooling tests
3. Commit RED → push both remotes
4. Implement (GREEN): generate jis0212 table, create `euc_jp.hpp`,
   add enum value, dispatch arms, CMakeLists updates
5. `make test` (all pass) + `make lint` (clean) → commit → push both
6. `make coverage` — ASCII, jis0208, jis0212, half-width katakana, and
   error paths should all be covered
7. Merge to main + push both
8. Mark checklist `[x]`

## Coverage Notes

- ASCII passthrough covered by encode + decode ASCII tests.
- jis0208 2-byte path (0xA1-0xFE lead) should be covered.
- Half-width katakana path (0x8E prefix) should be covered.
- jis0212 3-byte path (0x8F prefix) should be covered.
- Error paths (invalid lead, truncated sequences) should be covered.
- The defensive `if (current == end)` at top of decode is unreachable
  from the view; ~96% line coverage is acceptable.
