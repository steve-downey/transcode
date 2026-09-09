# P5-Step 8: Markup — Pipelines and Eager Transcoding

**Branch:** `p5-step8-transcode-pipeline`
**Depends on:** Step 7
**Read first:** `docs/plans/p5-step4-errors-concepts-null-term.md` ("the markup loop")

---

## Goal

Wording for `[transcode.pipeline]` (`transcode<From, To>`,
`pluggable_transcode`) and `[transcode.string]` (`transcode_string`, including
the label-string overload).

## What is different here

- **The composed views are specified in terms of the views they compose.**
  `transcode<From, To>` is `whatwg_decode<From> | whatwg_encode<To>`; that is
  exactly what `\effects-equiv` or a *Returns: Equivalent to:* element should
  say, and it is the cheapest correct wording in the phase.  Resist writing it
  out longhand.
- **`transcode_string` is eager and allocates**, so its wording needs
  *Complexity* and *Throws* elements that the lazy views do not.  It is also the
  one entry point that takes runtime labels, so its precondition on an unknown
  label (and the `optional` overload's behaviour) has to be explicit.
- **`detail/bulk_transcode.hpp`** holds the fast paths.  They are unobservable
  and should be `\omit`ted, with a *Remarks* or a design-section sentence noting
  that a bulk implementation is permitted — the wording must not accidentally
  require an element-at-a-time implementation.

## Acceptance

As Step 4, for `papers/wording/transcode.pipeline.md` and
`transcode.string.md`.

---

## Outcome (2026-09-08)

Both clauses are written and `--validate` reports nothing about either.  The
document's findings go from 22 to 16, and all sixteen are Step 9's: the iconv
views, their closures, and the one `detail::` qualifier left in the document,
which is `iconv_input_buf` reaching a signature there.

**A pipeline is specified as a composition**, which is what it is:
`transcode<From, To>` is expression-equivalent to
`E | whatwg_decode<From> | whatwg_encode<To>`, and the clause says so rather
than restating what decoding and encoding do.  The error behaviour follows from
the parts and is stated as following from them: a byte sequence `From` does not
allow decodes to U+FFFD, and it is that U+FFFD which then gets encoded, so a
transcode never drops an element for a decode error.

`pluggable_transcode` is the same sentence with codecs supplied as values, and
its clause is an `\iref` to the one above rather than a second copy.

**`transcode_string` is the eager form**, and its wording says what it is for:
the same pipeline run to completion into a `string`.  The label overload is the
one a program uses when the encodings are a runtime choice -- a `Content-Type`
header, a command-line option -- which is what labels are for, and its
`nullopt` is the label lookup's, cross-referenced rather than restated.

Two markup notes worth carrying to Step 9:

- A second element of the same kind on one declaration is a *warning*, not an
  error, and both are kept -- so a duplicate `\remarks` silently produces two
  *Remarks* paragraphs.  A blank `//!` line is how one element gets two
  paragraphs.
- The closure types are `\omit`ted here as everywhere else, which is what makes
  `operator|` and `operator()` stop being coverage findings.  Step 9's iconv
  closures are the last two.
