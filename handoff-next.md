# Handoff: beman.transcode — Step 28 (EUC-KR decode + encode)

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

**363 C++ tests + 91 Python tests pass** (`make test`). Steps 0–27
complete. On `main`.

### What Step 27 Built

Step 27 added ISO-2022-JP decode and encode — a **stateful** codec with
escape-sequence-driven state transitions.

Key design decisions that affect all future work:

- **Encode buffers resized** from `[4]` to `[8]` in both
  `whatwg_encode_view::iterator` and `whatwg_encode_or_error_view::iterator`.
  (ISO-2022-JP needs up to 5 bytes for ESC$B+lead+trail.)

- **State fields** added to all four iterator classes (always present,
  zeroed for non-ISO-2022-JP codecs):
  - Decode iterators: `int iso2022jp_state_{0}` and
    `unsigned char iso2022jp_lead_{0}`
  - Encode iterators: `int iso2022jp_state_{0}`

- **Pre-dispatch end-of-stream guard** modified in both decode `load()`
  functions: `if (!has_pending_ && iso2022jp_state_ < 2)` — states 2–6
  fall through to the codec arm. This is safe for all other codecs since
  `iso2022jp_state_` is always 0 for them.

- **WHATWG ISO-2022-JP semantics**:
  - ESC in Lead_Byte state: silent transition (no U+FFFD) per WHATWG
  - End-of-stream in Lead_Byte state: U+FFFD (error) per WHATWG
  - Encode end-of-stream in non-ASCII state: appends `ESC ( B`
  - So encoding a single JIS0208 char like U+3000 produces 8 bytes total:
    `1B 24 42 21 21 1B 28 42`

- **No new table**: ISO-2022-JP reuses `tables::shift_jis[0..8835]`
  (JIS X 0208 range, pointer = `(lead-0x21)*94 + (trail-0x21)`).

### Current codec enum (in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8, replacement, x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    utf_16be, utf_16le,
    gbk, gb18030, big5, shift_jis, euc_jp,
    iso_2022_jp,
    // euc_kr NOT YET — step 28
};
```

## What To Do Next — Step 28

**Branch:** `step28-euc-kr`

**Read the checklist:** `docs/plans/phase2-checklist.md`

### Overview

EUC-KR is a **stateless** two-byte encoding for Korean. It is the
simplest of the remaining codecs — no state machine. The WHATWG encoding
includes the UHC extension (aka CP949).

WHATWG index: `docs/whatwg/index-euc-kr.txt` (already downloaded).

### Table structure

```
pointer = (lead - 0x81) * 190 + (trail - 0x41)
lead  in [0x81, 0xFE]   (126 values)
trail in [0x41, 0xFE]   (190 values, but 0x7F excluded)
```

The WHATWG EUC-KR index has **17176 entries** (verify:
`wc -l docs/whatwg/index-euc-kr.txt`). Some entries are 0 (unmapped).

### Step 1: Extend `tools/generate_tables.py`

Add:
- `EUC_KR_POINTER_COUNT = 17176`
- `parse_euc_kr_index()`: reads `docs/whatwg/index-euc-kr.txt`
- `render_euc_kr_hpp()`: generates
  `include/beman/transcode/detail/tables/euc_kr.hpp`
- `generate_euc_kr()`: orchestrate; call from `main()`

Guard: `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_KR_HPP`
Array: `inline constexpr char32_t euc_kr[17176] = { ... };`

Also add tests to `tools/tests/test_generate.py`.

### Step 2: Create `include/beman/transcode/detail/euc_kr.hpp`

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_KR_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_EUC_KR_HPP

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/tables/euc_kr.hpp>
#include <iterator>

namespace beman::transcoding::detail {

struct euc_kr_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct euc_kr_encode_result {
    unsigned char bytes[2]{};
    int           count{0};
    bool          is_error{false};
};

template <std::input_iterator I, std::sentinel_for<I> S>
constexpr euc_kr_decode_result euc_kr_decode_one(I& current, S end);

constexpr euc_kr_encode_result euc_kr_encode_one(char32_t cp);

// Out-of-line definitions follow...
```

