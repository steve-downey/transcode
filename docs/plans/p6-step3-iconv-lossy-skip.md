# P6-Step 3: One Lossy Policy — Skip, Do Not Replace

**Branch:** `p6-step3-iconv-lossy-skip`
**Depends on:** Steps 0 and 2
**Findings:** C-04, S-02
**Read first:** `docs/plans/phase6-index.md`, `docs/plans/p6-step2-iconv-error-model.md`

---

## Goal

Make the eager iconv helpers recover the way the lazy one does, and stop them
emitting bytes that are not valid in the encoding they claim to produce.

## Why

`iconv_bulk.hpp` writes a literal `'?'` on `EILSEQ` and again on `EINVAL`, at
lines 123, 136, 206 and 211:

```cpp
*out++ = '?';
--out_left;
```

`0x3F` is `'?'` in ASCII and in everything ASCII-compatible.  It is not `'?'`
in UTF-16LE, where the question mark is `3F 00`.  Transcoding invalid UTF-8 to
UTF-16LE through `iconv_transcode_to` can therefore return the single byte
`{0x3F}` -- an odd-length "UTF-16" string, which no UTF-16 reader can consume.
The helper produced malformed output while reporting success.

The header's own docblock at lines 79-84 says something else again:

> Input the conversion does not accept is skipped, as it is by
> `iconv_transcode_view`.

That sentence is already in the committed wording.  It describes the streaming
view accurately (`iconv_transcode_view.hpp:252-257` skips one byte and
continues) and the bulk helper not at all.  So this is not a case where the
implementation and the specification disagree about a decision -- it is a case
where the specification is right and the code drifted.

That also answers S-02.  The reason a user cannot swap the eager helper for the
lazy view without changing their output is this one difference; removing it
removes the inconsistency rather than documenting it.

Encoding a replacement *through* the destination converter would also produce
well-formed output, and it is what the review offers as the alternative.  It
loses on two counts: it makes every skip a second conversion call that can
itself fail, and it would put the eager helpers *further* from the streaming
view, which replaces nothing.  One policy, and it is the one already written
down.

## What to change

`include/beman/transcode/iconv_bulk.hpp`, in `iconv_transcode_to` and
`iconv_transcode_into`.

Delete the four `*out++ = '?'` writes and the `--out_left` / `output++`
bookkeeping that goes with them.  On `EILSEQ`, advance the input by one byte
and continue -- which the code already does immediately above each write.  On
`EINVAL` at end of input, drop the incomplete tail and stop, which is also
already there.

The buffer-growth blocks that exist only to make room for the `'?'` (lines
116-122, 129-135) go with it.  Do not leave them; a resize with nothing to
write into it is a puzzle for the next reader.

`iconv_transcode_to_or_error` does not replace anything -- it returns on the
first failure -- so it is untouched.

The docblock at lines 79-84 needs no change.  That is the point.

## Tests

`tests/beman/transcode/iconv_bulk.test.cpp`.

- **The finding, directly.**  Invalid UTF-8 to UTF-16LE through
  `iconv_transcode_to`.  Assert the result length is even and that decoding it
  with `whatwg_decode<codec::utf_16le>` yields no `U+FFFD` that the input did
  not put there.  This test fails on `main` with a one-byte result.
- **Eager and lazy agree.**  The same invalid input through
  `iconv_transcode_to` and through `iconv_transcode(...)` collected with
  `ranges::to`, asserting byte-for-byte equality.  This is the S-02 claim as a
  test rather than a paragraph, and it is the one that will catch the next
  drift.
- **`iconv_transcode_into` matches `iconv_transcode_to`** for the same inputs.
- An incomplete multibyte tail (`EINVAL`) produces no trailing byte.

Existing tests in this file that assert a `'?'` in the output are asserting the
bug.  They change; say so in the commit message rather than quietly editing
expectations.

## Done when

- No `'?'` is written by any iconv helper.
- Eager and lazy produce identical bytes for identical invalid input.
- `make test` and `make lint` pass.
