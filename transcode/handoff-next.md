# Handoff: beman.transcode — Step 23 (GB18030 decode + encode)

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

**194 C++ tests + 44 Python tests pass** (`make test`). Steps 0–22
complete. On `main`.

### What Step 22 Built

Step 22 added GBK decode and encode:

- **`tools/generate_tables.py`** — extended with `parse_gbk_index()`,
  `render_gbk_hpp()`, and `generate_gbk()`. Now generates
  `include/beman/transcode/detail/tables/gbk.hpp` (23,940-entry table)
  from `docs/whatwg/index-gb18030.txt`. New `--include-tables-dir`
  argument added to `main()`.

- **`tools/tests/test_generate.py`** — 13 new tests for GBK table
  generation (length, known entries, spot checks, render output).

- **`include/beman/transcode/detail/gbk.hpp`** — new file with
  `gbk_decode_result`, `gbk_encode_result`, `gbk_decode_one(I&, S)`,
  and `gbk_encode_one(char32_t)`. ASCII passthrough on decode; trail
  byte always consumed even when invalid.

- **`include/beman/transcode/detail/tables/gbk.hpp`** — generated
  `inline constexpr char32_t gbk[23940]`. All 23,940 GBK pointers are
  mapped (no null entries in the WHATWG table).

- **`include/beman/transcode/whatwg_decode_view.hpp`** — added
  `codec::gbk` to the enum; added `#include <beman/transcode/detail/gbk.hpp>`;
  added `gbk` dispatch arm in both `load()` functions.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — added
  `#include <beman/transcode/detail/gbk.hpp>`; added `gbk` dispatch arm
  in both `load()` functions.

- **`tests/beman/transcode/gbk_decode.test.cpp`** and
  **`tests/beman/transcode/gbk_encode.test.cpp`** — new test files.

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
    // gb18030, big5, shift_jis, euc_jp, euc_kr NOT YET — step 23+
};
```

### Key design note: GBK has no unmapped table entries

The WHATWG index-gb18030.txt has exactly 23,940 non-null entries covering
all valid GBK pointers 0–23939. The defensive `if (cp == 0)` guard in
`gbk_decode_one` is unreachable in practice for GBK. GB18030 decode uses
the same table but also handles supplementary characters via ranges (step 23).

## What To Do Next — Step 23

**Branch:** `step23-gb18030`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step23 plan file yet. Use this handoff as the
authoritative spec.

### GB18030 overview

GB18030 is a superset of GBK that adds 4-byte sequences to cover all
Unicode codepoints. The WHATWG encoding spec defines it as:

- **GBK range** (same as step 22): 1-byte ASCII + 2-byte lead/trail
- **4-byte range**: Lead byte 0x81–0xFE, followed by a decimal ASCII byte
  (0x30–0x39), another lead (0x81–0xFE), and another decimal (0x30–0x39).
  These 4-byte sequences are decoded via a range table
  (`docs/whatwg/index-gb18030-ranges.txt`).

### WHATWG GB18030 decoder algorithm

```
1. If byte is ASCII (0x00–0x7F): output it directly.
2. If byte is 0x80: output U+20AC (EURO SIGN). [GBK special case]
3. If byte is 0x81–0xFE (lead):
   a. Read next byte (second).
   b. If second is 0x30–0x39 (decimal ASCII digit):
      → 4-byte sequence. Read third (0x81–0xFE) and fourth (0x30–0x39).
      index = ((lead - 0x81) * 10 + (second - 0x30)) * 126 * 10
              + (third - 0x81) * 10 + (fourth - 0x30)
      Look up via gb18030 ranges table to get the codepoint.
   c. If second is 0x40–0xFE (not 0x7F):
      → 2-byte GBK sequence. Same as GBK decode.
   d. Otherwise: error (bad second byte).
4. Otherwise: error (invalid lead).
```

### GB18030 encoder algorithm

```
For codepoint cp:
1. If cp is ASCII: output 1 byte.
2. If cp == U+20AC: output 0x80. [GBK special case]
3. Search GBK table for cp → output 2-byte sequence.
4. If not found: encode via GB18030 ranges → output 4-byte sequence.
5. If still not found: error (unmapped).
```

### The ranges table (`docs/whatwg/index-gb18030-ranges.txt`)

This file has ~207 data entries. Each entry is a pair:
`  <pointer>  <codepoint>`

The pointer is a "linear" GB18030 4-byte pointer (0–1,274,739).
The table is used to convert between linear pointers and codepoints via
binary search + range interpolation:

- **Decode** (pointer → codepoint): find the entry where
  `ranges[i].pointer <= p < ranges[i+1].pointer`. Then:
  `cp = ranges[i].codepoint + (p - ranges[i].pointer)`
- **Encode** (codepoint → pointer): find the entry where
  `ranges[i].codepoint <= cp < ranges[i+1].codepoint`. Then:
  `p = ranges[i].pointer + (cp - ranges[i].codepoint)`

The linear pointer is related to the 4-byte sequence by:
```
p = ((b1 - 0x81) * 10 + (b2 - 0x30)) * 126 * 10
    + (b3 - 0x81) * 10 + (b4 - 0x30)
