# P4-Step 5: iconv Null-Terminated Input

**Branch:** `p4-step5-iconv-null-term`
**Depends on:** None (independent of other steps)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Verify and test that `views::null_term(ptr) | iconv_transcode(...)` works.
Fix the view if `null_sentinel_t` is not handled correctly.

## Context for Executing Agent

`views::null_term(ptr)` (in `include/beman/transcode/detail/null_term.hpp`)
produces a range of `char` values with `null_sentinel_t` as the sentinel type.
The `whatwg_decode_view` handles this fine because it works with any
`legacy_byte_range`, comparing iterators against `null_sentinel_t`.

The `iconv_transcode_view` also accepts `legacy_byte_range`, but its
implementation needs to call `iconv()` which requires a `char*` + `size_t`
(contiguous buffer with known length).  With a sentinel-based range, the
length is unknown until the null terminator is reached.

### Possible outcomes

1. **Already works**: If `iconv_transcode_view` accumulates input into its
   staging buffer one byte at a time (feeding partial input to iconv), then
   null_sentinel_t just means "no more bytes" and the existing code handles it.

2. **Doesn't compile**: If the view template requires `std::ranges::sized_range`
   or uses `std::ranges::size()` somewhere, it won't work with null_term.

3. **Compiles but hangs/crashes**: If the view tries to compute input length
   upfront using `end - begin`, it will fail with a sentinel.

The fix (if needed) would be to ensure the view works byte-by-byte with any
input iterator + sentinel pair, not just random-access ranges.

Also test that `iconv_transcode_to(views::null_term(ptr), from, to)` works
(from Step 4), which may need to materialize the null-terminated range into a
contiguous buffer before calling iconv.

## Deliverables

- `tests/beman/transcode/iconv_null_term.test.cpp`:
  - Test: `views::null_term("Hello") | iconv_transcode("UTF-8", "UTF-32LE", buf)`
  - Test: `iconv_transcode_to<std::string>(views::null_term("Hello"), "UTF-8", "UTF-32LE")`
- Fixes to `iconv_transcode_view` if null_sentinel_t is not handled
- Register tests in CMakeLists.txt

## Procedure

1. Create branch `p4-step5-iconv-null-term` from `main`
2. Write test file `tests/beman/transcode/iconv_null_term.test.cpp`
3. Try to compile: `views::null_term("Hello") | iconv_transcode("UTF-8", "UTF-32LE", buf)`
4. If it compiles and produces correct output, the step is just the test
5. If it doesn't compile:
   - Check if `iconv_transcode_view` uses `std::ranges::size()` or
     `end - begin` anywhere
   - Fix to use iterator/sentinel pair without requiring sized_range
   - The view's staging buffer approach (accumulate bytes, call iconv when
     buffer is full or input exhausted) naturally handles sentinels
6. Test the bulk operation too: `iconv_transcode_to<std::string>(views::null_term(ptr), ...)`
   - This may need the bulk function to detect non-contiguous/non-sized ranges
     and materialize them into a `std::string` first
7. `make test` + `make lint`
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 6

Step 5 done, next read `docs/plans/p4-step6-error-coherence.md`.
