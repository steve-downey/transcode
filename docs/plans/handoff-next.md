# Handoff: Coverage Audit Complete — Ready for Phase 5 Planning

## Completed

- **P4-Step 1: Pluggable Encode View** — done, merged to `main`
- **P4-Step 2: Pluggable Bulk Operations** — done, merged to `main`
- **P4-Step 3: Pluggable Transcode Pipeline** — done, merged to `main`
- **P4-Step 4: iconv Bulk Operations** — done, merged to `main`
- **P4-Step 5: null_term support for iconv views** — done, merged to `main`
- **P4-Step 6: Error Enum Coherence** — done, merged to `main`
- **Coverage Audit** — done on `coverage-p4-audit` branch

## What was done in the Coverage Audit

Ran `make coverage` on `main` after merging all P4 steps. Baseline:
- **Lines**: 78.3% (8234/10510)
- **Functions**: 96.5% (2184/2264) — already excellent
- **Branches**: 42.6% — not a priority per CLAUDE.md

Added 10 new tests (+10 C++ tests, now 707 total) across 5 files:

### `tests/beman/transcode/iconv_mock.hpp`
- Added `mock_iconv_open_fail` — returns `(iconv_t)-1` to simulate
  `iconv_open` failure.

### `tests/beman/transcode/iconv_bulk.test.cpp` (+7 tests)
- `iconv_transcode_to` open failure → empty container (line 56)
- `iconv_transcode_into` open failure → unchanged iterator (line 151)
- `iconv_transcode_to_or_error` open failure → `invalid_sequence` (line 219)
- `iconv_transcode_into` with pairwise mock → EINVAL orphan byte → '?' (lines 177-179)
- `iconv_transcode_into` stateful flush → 0x0F appended (lines 185-191)
- `iconv_transcode_to_or_error` stateful flush → 0x0F appended (lines 250-253)

### `tests/beman/transcode/bulk_transcode.test.cpp` (+2 tests)
- `decode_to<codec::iso_8859_6>` with unmapped byte 0xA1 → U+FFFD
  (line 147 in `single_byte_tables.hpp`)
- `encode_to<codec::iso_8859_6>` with U+0100 (not in table) → '?'
  (line 160 in `single_byte_tables.hpp`)

### `tests/beman/transcode/whatwg_decode_or_error.test.cpp` (+1 test)
- `\xF4\x90\x80\x80` → `out_of_range` (line 66 in `utf8.hpp`):
  F4 with first continuation > 0x8F = U+110000, beyond Unicode max.

### `tests/beman/transcode/labels.test.cpp` (+1 test)
- 64-char label → `std::nullopt` (line 275 in `labels.hpp`):
  label buffer is 63 bytes; label ≥ 64 bytes cannot match any codec.

### After audit coverage
- **Lines**: 78.3% (8267/10564)
- **`labels.hpp`**: 100% (was 94.7%)
- **`iconv_bulk.hpp`**: 83.6% (was 78.9%)
- **`single_byte_tables.hpp`**: 88.9% (was 77.8%)
- **`utf8.hpp`**: 94.6% (was 91.9%)

## Remaining Coverage Gaps (Explained)

These are NOT worth fighting. Each has a documented reason:

| File | Lines | Reason |
|------|-------|--------|
| `single_byte.hpp` | 51, 56 | Template inlining artifact — gcov attributes coverage to `whatwg_encode_view.hpp` call site; the code runs but isn't attributed here |
| `utf8.hpp` | 73, 75 | Defensive dead code: surrogates always caught at line 62 (byte-level), 4-byte sequences can never produce values > 0x10FFFF given the F4/0x90 guard |
| `iconv_bulk.hpp` | 109, 185, 250 | gcov artifact: `while(true)` headers are not marked covered on a single-iteration loop |
| `iconv_bulk.hpp` | 83-87, 96-100 | Sub-cases requiring EILSEQ/EINVAL simultaneously with a full output buffer — needs a mock that coordinates two conditions at once |
| `iconv_bulk.hpp` | 114-118, 254-261 | E2BIG during stateful flush — requires a mock that forces E2BIG on first flush then succeeds on second; complex stateful mock |
| `single_byte_tables.hpp` | 144, 155 | `table == nullptr` guard in decode/encode — only reachable via `single_byte_decode<codec::replacement>` streaming view, which no current test exercises |
| `iconv_transcode_or_error_view.hpp` | 2 lines | Iterator equality when both iterators are mid-range (not at sentinel) |
| `whatwg_encode_view.hpp` | 6 lines | Iterator equality when both iterators are mid-range |
| `bulk_transcode.hpp` | 74, 92 | Closing `}` of if-constexpr branches — gcov attribute artifact |

## Current State

- `make test` passes: 707 C++ tests + 250 Python tests, all green
- `make lint` passes: all hooks clean

## Branch State

`coverage-p4-audit` is ready to merge to `main`.
Merge with `--no-ff`.

## What Comes Next

The coverage audit is done. Actionable next steps, in priority order:

1. **Paper update** (`papers/transcode-view.md`) — The pluggable codec API
   section was written before Steps 1-3 (encode views, bulk ops, transcode
   pipeline). It needs a new section describing these additions with code
   examples, matching the style of the existing WHATWG codec sections.
   Key files to read: `include/beman/transcode/encode_view.hpp`,
   `include/beman/transcode/bulk_transcode.hpp`,
   `include/beman/transcode/transcode_view.hpp`.

2. **WHATWG null-terminated encode gap** — The README API matrix shows
   `views::null_term` as ✅ for the WHATWG decode column but 🔴 for encode.
   The `whatwg_encode` view works fine with null_term input (it's a unicode
   scalar range, not a byte range), but there are no tests for it.

3. **BOM sniffing / runtime transcode gaps** — The README shows 🔴 for
   runtime transcode and BOM sniffing in pluggable and iconv columns.
   These are larger features; a Phase 5 plan would be needed.

A fresh agent should read `docs/plans/phase4-index.md` and this file to
understand the completed state before starting new work.
