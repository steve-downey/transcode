# Handoff: P4-Step 5 Complete → P4-Step 6 Next

## Completed

- **P4-Step 1: Pluggable Encode View** — done, merged to `main`
- **P4-Step 2: Pluggable Bulk Operations** — done, merged to `main`
- **P4-Step 3: Pluggable Transcode Pipeline** — done, merged to `main`
- **P4-Step 4: iconv Bulk Operations** — done, merged to `main`
- **P4-Step 5: null_term support for iconv views** — done on `p4-step5-iconv-null-term` branch

## What was done in Step 5

**Outcome: no code changes needed.** The `iconv_transcode_view` already works with
`views::null_term` because:

- Its iterator holds `base_iter` / `base_sent` by value and compares them with
  `current_ != end_`, which uses the C++20 rewritten comparison rule to call
  `null_sentinel_t::operator==`.
- The `load()` function appends input one byte at a time via `*current_` and
  `++current_`, with no `size()` or `end - begin` arithmetic.
- `iconv_transcode_to` (bulk) materializes input with a range-for loop, which
  also terminates correctly against any sentinel.

Added `tests/beman/transcode/iconv_null_term.test.cpp` — 9 tests:

- Mock: `null_term` string literal piped to `iconv_transcode_closure` identity
- Mock: `null_term` char pointer piped to `iconv_transcode_closure` identity
- Mock: empty `null_term` produces empty output
- Mock: `null_term` piped to `iconv_transcode_or_error_closure` identity
- Mock: `iconv_transcode_to` with `null_term` string literal source
- Mock: `iconv_transcode_to` with `null_term` char pointer source
- Real iconv: `null_term | iconv_transcode` UTF-8 identity
- Real iconv: `null_term | iconv_transcode` UTF-8→UTF-32LE ("A" → 4 bytes)
- Real iconv: `iconv_transcode_to` with `null_term` source UTF-8 identity

The linter (clang-format) also applied minor alignment fixes to
`iconv_bulk.hpp` and `iconv_bulk.test.cpp`.

Registered `beman.transcode.tests.iconv_null_term` in `tests/beman/transcode/CMakeLists.txt`
with `Iconv::Iconv` link.

## Files modified

- `tests/beman/transcode/iconv_null_term.test.cpp` — new file (9 tests)
- `tests/beman/transcode/CMakeLists.txt` — registered new test target
- `include/beman/transcode/iconv_bulk.hpp` — clang-format alignment fix
- `tests/beman/transcode/iconv_bulk.test.cpp` — clang-format alignment fix

## Current State

- `make test` passes: 697 C++ tests + 250 Python tests, all green
- `make lint` passes: mypy, ruff, clang-format, gersemi, codespell, shellcheck all clean

## Branch State

`p4-step5-iconv-null-term` is ready to merge to `main`.
Merge with `--no-ff`.

The user should merge step 5 before starting step 6.

## Next Step: P4-Step 6 — Error Coherence

Read `docs/plans/p4-step6-error-coherence.md` for full details.

**Summary of what Step 6 requires:**

Unify or document the error enum naming across the three backends. The
pluggable codec backend has its own error types, the iconv backend has
`iconv_error` (in `detail/error.hpp`), and the WHATWG backend has
`whatwg_error`. Step 6 audits whether these can share a common enum or
whether the differences are intentional and should be documented.

**Key files to read before starting Step 6:**

- `include/beman/transcode/detail/error.hpp` — `whatwg_error` and
  `iconv_error` enum definitions
- `include/beman/transcode/iconv_transcode_or_error_view.hpp` — uses
  `iconv_error`
- `include/beman/transcode/whatwg_decode_view.hpp` — WHATWG error path
  (replacement char `U'�'`, no enum surfaced)
- `docs/plans/p4-step6-error-coherence.md` — detailed step plan
- `docs/plans/phase4-index.md` — phase overview

**Start fresh:** create branch `p4-step6-error-coherence` from `main` after
merging step 5.
