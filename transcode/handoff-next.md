# Handoff: beman.transcode — Step 22 (GBK decode + encode)

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

**167 C++ tests + 33 Python tests pass** (`make test`). Steps 0–21
complete. On `main`.

### What Step 21 Built

Step 21 added UTF-16BE and UTF-16LE decode and encode:

- **`include/beman/transcode/detail/utf16.hpp`** — new file with
  `utf16_encode_result`, `utf16be_encode_one(char32_t)`, and
  `utf16le_encode_one(char32_t)`. Also has decode result/function
  declarations and definitions (used only from encode dispatch; decode
  is now inlined in the view iterators).

- **`include/beman/transcode/whatwg_decode_view.hpp`** — added
  `utf_16be` and `utf_16le` to the `codec` enum; added
  `has_pending_` / `pending_[2]` fields to both
  `whatwg_decode_view::iterator` and
  `whatwg_decode_or_error_view::iterator`; updated the initial
  `done_` check to `if (!has_pending_ && current_ == end_)` so
  pending bytes survive end-of-input; added combined
  `utf_16be || utf_16le` dispatch arms that inline all surrogate-pair
  logic with pending-buffer support.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — added
  `utf_16be` and `utf_16le` dispatch arms in both `load()` functions,
  calling `utf16be_encode_one` / `utf16le_encode_one`. Surrogates
  emit U+FFFD bytes (non-error) or
  `unexpected(surrogate_code_point)` (or_error).

- **`tests/beman/transcode/utf16_decode.test.cpp`** and
  **`tests/beman/transcode/utf16_encode.test.cpp`** — new test files.

- **`tests/beman/transcode/utf16_decode_reject_char32_range_fail.cpp`**
  — negative compile test confirming `vector<char32_t>` is rejected
  as input to `whatwg_decode_view<codec::utf_16be>`.

### Key design decision for UTF-16 decode

Input iterators cannot retreat. When a high surrogate is followed by
a non-low-surrogate code unit, the decode view saves the two bytes of
the "bad" code unit in a 2-byte `pending_[]` buffer and re-processes
them on the next `load()` call. This is why both iterator types now
carry `has_pending_` and `pending_[2]` fields.

### Codec enum (current, in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8,
    replacement,
    x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    utf_16be,
    utf_16le,
    // gbk, gb18030, big5, shift_jis, euc_jp, euc_kr NOT YET — step 22+
};
```

## What To Do Next — Step 22

**Branch:** `step22-gbk`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step22 plan file yet. Use this handoff as the
authoritative spec.

### GBK overview

GBK is a Chinese multi-byte encoding. The WHATWG encoding spec defines
it as an extension of GB 2312. There is a WHATWG index file
(`docs/whatwg/index-gb18030.txt`) that covers both GBK and GB 18030
mappings.

GBK decode uses a 2-byte lead-trail scheme:
- Lead byte: 0x81–0xFE
- Trail byte: 0x40–0xFE (excluding 0x7F)
- The index is: `(lead - 0x81) * 190 + (trail - 0x40) - (trail > 0x7F ? 1 : 0)`
- Total entries: 126 leads × 190 trail positions = 23,940 entries
- The WHATWG index maps these positions to Unicode codepoints (or null
  for unmapped).

GBK encode: for a codepoint, find its position in the index table and
convert back to lead/trail bytes.

### Data tooling

The existing `tools/generate_tables.py` already downloads and processes
WHATWG index files. The GBK table needs to be generated from
`docs/whatwg/index-gb18030.txt` (the same file used for GB 18030).

**Step 22 is primarily a data-tooling step:**

1. Extend `tools/generate_tables.py` to generate a GBK table:
   - Output: `include/beman/transcode/detail/tables/gbk.hpp`
   - Format: a `constexpr char32_t gbk_decode[23940]` array, where
     index is `(lead-0x81)*190 + (trail-0x40) - (trail>0x7F?1:0)`.
   - Unmapped entries should be `0` (or a sentinel like `U'�'`
     that the decode function can detect).
   - Add a Python test in `tools/tests/test_generate.py` to verify
     the table is generated correctly (spot-check a few known
     mappings from the WHATWG spec, e.g. 0x81 0x40 → U+4E02).

2. **Create `include/beman/transcode/detail/gbk.hpp`**:
   - `gbk_decode_result` struct
   - `constexpr gbk_decode_result gbk_decode_one(I& current, S end)`
   - `gbk_encode_result` struct
   - `constexpr gbk_encode_result gbk_encode_one(char32_t cp)`
   - Pattern: similar to `detail/single_byte.hpp` but 2-byte.

3. **Add `codec::gbk`** to the enum in `whatwg_decode_view.hpp`.

4. **Add dispatch arms** in both decode views and both encode views.

5. **Test files**:
   - `tests/beman/transcode/gbk_decode.test.cpp`
   - `tests/beman/transcode/gbk_encode.test.cpp`

6. **CMakeLists.txt** updates (include list + test registration).

### WHATWG GBK decode algorithm

From the WHATWG Encoding spec (§12.1 GBK decoder):

```
1. If byte is in 0x00–0x7F: output it as a codepoint directly (ASCII passthrough).
2. If lead byte 0x81–0xFE: read next byte (trail).
   - If trail is 0x40–0xFE (not 0x7F):
       index = (lead - 0x81) * 190 + (trail - 0x40) - (trail > 0x7F ? 1 : 0)
       cp = gbk_decode[index]  (0 means unmapped → U+FFFD or error)
   - Otherwise: error (bad trail byte)
