# Handoff: beman.transcode — Step 25 (Shift_JIS decode + encode)

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

**254 C++ tests + 68 Python tests pass** (`make test`). Steps 0–24
complete. On `main`.

### What Step 24 Built

Step 24 added Big5 decode and encode:

- **`tools/generate_tables.py`** — extended with `BIG5_POINTER_COUNT`,
  `parse_big5_index()`, `render_big5_hpp()`, and `generate_big5()`.
  Called from `main()`.

- **`include/beman/transcode/detail/tables/big5.hpp`** — generated
  `inline constexpr char32_t big5[19782]`. Pointer formula:
  ```
  lead: 0x81–0xFE (126 values)
  trail: 0x40–0x7E (offset 0–62) + 0xA1–0xFE (offset 63–156) = 157 values
  pointer = (lead - 0x81) * 157 + offset
  ```
  Four special pointers (1133, 1135, 1164, 1165) yield two codepoints each.
  They are null in the table but intercepted before table lookup.

- **`include/beman/transcode/detail/big5.hpp`** — `big5_decode_result`
  (with `code_point2` field for 2-codepoint results), `big5_encode_result`,
  `big5_decode_one()`, `big5_encode_one()`.

- **`include/beman/transcode/whatwg_decode_view.hpp`** — added
  `codec::big5` to enum; added `pending_cp_` / `has_pending_cp_` fields
  to both decode iterators; added pending-codepoint check at top of both
  `load()` functions; added Big5 dispatch arms.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — added Big5
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
    // shift_jis, euc_jp, euc_kr NOT YET — step 25+
};
```

### Current decode iterator design (IMPORTANT for step 25)

Both decode iterators now hold:

```cpp
base_iter     current_;
base_sent     end_;
char32_t      value_{};
bool          done_{false};
bool          has_pending_{false};   // raw bytes pending (UTF-16 only)
unsigned char pending_[2]{};        // raw bytes pending (UTF-16 only)
char32_t      pending_cp_{};        // codepoint pending (Big5 only)
bool          has_pending_cp_{false};
```

The `pending_cp_` mechanism is checked at the **very top** of `load()`,
before any EOF check or codec dispatch. This is already in place for Big5;
Shift_JIS does NOT need it (Shift_JIS is always 1-codepoint-per-call).

## What To Do Next — Step 25

**Branch:** `step25-shift-jis`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step25 plan file. Use this handoff as the
authoritative spec.

### WHATWG Shift_JIS decode algorithm

Source: https://encoding.spec.whatwg.org/#shift_jis-decoder

**Single-byte paths (1 byte consumed):**
1. `0x00–0x7F`: emit byte as codepoint directly (ASCII passthrough).
2. `0xA1–0xDF`: emit `U+FF61 + byte - 0xA1` (half-width katakana).
3. Any other byte not in lead range (`0x80`, `0xA0`, `0xFD–0xFF`): emit
   U+FFFD (non-error) or `invalid_byte` error.

**Multi-byte path (lead byte `0x81–0x9F` or `0xE0–0xFC`, 2 bytes consumed):**
1. Read trail byte.
2. If trail is not in `0x40–0x7E` or `0x80–0xFC`: emit U+FFFD / error.
3. Compute:
   ```
   lead_offset  = (lead <= 0x9F) ? lead - 0x81 : lead - 0xC1
   trail_offset = (trail < 0x7F) ? trail - 0x40 : trail - 0x41
   pointer      = lead_offset * 188 + trail_offset
   ```
4. Look up `tables::shift_jis[pointer]`. If zero: U+FFFD / error.
5. Otherwise: emit codepoint.

If end of input after reading lead (no trail): truncated sequence error.

### WHATWG Shift_JIS encode algorithm

1. If `cp` < 0x80: emit 1 byte (ASCII).
2. If `cp` == U+00A5: emit 0x5C (YEN SIGN).
3. If `cp` == U+203E: emit 0x7E (OVERLINE).
4. If `cp` is in `U+FF61–U+FF9F` (half-width katakana):
   emit `0xA1 + cp - U+FF61` (1 byte).
5. Otherwise: search `tables::shift_jis` for first pointer that maps to `cp`.
   - Convert pointer to bytes:
     ```
     lead_offset  = pointer / 188
     trail_offset = pointer % 188
     lead  = (lead_offset < 0x1F) ? lead_offset + 0x81 : lead_offset + 0xC1
     trail = (trail_offset < 63)  ? trail_offset + 0x40 : trail_offset + 0x41
     ```
   - If trail would be `0x7F`: skip this pointer (continue search). There
     are no valid trail offsets that map to 0x7F.
   - Emit 2 bytes.
6. If not found: error (U+FFFD replacement byte `0x3F` = `'?'`, or
   `unmapped_codepoint` in the or_error variant).

### Table structure

Index file: `docs/whatwg/index-jis0208.txt`

- 7724 non-null entries, max pointer value 11103.
- Table size: `60 * 188 = 11280` slots (60 lead offsets × 188 trail offsets).
- Guard: `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_SHIFT_JIS_HPP`
- Array: `inline constexpr char32_t shift_jis[11280] = { ... };`

Verify pointer formula for a known entry: pointer 0 should be U+3000
(IDEOGRAPHIC SPACE):
```
grep "^[[:space:]]*0[[:space:]]" docs/whatwg/index-jis0208.txt
```

Inverse for encode: given pointer P:
- `lead_offset = P / 188`
- `trail_offset = P % 188`
- `lead  = (lead_offset < 0x1F) ? lead_offset + 0x81 : lead_offset + 0xC1`
- `trail = (trail_offset < 63)  ? trail_offset + 0x40 : trail_offset + 0x41`
- Skip if trail == 0x7F (but this cannot happen given the valid range of trail_offset).

### Implementation order

1. **Extend `tools/generate_tables.py`**:
   - Add `SHIFTJIS_POINTER_COUNT = 11280` constant.
   - Add `parse_shift_jis_index(path)` → 11280-entry list. Parse
     `index-jis0208.txt` (same format as `index-gb18030.txt` and
     `index-big5.txt`). Pointers outside 0–11279 are ignored.
   - Add `render_shift_jis_hpp(table)` → header with guard
     `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_SHIFT_JIS_HPP` and array
     `shift_jis[11280]`.
   - Add `generate_shift_jis(in_dir, out_dir)` and call from `main()`.
   - Add Python tests for the new functions.

2. **Create `include/beman/transcode/detail/shift_jis.hpp`**:
   ```cpp
   struct shift_jis_decode_result {
       char32_t     code_point{0xFFFD};
       whatwg_error error{};
       bool         is_error{false};
   };
   struct shift_jis_encode_result {
       unsigned char bytes[2]{};
       int           count{0};
       bool          is_error{false};
   };
   template <std::input_iterator I, std::sentinel_for<I> S>
   constexpr shift_jis_decode_result shift_jis_decode_one(I& current, S end);
   constexpr shift_jis_encode_result shift_jis_encode_one(char32_t cp);
   ```
   No `code_point2` field — Shift_JIS is always 1-codepoint-per-call.

3. **Add `codec::shift_jis`** to the enum in `whatwg_decode_view.hpp`
   (after `big5`).

4. **Add Shift_JIS dispatch arms** in both decode `load()` functions and
   both encode `load()` functions (same single-result pattern as GBK).

5. **Add `#include <beman/transcode/detail/shift_jis.hpp>`** to both
   view headers.

