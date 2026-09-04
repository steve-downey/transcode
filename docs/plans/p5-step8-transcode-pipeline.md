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
