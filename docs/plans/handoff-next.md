# Handoff: P4-Step 3 Complete → P4-Step 4 Next

## Completed

- **P4-Step 1: Pluggable Encode View** — done on `p4-step1-pluggable-encode` branch
- **P4-Step 2: Pluggable Bulk Operations** — done on `p4-step2-pluggable-bulk` branch
- **P4-Step 3: Pluggable Transcode Pipeline** — done on `p4-step3-pluggable-transcode` branch

## What was done in Step 3

- Modified `include/beman/transcode/detail/transcode_view.hpp`:
  - Added `#include <beman/transcode/decode_view.hpp>` and `<beman/transcode/encode_view.hpp>`
  - Added `pluggable_transcode_closure<From, To>` struct that holds two codec objects
    and composes `decode(from_) | encode(to_)` in its `operator()(R&&)` and pipe `operator|`
  - Added `pluggable_transcode(From, To)` factory function that returns the closure
  - Out-of-line `operator()` definition follows the project convention

- **Naming note:** The factory is named `pluggable_transcode` (not `transcode`) because
  C++ does not allow a function template and a variable template to share the same name
  in the same namespace.  The existing `inline constexpr transcode_closure<From, To> transcode{}`
  variable template would conflict with a `transcode(From, To)` function template.
  The plan said they would coexist but GCC (correctly) rejects it.

- `tests/beman/transcode/pluggable_transcode.test.cpp` — 8 tests:
  - ASCII identity round-trip via pipe
  - Upper-half (0x80-0xFF) identity round-trip via pipe
  - Empty input
  - Mixed ASCII and upper-half bytes
  - Closure `operator()` called directly (not piped)
  - `span<const char>` input
  - Two consteval (constexpr lambda) tests using `constify()`

- Updated `tests/beman/transcode/CMakeLists.txt` — registered
  `beman.transcode.tests.pluggable_transcode` executable with `add_test`

## Files modified

- `include/beman/transcode/detail/transcode_view.hpp` — added pluggable closure and factory
- `tests/beman/transcode/pluggable_transcode.test.cpp` — new test file
- `tests/beman/transcode/CMakeLists.txt` — added pluggable_transcode test target
- `include/beman/transcode/encode_view.hpp`, `tests/beman/transcode/encode_view.test.cpp`,
  `tests/beman/transcode/pluggable_bulk.test.cpp` — clang-format whitespace fixes

## Current State

- `make test` passes: 675 C++ tests + 250 Python tests, all green
- `make lint` passes: mypy, ruff, clang-format, gersemi, codespell, shellcheck all clean

## Branch State

`p4-step3-pluggable-transcode` is ready to merge to `main`.
All three of steps 1-3 live in this branch's ancestry (`p4-step3-pluggable-transcode`
was branched from `p4-step2-pluggable-bulk`).  Merge with `--no-ff`.

The user should merge step 3 before starting step 4.

## Next Step: P4-Step 4 — iconv Bulk Operations

Read `docs/plans/p4-step4-iconv-bulk.md` for full details.

**Summary of what Step 4 requires:**

Add `iconv_transcode_to` and `iconv_transcode_into` bulk functions that wrap
`iconv_transcode_view` to transcode a byte range from one encoding to another
using iconv, yielding a `std::string` or writing to an output iterator.
This closes the performance gap for iconv bulk transcoding (currently requiring
users to drive the view manually for every string).

**Key files to read before starting Step 4:**

- `include/beman/transcode/iconv_transcode_view.hpp` — the iconv view (source of truth)
- `include/beman/transcode/detail/bulk_transcode.hpp` — existing WHATWG bulk functions
  (pattern to follow for new iconv bulk overloads)
- `docs/plans/p4-step4-iconv-bulk.md` — detailed step plan
- `docs/plans/phase4-index.md` — phase overview

**Start fresh:** create branch `p4-step4-iconv-bulk` from `main` after merging step 3.
