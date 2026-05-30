# P4-Step 4: iconv Bulk Operations

**Branch:** `p4-step4-iconv-bulk`
**Depends on:** None (independent of pluggable codec steps)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Add `iconv_transcode_to<Container>(input, from, to)` and
`iconv_transcode_into(input, from, to, output_iter)` that call raw `iconv()`
with block buffers, bypassing per-byte iteration.  Target: ~1.5-2x overhead
vs raw iconv (down from the current ~15-20x of `iconv_transcode_view`).

## Context for Executing Agent

The `iconv_transcode_view` in `include/beman/transcode/iconv_transcode_view.hpp`
wraps iconv as a per-byte range adaptor.  Each `++iterator` may trigger an
`iconv()` call that processes a few bytes into a small staging buffer, then
doles them out one at a time.  This is correct but slow.

The bulk operation should implement the same logic as the "before" code in
`examples/paper_iconv_view.cpp` (lines 12-56), but wrapped in a clean function
with RAII and proper error handling:

1. `iconv_open(to, from)` — RAII guard ensures `iconv_close()`
2. Loop: call `iconv()` with the full input as `inbuf`/`inbytesleft` and a
   growing output buffer
3. Handle `E2BIG`: grow output buffer (2x), retry
4. Handle `EILSEQ`: skip invalid byte, insert replacement char (U+FFFD as UTF-8:
   `\xEF\xBF\xBD` if output is UTF-8, else `?`)
5. Handle `EINVAL`: incomplete sequence at end of input — insert replacement
6. Return the completed container

### Dependency injection

Accept an `iconv_functions` struct (same pattern as the view in
`iconv_transcode_view.hpp` lines 16-27) so tests can use `mock_iconv` from
`tests/beman/transcode/iconv_mock.hpp`.

Default to real iconv (from `iconv_real.hpp`).

### Key files to reference

- `include/beman/transcode/iconv_transcode_view.hpp` — the view implementation
  (shows how iconv_functions is injected, how staging works)
- `include/beman/transcode/iconv_real.hpp` — the real iconv function pointers
- `tests/beman/transcode/iconv_mock.hpp` — mock functions for testing
- `examples/paper_iconv_view.cpp` — the "before" code to model
- `include/beman/transcode/detail/error.hpp` — `iconv_error` enum

## Deliverables

- `include/beman/transcode/iconv_bulk.hpp` — new file:
  - `iconv_transcode_to<Container>(range, from, to)` → Container
  - `iconv_transcode_to<Container>(range, from, to, fns)` → Container (DI overload)
  - `iconv_transcode_into(range, from, to, output_iter)` → output_iter
  - `iconv_transcode_into(range, from, to, output_iter, fns)` → output_iter
  - `iconv_transcode_to_or_error<Container>(range, from, to)` →
    `std::expected<Container, iconv_error>` (stops on first error instead of
    replacing)
- `tests/beman/transcode/iconv_bulk.test.cpp` — tests using mock_iconv
- Update `include/beman/transcode/transcode.hpp` to include the new header

## Constraints

- Input must satisfy `legacy_byte_range` (same as the view)
- The function must work with `std::string_view`, `std::span<const char>`,
  `std::vector<char>`, etc.
- For `iconv_transcode_into`, the output iterator receives `char` values
- The initial output buffer size should be `input.size() * 4` (worst case
  expansion for UTF-8 output), capped at some reasonable maximum to avoid
  huge allocations for small inputs
- The RAII guard for `iconv_t` should use a local struct or `std::unique_ptr`
  with a custom deleter

## Procedure

1. Create branch `p4-step4-iconv-bulk` from `main`
2. Create `include/beman/transcode/iconv_bulk.hpp`:
   - Include guard: `INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP`
   - Include: `<beman/transcode/detail/concepts.hpp>`,
     `<beman/transcode/detail/error.hpp>`, `<beman/transcode/iconv_real.hpp>`,
     `<expected>`, `<span>`, `<string>`, `<vector>`
3. Implement RAII guard:
   ```cpp
   struct iconv_guard {
       iconv_t handle;
       IconvFns fns;
       ~iconv_guard() { if (handle != (iconv_t)-1) fns.close(handle); }
   };
   ```
4. Implement `iconv_transcode_to`:
   - Materialize input into contiguous buffer if not already contiguous
   - Open iconv handle
   - Allocate output buffer (initial size = max(input_size * 4, 256))
   - Loop: call iconv(), handle E2BIG (grow 2x), EILSEQ (skip + replace),
     EINVAL (replace at end)
   - Resize output to actual bytes written, return
5. Implement `iconv_transcode_into`:
   - Same loop but write to output iterator instead of growing container
   - Need a fixed-size temporary buffer that gets flushed to the iterator
6. Implement `_or_error` variant:
   - Same structure but return `std::unexpected(iconv_error::invalid_sequence)`
     on EILSEQ instead of inserting replacement
7. Add `#include <beman/transcode/iconv_bulk.hpp>` to `transcode.hpp`
8. Write tests using mock_iconv:
   - Test identity transcode (mock that copies input to output)
   - Test E2BIG recovery (mock that always returns E2BIG with 1 byte consumed)
   - Test EILSEQ handling (mock that returns EILSEQ for specific bytes)
   - Test real iconv: `iconv_transcode_to<std::string>(input, "UTF-8", "UTF-32LE")`
9. Register tests in CMakeLists.txt
10. Run `make test` + `make lint`
11. Run `make bench-perf` and compare new bulk iconv vs view iconv numbers
12. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# Performance check — should see ~10x improvement over iconv_transcode_view:
make bench-perf
```

## Handoff to Step 5

Step 4 done, next read `docs/plans/p4-step5-iconv-null-term.md`.
The bulk operations close the iconv performance gap.
