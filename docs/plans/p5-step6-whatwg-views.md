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

---

## Outcome (2026-09-08)

Both clauses are written, and `--validate` reports nothing about either view,
its iterator, or its adaptor objects.  The document's findings go from 133 to
79: what remains is Steps 7-9's, and one of the two entities the count blames
on this clause was the `detail::` qualifier W1 took away.

**W1 is carried out.**  `random_access_whatwg_decode_view` and its encode twin
are `\omit`ted, and each clause says instead that the view models
`random_access_range` when the codec decodes one byte to one scalar value and
the base range does.  That is a design claim, so it belongs in the paper's
design section as well; Step 10 puts it there.  It also removed the last
surviving `detail::` qualifier from the document, which reached the wording
through those two views' requires-clauses -- the outline predicted exactly
that.

**A view's iterator is exposition-only, declared and not defined.**

    class $iterator$; // exposition only

which is [range.transform.view]'s shape and the only one that keeps a view's
private state, and the implementation types it names, out of the header
synopsis.  It needed specgen#80: `\expos` did not reach a nested class, so the
class was dropped as an ordinary private member while every `begin`, `end` and
`operator++` signature went on naming it -- and nothing reported that, because
the private-member check does not look at a nested class.  The iterator's own
members are exposition with it, so an extracted body says `$value$`.

**The closure types are omitted and the adaptor objects masked.**  Each clause
names `whatwg_decode<C>` / `whatwg_encode<C>` and their `_or_error` twins as
`inline constexpr $unspecified$`, with the expression-equivalence the draft
writes for a range adaptor object.  The pre-unification aliases are `\omit`ted:
they are transition spellings, not API.

**Bodies carry the wording where the body is the specification.**
`\returns-equiv` on `base()` and `operator*`, `\effects-equiv` on the
post-increments; authored prose everywhere the body is a state machine, which
is every `load()`-driven operation.  Step 3's pushdown is what made that a
choice rather than a hazard: the bodies that remain in the views are the ones
worth extracting.

Two things for the steps that follow:

- A clause's sub-structure can be a `\rSec3` inside the same fragment, which is
  how each iterator got its own subclause without adding a fragment or moving a
  stable name.
- The findings that remain are all `encode_view` / `decode_view` -- the
  *pluggable* views, Step 7 -- plus the iconv surface, Step 9.
