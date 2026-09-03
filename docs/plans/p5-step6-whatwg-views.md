# P5-Step 6: Markup — WHATWG Decode and Encode Views

**Branch:** `p5-step6-whatwg-views`
**Depends on:** Step 5
**Read first:** `docs/plans/p5-step4-errors-concepts-null-term.md` ("the markup loop")

---

## Goal

Wording for `[transcode.whatwg.decode]` and `[transcode.whatwg.encode]`: eight
class templates (lossy and `_or_error`, each with a `random_access_` variant)
and their pipe closures.

## What is different here

- **This is the bulk of the wording and the bulk of the validation findings.**
  The measurement in the phase index — 64 findings on today's unmarked
  `whatwg_decode_view.hpp` — is this clause's starting worklist.  Expect the
  same order of magnitude for the encode side.  It is reasonable to split this
  step into decode and encode branches.
- **The `random_access_` specializations exist for O(1) single-byte codecs.**
  The specification question is whether they are separate specified entities or
  an unobservable optimization.  If the latter, they are `\omit`ted and the
  wording says the view models `random_access_range` when the codec is
  single-byte — which is how the draft would say it.  Decide this before writing
  a line of markup; it changes the clause structure, not just its prose.
  Whatever is decided, the answer belongs in `docs/wording-outline.md` and in
  the paper's design section, because it is a design claim, not a formatting
  one.
- **Iterator members are where `\effects-equiv` earns its keep**, and also where
  it is most dangerous: an extracted `load()` body that still contains codec
  state machines would put implementation into the wording.  Step 3 task 2 is
  what makes extraction safe here; if it was deferred for some codec, that
  codec's members take authored `\effects` prose instead.
- **`enable_borrowed_range` specializations** sit in `namespace std::ranges` in
  the encode header.  Check how specgen renders a namespace-scope variable
  template specialization outside the header's own namespace before assuming it
  needs `\omit`.

## Acceptance

As Step 4, for `papers/wording/transcode.whatwg.decode.md` and
`transcode.whatwg.encode.md`.
