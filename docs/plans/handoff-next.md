# Handoff: P4-Step 6 Complete — Phase 4 Done

## Completed

- **P4-Step 1: Pluggable Encode View** — done, merged to `main`
- **P4-Step 2: Pluggable Bulk Operations** — done, merged to `main`
- **P4-Step 3: Pluggable Transcode Pipeline** — done, merged to `main`
- **P4-Step 4: iconv Bulk Operations** — done, merged to `main`
- **P4-Step 5: null_term support for iconv views** — done, merged to `main`
- **P4-Step 6: Error Enum Coherence** — done on `p4-step6-error-coherence` branch

## What was done in Step 6

**Decision: remove the `decode_error` alias; use `whatwg_error` directly.**

The alias `using decode_error = whatwg_error;` was confusing because it was
named as if it were a generic codec error type, but it was literally `whatwg_error`.

### Files modified

- **`include/beman/transcode/detail/error.hpp`** — added documentation comment
  blocks explaining:
  - `whatwg_error`: WHATWG Encoding Standard error categories; used by
    `whatwg_decode_or_error`, `whatwg_encode_or_error`, and all pluggable codec
    `_or_error` views (pluggable codecs adopt WHATWG error semantics).
  - `iconv_error`: POSIX iconv errno mappings; kept separate because iconv
    cannot distinguish WHY a byte sequence is invalid (`EILSEQ`), only that it
    is, that a sequence is incomplete (`EINVAL`), or that the output buffer is
    full (`E2BIG`).

- **`include/beman/transcode/detail/codec_result.hpp`** — removed
  `using decode_error = whatwg_error;`; changed `decode_result::error` field
  type from `decode_error` to `whatwg_error`.

- **`include/beman/transcode/decode_view.hpp`** — replaced all `decode_error`
  references with `whatwg_error` (value_type, result_t, unexpected call).

- **`include/beman/transcode/detail/table_codec.hpp`** — replaced
  `decode_error::invalid_byte` with `whatwg_error::invalid_byte` in both
  `table_codec` and `full_table_codec`.

- **`tests/beman/transcode/codec_concepts.test.cpp`** — updated two test
  assertions from `decode_error::*` to `whatwg_error::*`.

- **`tests/beman/transcode/decode_view.test.cpp`** — updated two test
  assertions from `decode_error::*` to `whatwg_error::*`.

- **`tests/beman/transcode/iconv_null_term.test.cpp`** — clang-format alignment
  fixes only (no logic change).

- **`README.md`** — updated API Surface Matrix error row: pluggable codec column
  now shows `whatwg_error` instead of `decode_error`. Updated observations text
  to explain the separation.

- **`papers/transcode-view.md`** — same API Surface Matrix update.

## Current State

- `make test` passes: 697 C++ tests + 250 Python tests, all green
- `make lint` passes: all hooks clean

## Branch State

`p4-step6-error-coherence` is ready to merge to `main`.
Merge with `--no-ff`.

## Phase 4 Complete

All API surface gaps from the Phase 4 scope are now closed:

| Step | Deliverable | Status |
|------|-------------|--------|
| 1 | Pluggable encode view + encode_or_error | ✅ merged |
| 2 | Pluggable bulk decode_to/encode_to/into | ✅ merged |
| 3 | Pluggable transcode pipeline | ✅ merged |
| 4 | iconv bulk transcode (iconv_transcode_to/into) | ✅ merged |
| 5 | null_term support for iconv views | ✅ merged |
| 6 | Error enum coherence (removed decode_error alias) | ✅ ready to merge |

The README API Surface Matrix no longer shows `decode_error` anywhere;
the pluggable codec column now correctly shows `whatwg_error`.

## What Comes Next

There is no Phase 5 plan yet. Candidates for future work based on the
README matrix and paper:

1. **Coverage audit** — Run `make coverage` and look for uncovered lines in
   the Phase 4 additions (pluggable bulk, iconv bulk, error paths in
   decode_or_error_view). This is low-effort and may surface gaps.

2. **WHATWG encode gap** — The WHATWG column in the API matrix still lacks a
   streaming encode view for non-UTF-8 targets (BOM sniffing, runtime
   transcode). These are longer-horizon features.

3. **Paper update** — `papers/transcode-view.md` may need the pluggable codec
   API section updated to describe the new encode views and bulk operations
   added in Steps 1-3.

4. **P2728R12 alignment** — The paper column gaps are expected (different
   axis), but the `_or_error` suffix convention and `char32_t` codepoint type
   are intentional alignments worth keeping in sync as that paper evolves.

A fresh agent should read `docs/plans/phase4-index.md` and this file to
understand the completed state before starting new work.
