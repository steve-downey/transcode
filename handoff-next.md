# Handoff: beman.transcode — Step 19 (Single-byte encoders)

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

**102 tests pass** (`make test`). Steps 0–18 complete, coverage gaps
addressed. On `main`.

The `codec` enum has 31 values:

```
utf_8, replacement, x_user_defined,           ← algorithmic (non-table)
ibm866, iso_8859_2, iso_8859_3, iso_8859_4,
iso_8859_5, iso_8859_6, iso_8859_7,
iso_8859_8, iso_8859_8_i,                     ← share one table file
iso_8859_10, iso_8859_13, iso_8859_14,
iso_8859_15, iso_8859_16, koi8_r, koi8_u,
macintosh, windows_874, windows_1250,
windows_1251, windows_1252, windows_1253,
windows_1254, windows_1255, windows_1256,
windows_1257, windows_1258, x_mac_cyrillic
```

28 single-byte codec values (27 unique table files — `iso_8859_8_i`
reuses `detail/tables/iso_8859_8.hpp`).

All 27 table headers live in:
`include/beman/transcode/detail/tables/*.hpp`

Both decode views (`whatwg_decode_view`, `whatwg_decode_or_error_view`)
are fully implemented for all 31 codec values.

No encode infrastructure exists yet — `whatwg_encode_view.hpp` does
not exist.

## Critical Facts Learned in Steps 17–18 + Coverage Sprint

**WHATWG windows_1252 table has no null entries.** The 5 bytes that
appear "undefined" in legacy documentation (0x81, 0x8D, 0x8F, 0x90,
0x9D) are mapped by WHATWG to C1 control characters (U+0081, U+008D,
U+008F, U+0090, U+009D). The generated table is correct. The decode
view's `windows_1252` error branch (`value_ = std::unexpected(r.error)`)
is therefore unreachable in practice; it is annotated with a comment
explaining this.

**Tables are in `include/`, not `data/`.** The step 18 Sonnet put
generated tables directly in `include/beman/transcode/detail/tables/`,
not `data/tables/`. Use the `include/` tree — it is authoritative for
the C++ headers.

**`data/tables/` exists but is not used by C++ code.** It was produced
by `tools/generate_tables.py` as an intermediate step. Step 18 copied
the generated content into `include/`. Ignore `data/tables/` for C++
include purposes.

**The zero-sentinel matters for some codecs.** `iso_8859_6`,
`iso_8859_8`, `windows_874`, `windows_1255`, `iso_8859_3` all have
genuine null entries (unmapped bytes). Use `iso_8859_6` byte `0xA1`
to test the unmapped-byte error path — it is confirmed null in the
table and covered by tests.

**`utf8.hpp` has two dead-code lines** (60 and 62). The guards
`cp > 0x10FFFF` and `extra==1 && cp < 0x80` are unreachable given
the lead byte constraints (0xF0–0xF4 and 0xC2–0xDF). Do not add
tests to cover them — they are defensive checks that cannot fire.

## What To Do Next — Step 19

**Branch:** `step19-single-byte-encode`

**Read the checklist:** `docs/plans/phase2-checklist.md`

### Goal

Implement single-byte encoding: `char32_t` codepoint → legacy byte.
Adds `single_byte_encode_one()` to `detail/single_byte.hpp` and
creates `include/beman/transcode/whatwg_encode_view.hpp`.

### 1. Add `single_byte_encode_one()` to `detail/single_byte.hpp`

```cpp
struct single_byte_encode_result {
    unsigned char byte{};
    bool          is_error{false};
};

// Encode one codepoint to a single legacy byte.
// ASCII passthrough for cp < 0x80. For cp >= 0x80: scans table[0..127]
// for cp == table[i]; returns {0x80 + i, false} if found, {{}, true} if not.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr single_byte_encode_result
single_byte_encode_one(I& current, S end, const char32_t (&table)[128]);
```

ASCII (cp < 0x80) is always a passthrough — all WHATWG single-byte
encodings are ASCII-compatible. The reverse scan is O(128) per
codepoint; that is acceptable for correctness now.

### 2. Create `include/beman/transcode/whatwg_encode_view.hpp`

New header following the exact same structure as `whatwg_decode_view.hpp`:

- `whatwg_encode_view<C, R>` — input: range of `char32_t`, output: `char`
- `whatwg_encode_or_error_view<C, R>` — output: `expected<char, whatwg_error>`
- `whatwg_encode_closure<C>` — pipe adapter (`| whatwg_encode<codec::windows_1252>`)
- `whatwg_encode_or_error_closure<C>` — pipe adapter
- Global variables: `whatwg_encode<C>`, `whatwg_encode_or_error<C>`

**Error convention for the non-error view:** unmapped codepoint →
emit `'?'` (0x3F, question mark). This is the WHATWG fallback for
encoder errors.

**Error convention for the or_error view:** unmapped codepoint →
`unexpected(whatwg_error::unmapped_codepoint)`. Add
`unmapped_codepoint` to the `whatwg_error` enum in `detail/error.hpp`
if not already present.

**Input concept:** Add `unicode_scalar_range` to `detail/concepts.hpp`:
```cpp
template <typename R>
concept unicode_scalar_range = std::ranges::input_range<R> &&
    std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, char32_t>;
```

**Include guard:** `INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP`

**Array rejection:** Same pattern as `whatwg_decode_view.hpp` — add
a `static_assert` overload of `operator|` rejecting `char32_t[N]`
raw arrays.

**`load()` dispatch:** Same `if constexpr` chain on `C` as the decode
views, covering all 28 single-byte codec values. `iso_8859_8_i` uses
the `iso_8859_8` table, same as for decode.

### 3. Update `include/beman/transcode/CMakeLists.txt`

Add `whatwg_encode_view.hpp` to the HEADERS FILES list.

### 4. Tests

New files:
- `tests/beman/transcode/whatwg_encode.test.cpp`
- `tests/beman/transcode/whatwg_encode_or_error.test.cpp`

Minimum tests:
- `windows_1252`: ASCII passthrough, valid high byte (0x20AC → 0x80),
  unmapped codepoint → `'?'`
- `iso_8859_2`: spot-check a non-ASCII mapping
- `koi8_r`: spot-check
- Or_error: unmapped codepoint → `unexpected(unmapped_codepoint)`,
  valid codepoint → `has_value()`
- Consteval test for at least one codec
- static_assert that the view satisfies `input_range`

Register both test executables in `tests/beman/transcode/CMakeLists.txt`
following the existing pattern.

### TDD Process

1. Branch: `git checkout -b step19-single-byte-encode`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): `single_byte_encode_one()` + `whatwg_encode_view.hpp`
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — new code should be covered; note any surprises
6. Merge to main + push both
7. Mark checklist `[x]`

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

## Coverage Rule

Run `make coverage` after tests and lint pass. New code should be
covered. Don't fight the compiler for 100% — template instantiations
and unreachable defensive guards are fine uncovered. Any *surprising*
gap should get a test or a follow-up note. Branch coverage is not a
priority.

**Known uncovered lines (do not add tests for these):**
- `utf8.hpp` lines 60, 62: dead code — unreachable given lead byte
  constraints
- `whatwg_decode_view.hpp` line 535 (windows_1252 `std::unexpected`):
  unreachable — generated table has no null entries for that codec
