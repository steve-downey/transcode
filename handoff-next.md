# Handoff: beman.transcode — Step 41 (sniff_encoding / BOM Detection API)

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

**Step 40 complete and merged to main.**

502 C++ tests + 171 Python tests pass (`make test`). `make lint` clean
(ignoring pre-existing failures in `papers/wg21/` which are unrelated to
the transcode library).

### What Step 40 Added

WHATWG label lookup API — `get_encoding(std::string_view label)`.

Key changes:
- `tools/generate_labels.py` — reads `docs/whatwg/encodings.json`, builds
  a sorted `label → codec` table, generates `include/beman/transcode/detail/labels.hpp`
- `tools/tests/test_generate_labels.py` — 12 Python tests for the generator
- `include/beman/transcode/detail/labels.hpp` — generated header with a
  228-entry `constexpr` sorted table and a `constexpr get_encoding()` that
  strips ASCII whitespace, lowercases, and binary-searches the table
- `tests/beman/transcode/labels.test.cpp` — 7 C++ TEST_CASE blocks including
  a `constify()` consteval test

The `encodings.json` was already in `docs/whatwg/` (downloaded in an earlier step).

API:
```cpp
// In namespace beman::transcoding
constexpr std::optional<codec> get_encoding(std::string_view label) noexcept;

static_assert(get_encoding("  UTF-8  ") == codec::utf_8);
static_assert(get_encoding("sjis")      == codec::shift_jis);
static_assert(get_encoding("unknown")   == std::nullopt);
```

## What To Do Next — Step 41 (sniff_encoding / BOM Detection)

**Branch:** `step41-sniff-encoding`

### Goal

Implement `sniff_encoding(range)` — WHATWG Encoding Standard §8.2 "Determining
the fallback encoding". This detects the encoding of a byte sequence by looking
for a BOM at the start, returning:

- `codec::utf_8` if the sequence starts with `EF BB BF`
- `codec::utf_16be` if the sequence starts with `FE FF`
- `codec::utf_16le` if the sequence starts with `FF FE`
- `std::nullopt` if no BOM is found

API (in `include/beman/transcode/detail/sniff.hpp`):

```cpp
namespace beman::transcoding {

// Returns the codec if a BOM is detected, or nullopt.
// Accepts any legacy_byte_range (same concept as whatwg_decode_view).
template <legacy_byte_range R>
constexpr std::optional<codec> sniff_encoding(R&& r) noexcept;

} // namespace beman::transcoding
```

Example:
```cpp
std::vector<unsigned char> utf8_bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
assert(sniff_encoding(utf8_bom) == codec::utf_8);

std::vector<unsigned char> no_bom = {'h', 'i'};
assert(sniff_encoding(no_bom) == std::nullopt);
```

### Implementation notes

- `sniff_encoding` is a range-based function: it consumes at most 3 bytes.
- It should work on `input_range` (not just `random_access_range`).
- The `legacy_byte_range` concept already exists in `detail/concepts.hpp`
  and accepts `char`/`signed char`/`unsigned char`/`std::byte` ranges.
- No new concept constraints needed — negative compile test not required.
- Include `<optional>` and `<ranges>`.
- The function should be `constexpr`.

### Files to create/modify

1. `include/beman/transcode/detail/sniff.hpp` — new header with `sniff_encoding()`
2. `tests/beman/transcode/sniff.test.cpp` — C++ tests
3. `tests/beman/transcode/CMakeLists.txt` — register new test

No generator script is needed for this step — the logic is simple enough
to write by hand. No new Python tests either.

### C++ tests

1. Runtime tests: `sniff_encoding` on vectors with each BOM and without
2. `consteval` test via `constify()` from `test_utilities.hpp`
3. Test with `views::null_term` (a C string) — no BOM expected

### TDD Process

1. Branch: `git checkout -b step41-sniff-encoding`
2. Write failing C++ tests (RED) — `sniff.hpp` doesn't exist yet
3. Commit RED → push both remotes
4. Implement `detail/sniff.hpp`
5. `make test` (all pass) + `make lint` (clean)
6. Commit GREEN → push both remotes
7. Merge: `git checkout main && git merge --no-ff step41-sniff-encoding`
8. Push main to both remotes
9. Update `docs/plans/phase2-checklist.md`

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
```

## Coding Rules (abbreviated)

- Include guard: `INCLUDE_BEMAN_TRANSCODE_DETAIL_SNIFF_HPP`
- Test file: include the primary header **twice** (idempotent check)
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files to read for context

- `include/beman/transcode/detail/concepts.hpp` — `legacy_byte_range` concept
- `include/beman/transcode/whatwg_decode_view.hpp` — codec enum + existing BOM
  stripping logic (for reference on how BOMs are checked)
- `tests/beman/transcode/labels.test.cpp` — recent test file for style reference
