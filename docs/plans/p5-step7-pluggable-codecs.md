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
