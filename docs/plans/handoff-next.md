# Handoff: P4-Step 1 Complete — Pluggable Encode View

## Completed

- **P4-Step 1: Pluggable Encode View** — done on branch `p4-step1-pluggable-encode`

## What was done

Created `include/beman/transcode/encode_view.hpp` providing:
- `encode_view<Codec, R>` — forward/input range that yields `char` bytes by calling
  `codec.encode_one(cp)` for each `char32_t` in a `unicode_scalar_range`.  One codepoint
  may produce 1–8 bytes; the iterator buffers them internally (`buf_[8]`, `len_`, `pos_`).
  Unmapped codepoints (where `encode_result::is_error == true`) silently emit `'?'`.
- `encode_or_error_view<Codec, R>` — same but yields `std::expected<char, decode_error>`.
  Errors yield `std::unexpected(decode_error::unmapped_codepoint)`.
- `encode_closure<Codec>` / `encode_or_error_closure<Codec>` — pipe adaptors.
- `encode(Codec)` / `encode_or_error(Codec)` factory functions.

The design mirrors `decode_view.hpp` exactly:
- Same out-of-line function-body convention.
- Same `terminal()` static sentinel for forward-range `end()`.
- Same `std::default_sentinel_t end() const` fallback.
- Constrained on `encode_codec` concept (from `detail/codec_concepts.hpp`).
- Input constrained on `unicode_scalar_range` (from `detail/concepts.hpp`).

## Files created

- `include/beman/transcode/encode_view.hpp`
- `tests/beman/transcode/encode_view.test.cpp` — 14 tests: ASCII, upper-half,
  replacement byte, empty, span input, `_or_error` success/error/mixed,
  round-trip via `decode|encode`, constexpr, `base()`.
- `tests/beman/transcode/encode_view_reject.cpp` — negative compile test:
  a struct with no `encode_one` fails with "encode_codec" in the diagnostic.

## Files modified

- `include/beman/transcode/transcode.hpp` — added `#include <beman/transcode/encode_view.hpp>`
- `tests/beman/transcode/CMakeLists.txt` — added `beman.transcode.tests.encode_view`
  executable + `add_test` and `beman.transcode.tests.encode_view_reject` negative-compile
  target with `PASS_REGULAR_EXPRESSION "encode_codec"`.

## Current State

- `make test` passes (673 C++ + 250 Python tests)
- `make lint` passes (clang-format, gersemi, mypy, ruff, codespell, shellcheck all clean)
- Branch `p4-step1-pluggable-encode` is ready to merge into `main`

## Branch State

`p4-step1-pluggable-encode` is one commit ahead of `main`.

## Next Steps for P4-Step 2

Read `docs/plans/p4-step2-pluggable-bulk.md` next.

**What you need to know to start:**

- `encode(codec)` and `encode_or_error(codec)` pipe adaptors now exist in
  `include/beman/transcode/encode_view.hpp`.  They are the encode mirror of
  `decode(codec)` / `decode_or_error(codec)` in `include/beman/transcode/decode_view.hpp`.

- The test subject is `table_codec<Table>` from `include/beman/transcode/detail/table_codec.hpp`.
  It satisfies both `decode_codec` and `encode_codec`.  A Latin-1 test instance is created
  with `std::array<char32_t, 128> latin1_upper` (maps bytes 0x80-0xFF → U+0080-U+00FF).

- Step 2 adds *bulk* operations: `decode_to`, `encode_to`, and `into` for pluggable codecs.
  These are string/container-filling functions that avoid per-element overhead.
  They depend on Step 1's `encode_view` being available (Step 2 depends on Step 1).

- Look at `include/beman/transcode/detail/bulk_transcode.hpp` for the existing WHATWG
  bulk transcode pattern to follow.

- The worktree for the next step must be created fresh from `main` after the user
  merges `p4-step1-pluggable-encode` into `main`.  Per CLAUDE.md: start every task on
  its own branch and separate git worktree rooted from `main`.