3. Otherwise: error (invalid lead byte range)
```

Note: for the ASCII passthrough, GBK decode processes 1 byte at a time
for ASCII, unlike UTF-16 which always reads 2 bytes. The decode view
iterator's current design reads bytes one at a time from the base range
in the codec arm — this is fine because the iterator already tracks
`current_` as it reads.

### WHATWG GBK encode algorithm

For a codepoint `cp`:
1. If `cp` is ASCII (0x00–0x7F): output 1 byte directly.
2. Find index `i` such that `gbk_decode[i] == cp`.
   - lead  = i / 190 + 0x81
   - trail = i % 190 + 0x40 + (i % 190 >= 0x3F ? 1 : 0)
   - Output 2 bytes: lead, trail.
3. If not found: unmapped → '?' (non-error) or error (or_error).

### GBK iterator design

Unlike UTF-16 (always 2 bytes) or UTF-8 (1–4 bytes with buffer), GBK
decode can produce 1 codepoint from 1 byte (ASCII) or 2 bytes
(multi-byte). The `whatwg_decode_view::iterator::load()` design handles
this naturally: it just reads however many bytes `gbk_decode_one` needs,
advancing `current_`.

For GBK encode, a codepoint can produce 1 byte (ASCII) or 2 bytes
(multi-byte). The existing `buf_[4]` / `len_` / `pos_` buffer design
handles this without changes to the iterator structure.

### Implementation order

1. Extend `tools/generate_tables.py` + add Python test
2. Generate table → `include/beman/transcode/detail/tables/gbk.hpp`
3. Create `include/beman/transcode/detail/gbk.hpp`
4. Add `codec::gbk` to enum in `whatwg_decode_view.hpp`
5. Add dispatch arms in both views (decode + encode)
6. Add `detail/gbk.hpp` + `detail/tables/gbk.hpp` to include CMakeLists.txt
7. Write test files and register in tests CMakeLists.txt
8. `make test` + `make lint` + `make coverage`

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

1. Branch: `git checkout -b step22-gbk`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): table generation + gbk.hpp + dispatch arms
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — gbk.hpp and gbk table should have good coverage
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- ASCII passthrough path (decode + encode) should be covered.
- Multi-byte decode path should be covered.
- Unmapped/error path should be covered.
- The table has ~23,940 entries; not all need direct test coverage —
  spot-check a representative sample.