```
where b1=lead, b2=second, b3=third, b4=fourth.

To convert a linear pointer back to 4 bytes:
```
b4 = p % 10 + 0x30;   p /= 10;
b3 = p % 126 + 0x81;  p /= 126;
b2 = p % 10 + 0x30;   p /= 10;
b1 = p + 0x81;
```

### Implementation order

1. **Extend `tools/generate_tables.py`**:
   - Add `parse_gb18030_ranges(path)` → returns a list of `(pointer, codepoint)`
     tuples sorted by pointer.
   - Add `render_gb18030_ranges_hpp(ranges)` → outputs
     `include/beman/transcode/detail/tables/gb18030_ranges.hpp` with a
     `constexpr` struct array like:
     ```cpp
     struct gb18030_range { uint32_t pointer; char32_t codepoint; };
     inline constexpr gb18030_range gb18030_ranges[] = { ... };
     inline constexpr int gb18030_ranges_count = N;
     ```
   - Add `generate_gb18030_ranges(in_dir, include_tables_dir)` and call it
     from `main()`.
   - Add Python tests for the new functions.

2. **Create `include/beman/transcode/detail/gb18030.hpp`**:
   - `gb18030_decode_result` struct (same shape as `gbk_decode_result`)
   - `gb18030_encode_result` struct (same shape, but bytes[4])
   - `constexpr gb18030_decode_result gb18030_decode_one(I& current, S end)`
   - `constexpr gb18030_encode_result gb18030_encode_one(char32_t cp)`
   - Include `<beman/transcode/detail/tables/gbk.hpp>` and
     `<beman/transcode/detail/tables/gb18030_ranges.hpp>`.

3. **Add `codec::gb18030`** to the enum in `whatwg_decode_view.hpp`
   (after `codec::gbk`).

4. **Add dispatch arms** in both decode views and both encode views,
   calling `gb18030_decode_one` / `gb18030_encode_one`.

5. **Add includes** for `detail/gb18030.hpp` in both view headers.

6. **Update `include/beman/transcode/CMakeLists.txt`**: add
   `detail/gb18030.hpp` and `detail/tables/gb18030_ranges.hpp`.

7. **Test files**:
   - `tests/beman/transcode/gb18030_decode.test.cpp`
   - `tests/beman/transcode/gb18030_encode.test.cpp`
   - Register in `tests/beman/transcode/CMakeLists.txt`.

8. **Test cases to write**:

   Decode:
   - ASCII passthrough `0x41` → U+0041
   - 0x80 → U+20AC (EURO SIGN)
   - 2-byte GBK sequence `0x81 0x40` → U+4E02
   - 4-byte sequence for a supplementary codepoint (e.g. U+10000)
     `0x90 0x30 0x81 0x30` → U+10000
   - 4-byte sequence for U+1F600 (emoji) → encode then decode
   - Invalid lead → U+FFFD
   - Truncated 4-byte → U+FFFD
   - Pipe syntax + consteval

   Encode:
   - ASCII `U'A'` → `0x41`
   - `U'€'` → `0x80`
   - `U'丂'` → `0x81 0x40` (GBK 2-byte)
   - U+10000 → `0x90 0x30 0x81 0x30` (4-byte)
   - Unmapped? (GB18030 covers all Unicode so no unmapped)
   - Pipe syntax + consteval

### Note on GB18030 coverage of Unicode

GB18030 covers all 1,114,112 Unicode code points — there are no unmapped
codepoints. The encode function never returns `is_error=true` for valid
Unicode scalar values. The or_error variant can therefore be tested with
normal codepoints only.

### Note on the 4-byte U+20AC special case

The WHATWG spec says: "If byte is 0x80, return code point U+20AC." This is
a special-case fallback inherited from GBK. In 4-byte mode, U+20AC is
encoded as `0xA2 0xE3` (2-byte GBK), NOT as `0x80`. Only decoding of
the byte `0x80` triggers this special case.

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

1. Branch: `git checkout -b step23-gb18030`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): ranges table generation + gb18030.hpp + dispatch arms
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — both paths (2-byte GBK and 4-byte range) covered
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- ASCII passthrough path should be covered.
- 2-byte GBK path should be covered.
- 4-byte range path should be covered.
- The 0x80 → U+20AC special case should be covered.
- All GBK pointers and all range entries don't need individual tests —
  spot-check a representative sample.
