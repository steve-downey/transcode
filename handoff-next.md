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

**76 tests pass** (`make test`). Steps 0–18 complete on `main`.

The `codec` enum now has 31 values covering all WHATWG single-byte
decoders: utf_8, replacement, x_user_defined, ibm866, iso_8859_2–16
(skipping 1/9/11/12; iso_8859_8_i shares iso_8859_8 table), koi8_r,
koi8_u, macintosh, windows_874, windows_1250–1258, x_mac_cyrillic.

All 27 table headers live in `include/beman/transcode/detail/tables/`
with guards `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_*_HPP`.

The decode infrastructure (`whatwg_decode_view` + `whatwg_decode_or_error_view`)
dispatches every codec via `detail::single_byte_decode_one()`.

## What To Do Next — Step 19

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

### What Step 19 Does

Implement single-byte encoding: codepoint → legacy byte.

1. Add `single_byte_encode_one()` to `detail/single_byte.hpp` — reverse
   lookup through a 128-entry table to find the byte for a given codepoint.
   Returns `std::optional<unsigned char>` (nullopt if the codepoint is not
   in the table).

2. Create `include/beman/transcode/whatwg_encode_view.hpp`:
   - New view `whatwg_encode_view<C, R>` where R is a range of `char32_t`
     (or `char32_t`-convertible scalar values).
   - Encodes each codepoint to a legacy byte; invalid codepoints yield
     `U+003F` (question mark `?`) per WHATWG fallback.
   - Pipe adapter `whatwg_encode<codec::windows_1252>` etc.
   - `whatwg_encode_or_error_view` variant yielding
     `expected<unsigned char, whatwg_error>`.

3. Add dispatch in the encode view's `load()` for all 27 single-byte
   codecs (same list as decode). utf_8/replacement/x_user_defined are
   encode-side algorithmic — save them for steps 20/21.

4. Write tests for representative codecs (at least windows_1252,
   iso_8859_2, koi8_r) — both the success path and the fallback/error path.

### single_byte_encode_one() design

```cpp
// In detail/single_byte.hpp:
struct single_byte_encode_result {
    unsigned char byte{};
    bool          is_error{false};
};

template <std::ranges::input_iterator I, std::sentinel_for<I> S>
constexpr single_byte_encode_result
single_byte_encode_one(I& current, S end, const char32_t (&table)[128]);
```

The reverse lookup scans `table[0..127]` for `*current == table[i]`.
If found, returns `{static_cast<unsigned char>(0x80 + i), false}`.
For codepoints < 0x80 (ASCII), returns `{static_cast<unsigned char>(*current), false}`.
If not found, returns `{{}, true}` (error).

### New concept

The encode view needs a concept for the input range:

```cpp
// In detail/concepts.hpp or a new detail/unicode_scalar.hpp:
template <typename T>
concept unicode_scalar_type = std::same_as<T, char32_t>;

template <typename R>
concept unicode_scalar_range = std::ranges::input_range<R> &&
    unicode_scalar_type<std::ranges::range_value_t<R>>;
```

(This will be refined in step 20 when the UTF-8 encoder is added.)

### Include structure

New header: `include/beman/transcode/whatwg_encode_view.hpp`
- Add it to the FILE_SET in `include/beman/transcode/CMakeLists.txt`
- Include it from `include/beman/transcode/transcode.hpp` if that's the
  umbrella header pattern (check the existing `transcode.hpp`)

### TDD Process (RED → GREEN)

1. Branch: `git checkout -b step19-single-byte-encode`
2. Write failing tests (RED) → commit → push both
3. Implement (GREEN): `single_byte_encode_one()` + `whatwg_encode_view.hpp`
4. `make test` + `make lint` → commit → push both
5. Merge to main + push both
6. Mark checklist items `[x]`

## Important Facts from Steps 16–18

- `single_byte_decode_one()` treats table entry `0` as "unmapped" error.
  The same convention applies to encode: scan the table; if codepoint not
  found → error.

- The generated (WHATWG-normative) windows_1252 table maps bytes
  0x81/8D/8F/90/9D to C1 control characters (U+0081 etc.), not to 0.
  So the encode side WILL find mappings for those C1 codepoints.

- `iso_8859_8_i` shares `detail::tables::iso_8859_8` — both codec enum
  values dispatch to the same table for both encode and decode.

- Tables are in `include/beman/transcode/detail/tables/*.hpp` with guards
  `INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_*_HPP`. Do not use `data/tables/`
  for new code — the `include/` tree is authoritative.

- The `tools/generate_tables.py` script targets `data/tables/`. Future
  steps may update it to target `include/beman/transcode/detail/tables/`
  directly, but that is out of scope for step 19.

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Includes: angle brackets only, full path from include root
- Test files: include primary header twice (idempotent check)
- Functions: out-of-line in headers with full qualification
- `constexpr` everything that can be
- License: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Build Commands

```bash
make test      # build + run all tests
make lint      # clang-format + gersemi + codespell + ruff + gitleaks
make compile   # build only
make coverage  # coverage report
uv run pytest tools/tests/   # Python tests
uv run mypy tools/           # Python type check
```

## Coverage Rule

Run `make coverage` after tests and lint pass. New code should have
coverage. Don't fight the compiler for 100% — template instantiations
and unreachable platform branches are fine uncovered. But any
*surprising* uncovered code should either get a test or a note for
follow-up. Branch coverage is not a priority.
