# P6-Step 2: The iconv Error Model

**Branch:** `p6-step2-iconv-error-model`
**Depends on:** Step 0
**Findings:** C-02, C-03, C-05
**Read first:** `docs/plans/phase6-index.md`

---

## Goal

Make every iconv failure either produce correct output or say what went wrong.
Today three of them do neither.

## Why

These are one finding wearing three hats: the adaptors treat "no bytes came
out" as "there are no more bytes," and that conflation is wrong in three
different places.

**A buffer too small for one output unit yields an empty range (C-02).**
`load()` at `iconv_transcode_view.hpp:242-243` breaks out on `E2BIG`, reaches
line 303 with `output_pos_ == output_end_`, flushes, produces nothing, and sets
`done_`.  Converting UTF-8 `"A"` to UTF-32LE through a one-byte buffer gives an
empty range and no error.  A zero-length buffer does the same.  The caller
cannot tell that from a successful conversion of empty input.

**The flush happens once and its result is discarded (C-03).**
`iconv_transcode_view.hpp:304-311` sets `flushed_ = true` *before* calling
`fns_.convert(handle_, nullptr, nullptr, ...)`, then ignores both the return
value and `errno`.  A stateful converter that returns `E2BIG` while still owing
shift-state bytes has those bytes truncated, and the `_or_error` variant
(`iconv_transcode_or_error_view.hpp:296-307`) does not report it either.

**One `iconv_open` failure has three different meanings (C-05).**  The
streaming view constructs `done_(handle == (iconv_t)-1)` and looks empty;
`iconv_transcode_to` returns an empty container (`iconv_bulk.hpp:90-91`);
`iconv_transcode_into` returns the iterator unchanged (`:183-184`); and
`iconv_transcode_to_or_error` reports `invalid_sequence` (`:254-255`), which is
a lie about what happened.  `error.hpp:67-71` has no enumerator that means
"the descriptor would not open," though `papers/transcode-view.md:530` claims
`iconv_error` covers it.

**And any unexpected `errno` is misfiled.**  The branches at
`iconv_bulk.hpp:125`, `:207`, `:277` and `iconv_transcode_view.hpp:252` are
commented `EILSEQ` or `EINVAL` but are reached for *anything* that is not the
one errno the preceding branch tested.  `EBADF` from a closed descriptor
currently means "skip a byte and carry on."

## What to change

### `error.hpp`

Add two enumerators to `iconv_error`:

- `open_failed` — the conversion descriptor could not be opened, which is
  `iconv_open` failing, and covers an encoding name the implementation does not
  support.
- `system_error` — the conversion failed in a way POSIX does not enumerate for
  `iconv`.  This is the honest home for an unexpected `errno`.

Add matching `\row` / `\cell` pairs to the existing
`\libtab2[transcode.errors.iconv]` table.  Keep the table's voice: each cell
says what the condition *is*, with the errno in parentheses.

`open_failed` has no errno of its own -- `iconv_open` sets `EINVAL` for an
unsupported pair, which would collide with `incomplete_sequence` if it were
mapped by errno.  Say so in the cell; it is the kind of detail that looks like
an oversight if it is not stated.

### The two streaming views

`iconv_transcode_view.hpp` and `iconv_transcode_or_error_view.hpp` carry the
same `load()` shape and both need the same three fixes.  Do them together and
keep the two functions structurally parallel -- they are read side by side.

- **Flush.**  Replace `bool flushed_` with a three-state member (`not_started`,
  `in_progress`, `done`).  Enter it when input is exhausted, call the flush,
  and *check the result*: on `E2BIG`, yield what came out and stay
  `in_progress` so the next `load()` resumes; on success move to `done`; on
  anything else apply the view's error policy.  The state must survive the move
  constructor and move assignment, which already copy `flushed_`.
- **Buffer too small.**  Distinguish `E2BIG` with bytes produced (normal
  batching, keep going) from `E2BIG` with **zero** bytes produced and input
  remaining (the buffer cannot hold one output unit).  The second is not
  end-of-input.  Define `iconv_min_buffer_size` as an exported constant and
  state it as an `\expects` on the view constructor and on the
  `iconv_transcode` / `iconv_transcode_or_error` closures.  The `_or_error`
  view reports `output_full`.  The plain view has no error channel, so the
  precondition is its answer -- but it must not silently truncate: assert in a
  checked build rather than ending the range.
- **Unexpected errno.**  Test `EILSEQ` explicitly instead of falling into it.
  Anything that is neither `E2BIG`, `EINVAL` nor `EILSEQ` is `system_error`:
  reported by the `_or_error` view, and terminating the range for the plain one
  rather than being skipped as if it were bad input.

### The bulk helpers

`iconv_bulk.hpp`.  `iconv_transcode_to_or_error` returns `open_failed` rather
than `invalid_sequence` at line 254, and `system_error` rather than
`incomplete_sequence` for an unexpected errno at line 277.  The two lossy
helpers keep returning an empty container and an unchanged iterator, but their
docblocks stop describing that as if it were a result and say plainly that a
failed open is not distinguishable in these overloads -- and point at the
`_or_error` form for a caller who needs to know.

The `'?'` writes in this file belong to Step 3.  Leave them; do not fix two
things in one diff through the same lines.

## Tests

`tests/beman/transcode/iconv_mock.hpp` is the injection point -- the views take
`IconvFns` as a template parameter precisely so a test can supply its own.

- A mock whose flush returns `E2BIG` on the first call and succeeds on the
  second, asserting every owed byte arrives.
- A mock returning an errno that is none of the three, asserting
  `system_error` from the `_or_error` view and a terminated range from the
  plain one.
- A mock whose `open` fails, asserting `open_failed` from both `_or_error`
  forms.
- Real-iconv cases in `iconv_transcode.test.cpp` for buffer sizes zero, one,
  `iconv_min_buffer_size - 1` and `iconv_min_buffer_size`, converting UTF-8 to
  UTF-32LE so one input byte needs four output bytes.

## Done when

- A one-byte buffer no longer yields a silent empty range.
- A converter that needs two flush calls gets two.
- `open_failed` and `system_error` exist, are reachable, and are covered.
- `make test`, `make lint` and `make coverage` pass; new branches are covered.

## Hand off to Step 8

`papers/transcode-view.md` prints the `iconv_error` enumeration as hand-written
prose in its Error Handling section, and claims at line 530 that the enum
already covers a descriptor that would not open.  Regenerating the wording will
not touch either.  Note the two new enumerators in the pull request so Step 8
picks them up.