6. **Update `include/beman/transcode/CMakeLists.txt`**: add
   `detail/shift_jis.hpp` and `detail/tables/shift_jis.hpp`.

7. **Create test files**:
   - `tests/beman/transcode/shift_jis_decode.test.cpp`
   - `tests/beman/transcode/shift_jis_encode.test.cpp`
   - Register in `tests/beman/transcode/CMakeLists.txt`.

8. **Test cases to write**:

   Decode:
   - ASCII passthrough `0x41` → U'A'
   - Half-width katakana `0xA1` → U+FF61 (HALFWIDTH IDEOGRAPHIC FULL STOP)
   - Half-width katakana `0xDF` → U+FF9F (HALFWIDTH KATAKANA VOICED ITERATION MARK)
   - Multi-byte: verify pointer 0 (look up byte sequence) → U+3000
   - Multi-byte: known CJK character (e.g. pointer for 一)
   - Invalid lead byte `0x80` → U+FFFD
   - Bad trail byte → U+FFFD
   - Truncated lead byte → U+FFFD
   - or_error variants for each error case
   - Pipe syntax + consteval

   Encode:
   - ASCII → 1 byte
   - U+FF61 → `0xA1` (half-width katakana)
   - U+3000 → 2-byte Shift_JIS bytes (pointer 0)
   - Unmapped → `'?'` / `unmapped_codepoint`
   - U+00A5 → `0x5C` (yen sign special case)
   - U+203E → `0x7E` (overline special case)
   - Pipe syntax + consteval

### Verifying the pointer formula for known entries

```bash
# pointer 0: lead_offset=0, trail_offset=0
# lead = 0 + 0x81 = 0x81
# trail = 0 + 0x40 = 0x40
# bytes: 0x81 0x40 → U+3000 (IDEOGRAPHIC SPACE)

grep "^[[:space:]]*0[[:space:]]" docs/whatwg/index-jis0208.txt
```

### Note on `0x7F` trail byte

The valid trail ranges (0x40–0x7E and 0x80–0xFC) exclude 0x7F. In the
decode direction, 0x7F as a trail byte is an error. In the encode
direction, the inverse formula can never produce 0x7F because:
- offset 0–62 maps to 0x40–0x7E (max 0x7E, not 0x7F)
- offset 63–187 maps to 0x80–0xFC (min 0x80, not 0x7F)
So no guard against 0x7F is needed in `shift_jis_encode_one`.

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

1. Branch: `git checkout -b step25-shift-jis`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): table generation + shift_jis.hpp + dispatch arms +
   CMakeLists updates
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — ASCII, half-width katakana, multi-byte, error paths covered
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- ASCII passthrough path should be covered.
- Half-width katakana path (0xA1–0xDF) should be covered.
- Valid multi-byte path should be covered.
- Invalid lead byte path should be covered.
- Bad trail byte error path should be covered.
- Encode: all 4 paths (ASCII, katakana, 2-byte table, unmapped) should be covered.
- Defensive empty-range check at function entry (line 1 of decode function)
  is unreachable from the view; a ~96% coverage result is fine.
