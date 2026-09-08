# P5-Step 7: Markup — Codec Requirements and Pluggable Views

**Branch:** `p5-step7-pluggable-codecs`
**Depends on:** Step 6
**Read first:** `docs/plans/p5-step4-errors-concepts-null-term.md` ("the markup loop")

---

## Goal

Wording for `[transcode.codec.reqs]`, `[transcode.decode]` and
`[transcode.encode]`: the `decode_codec` / `encode_codec` /
`flushable_decode_codec` / `random_access_decode_codec_type` concepts, the
`decode_result` / `encode_result` structs, and the codec-parameterized views.

## What is different here

- **The concepts are the extension point, so their wording is the contract a
  user-written codec must satisfy.**  This is a *Requirements* clause in the
  draft's sense: what `decode_one(iter, sentinel)` must return, what `flush()`
  means, when `decode_byte(byte)` may be called.  The existing negative compile
  tests are the specification's test suite; every constraint they pin should be
  a sentence here.
- **`decode_result` and `encode_result`** (`detail/codec_result.hpp`) are part
  of that contract and so are specification, not detail — Step 3 task 1 should
  already have moved them.  If it did not, do it here.
- **The pluggable views are structurally parallel to the WHATWG views** but
  parameterized on a codec *object* rather than an enumerator.  Where the
  wording is genuinely the same, say so by cross-reference (`\iref`) rather than
  duplicating paragraphs; where it is not, the difference is worth a sentence in
  the paper's design section too.

## Acceptance

As Step 4, for `papers/wording/transcode.codec.reqs.md`,
`transcode.decode.md`, `transcode.encode.md`.

---

## Outcome (2026-09-08)

All three clauses are written and `--validate` reports nothing about any of
their entities.  The document's findings go from 79 to 22, and every one of
the 22 belongs to Step 8's pipeline closures or Step 9's iconv surface.

**The clause names are the outline's**, not this plan's.  This document was
written before Step 2 settled `transcode.custom.*` for the codec-parameterized
family (`docs/wording-outline.md`, "`transcode.custom.*` rather than
`transcode.decode` / `transcode.encode`"), so its Goal and Acceptance still say
`transcode.codec.reqs`, `transcode.decode` and `transcode.encode`.  The outline
is the contract; these are `[transcode.custom.reqs]`,
`[transcode.custom.decode]` and `[transcode.custom.encode]`.

**`[transcode.custom.reqs]` is a requirements clause**, and the four concepts
are the contract a user-written codec satisfies: what `decode_one` reads and
advances, that it is called only with a non-empty range, that a codec is a
value and keeps its own state, what `decode_byte` means and what it buys, when
`flush()` is called and what `nullopt` means from it.  `decode_result` and
`encode_result` are described by their classes' own paragraphs, which needed
specgen#82: a public data member is specification, and both §9 checks used to
say otherwise.

**The views cross-reference rather than repeat.**  `decode_view` is
`whatwg_decode_view` with the codec supplied as a value rather than named by an
enumerator, and its clause says so with an `\iref` instead of a second copy of
the same three paragraphs.  Where the difference is real it is stated: the
random-access property is a property of the *codec* here
(`random_access_decode_codec_type`) rather than of the enumerator, and
`flushable_decode_codec` gives the pluggable decode an end-of-input step the
WHATWG views have no equivalent of.

The rest is the Step 6 pattern, unchanged: `\omit` on the `random_access_`
specialization and the closure struct, `\expos` with bare `\seebelow` on each
nested iterator, `\seebelow` on the factory functions so they return
*see below*, `\returns-equiv` and `\effects-equiv` where the body is the
specification, authored prose everywhere `load()` drives it.

Steps 8 and 9 are what remain before the paper is assembled.