**Decode algorithm**:
```
if current == end: return {0xFFFD, truncated_sequence, true}
lead = *current++
if lead < 0x80: return {char32_t(lead), {}, false}
if lead < 0x81 or lead > 0xFE: return {0xFFFD, invalid_byte, true}
if current == end: return {0xFFFD, truncated_sequence, true}
trail = *current++
if trail < 0x41 or trail > 0xFE or trail == 0x7F:
    return {0xFFFD, invalid_byte, true}
pointer = (lead - 0x81) * 190 + (trail - 0x41)
cp = tables::euc_kr[pointer]
if cp == 0: return {0xFFFD, invalid_byte, true}
return {cp, {}, false}
```

**Encode algorithm**:
```
if cp < 0x80: return {cp, 1, false}
for i in 0..17175:
    if tables::euc_kr[i] == cp:
        lead  = (i / 190) + 0x81
        trail = (i % 190) + 0x41
        if trail >= 0x7F: trail += 1  // skip DEL
        return {{lead, trail}, 2, false}
return {{}, 0, true}  // unmapped
```

### Step 3: Add `codec::euc_kr` to the enum

In `whatwg_decode_view.hpp`, after `iso_2022_jp`:
```cpp
    iso_2022_jp,
    euc_kr,
```

### Step 4: Add dispatch arms in both view headers

In `whatwg_decode_view.hpp` (both `load()` functions):
```cpp
} else if constexpr (C == codec::euc_kr) {
    auto r = detail::euc_kr_decode_one(current_, end_);
    value_ = r.is_error ? U'\xFFFD' : r.code_point;
```

Or-error variant:
```cpp
} else if constexpr (C == codec::euc_kr) {
    auto r = detail::euc_kr_decode_one(current_, end_);
    if (r.is_error) value_ = std::unexpected(r.error);
    else value_ = r.code_point;
```

In `whatwg_encode_view.hpp` (both `load()` functions):
```cpp
} else if constexpr (C == codec::euc_kr) {
    auto r = detail::euc_kr_encode_one(static_cast<char32_t>(*current_));
    ++current_;
    if (r.is_error) { buf_[0] = '?'; len_ = 1; }
    else {
        for (int i = 0; i < r.count; ++i)
            buf_[i] = static_cast<char>(r.bytes[i]);
        len_ = r.count;
    }
    pos_ = 0;
```

Or-error variant replaces `'?'` with
`std::unexpected(whatwg_error::unmapped_codepoint)`.

### Step 5: Add includes

In `whatwg_decode_view.hpp`:
```cpp
#include <beman/transcode/detail/euc_kr.hpp>
```

In `whatwg_encode_view.hpp`: same (or it comes via `whatwg_decode_view.hpp`).

### Step 6: Test files

Create `tests/beman/transcode/euc_kr_decode.test.cpp` and
`tests/beman/transcode/euc_kr_encode.test.cpp`. Register both in
`tests/beman/transcode/CMakeLists.txt`.

**Key test values** — verify by running:
```bash
python3 -c "
lines = [l.strip() for l in open('docs/whatwg/index-euc-kr.txt') if l.strip() and not l.startswith('#')]
for i, line in enumerate(lines):
    parts = line.split()
    if parts[1] == '0xAC00':
        lead = (i // 190) + 0x81
        trail_raw = (i % 190) + 0x41
        trail = trail_raw if trail_raw < 0x7F else trail_raw + 1
        print(f'U+AC00: pointer={i}, lead=0x{lead:02X}, trail=0x{trail:02X}')
        break
"
```

Then use those values in:
- Decode test: `{lead_byte, trail_byte}` → U+AC00 (가)
- Encode test: U+AC00 → `{lead_byte, trail_byte}`

Standard test cases:
- ASCII passthrough ('A' = 0x41 → U'A')
- Korean syllable (verify exact bytes)
- Unmapped pointer (find a zero entry in the table) → U+FFFD
- Invalid lead byte 0x80 → U+FFFD
- Invalid trail byte 0x7F → U+FFFD
- Truncated sequence (lone lead byte) → U+FFFD
- or_error variants for each error
- Pipe syntax and consteval (ASCII only)

### No changes needed to `tools/generate_tables.py` invocation

The `main()` function already calls `generate_euc_jp()` etc. Just add
`generate_euc_kr()` to that list.

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
```

## TDD Process

1. Branch: `git checkout -b step28-euc-kr`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): generate table, create header, add enum + dispatch
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage`
6. Merge to main + push both
7. Mark checklist `[x]`
