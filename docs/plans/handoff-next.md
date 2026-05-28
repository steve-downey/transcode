# Handoff: P4-Step 4 Complete → P4-Step 5 Next

## Completed

- **P4-Step 1: Pluggable Encode View** — done, merged to `main`
- **P4-Step 2: Pluggable Bulk Operations** — done, merged to `main`
- **P4-Step 3: Pluggable Transcode Pipeline** — done, merged to `main`
- **P4-Step 4: iconv Bulk Operations** — done on `p4-step4-iconv-bulk` branch

## What was done in Step 4

Added bulk iconv transcoding functions in a new header
`include/beman/transcode/iconv_bulk.hpp`:

- `iconv_transcode_to<Container>(source, from, to, fns)` / `(source, from, to)` —
  transcodes a `legacy_byte_range` from encoding `from` to `to`, returning a
  Container (default `std::string`). Preallocates `max(input_size * 4, 256)` bytes
  and grows 2x on E2BIG. EILSEQ and trailing EINVAL replace bytes with `'?'`.
  Flushes stateful encodings at the end.

- `iconv_transcode_into(source, from, to, output, fns)` / `(source, from, to, output)` —
  same semantics but writes to an `std::output_iterator<char>`. Uses a fixed
  4096-byte temp buffer internally. Returns the advanced output iterator.

- `iconv_transcode_to_or_error<Container>(source, from, to, fns)` /
  `(source, from, to)` — like `iconv_transcode_to` but returns
  `std::unexpected(iconv_error::invalid_sequence)` on EILSEQ and
  `std::unexpected(iconv_error::incomplete_sequence)` on EINVAL instead of
  inserting a replacement character. E2BIG still grows the buffer.

All three accept an `IconvFns` template parameter for dependency injection;
the no-fns overloads call `make_real_iconv_fns()` from `iconv_real.hpp`.

A `detail::iconv_guard<IconvFns>` RAII struct closes the iconv handle.

- Added `#include <beman/transcode/iconv_bulk.hpp>` to `transcode.hpp`.

- `tests/beman/transcode/iconv_bulk.test.cpp` — 17 tests:
  - Identity transcode, empty input, `vector<char>` container
  - E2BIG recovery (`mock_iconv_e2big`)
  - EILSEQ replacement with `'?'` (`mock_iconv_eilseq`)
  - Trailing EINVAL (orphan byte) → `'?'` (`mock_iconv_pairwise`)
  - Stateful encoding flush (`mock_iconv_stateful`)
  - Real iconv UTF-8→UTF-32LE check
  - `_into` identity, EILSEQ, E2BIG, and iterator-advancement tests
  - `_or_error` success, EILSEQ error, EINVAL error, E2BIG recovery, real iconv

- Updated `tests/beman/transcode/CMakeLists.txt` — registered
  `beman.transcode.tests.iconv_bulk` with `Iconv::Iconv` link

## Files modified

- `include/beman/transcode/iconv_bulk.hpp` — new file
- `tests/beman/transcode/iconv_bulk.test.cpp` — new file
- `include/beman/transcode/transcode.hpp` — added iconv_bulk include
- `tests/beman/transcode/CMakeLists.txt` — registered new test target

## Current State

- `make test` passes: 688 C++ tests + 250 Python tests, all green
- `make lint` passes: mypy, ruff, clang-format, gersemi, codespell, shellcheck all clean

## Branch State

`p4-step4-iconv-bulk` is ready to merge to `main`.
Merge with `--no-ff`.

The user should merge step 4 before starting step 5.

## Next Step: P4-Step 5 — null_term support for iconv views

Read `docs/plans/p4-step5-iconv-null-term.md` for full details.

**Summary of what Step 5 requires:**

Add `null_term` support so that C strings can be piped directly to
`iconv_transcode` and `iconv_transcode_or_error` without manually wrapping
them in `views::null_term`. This closes the ergonomic gap where the WHATWG
pipeline accepts `views::null_term(ptr)` but the iconv pipeline requires an
explicit intermediate step.

**Key files to read before starting Step 5:**

- `include/beman/transcode/detail/null_term.hpp` — `views::null_term` adapter
- `include/beman/transcode/iconv_transcode_view.hpp` — iconv_transcode_closure
- `include/beman/transcode/iconv_transcode_or_error_view.hpp` — or_error closure
- `include/beman/transcode/iconv_real.hpp` — the pipe factories
- `docs/plans/p4-step5-iconv-null-term.md` — detailed step plan
- `docs/plans/phase4-index.md` — phase overview

**Start fresh:** create branch `p4-step5-iconv-null-term` from `main` after
merging step 4.
