# P5-Step 5: Markup — Encodings, Labels, BOM Sniffing

**Branch:** `p5-step5-codec-labels-sniff`
**Depends on:** Step 4
**Read first:** `docs/plans/p5-step4-errors-concepts-null-term.md` ("the markup loop")

---

## Goal

Wording for `[transcode.codec]`: the `codec` enumeration, the label lookup
`get_encoding`, and `sniff_encoding`.

## What is different here

- **The enumeration has 39 enumerators and is normatively the WHATWG list.**
  The wording should not restate the table; it should say that each enumerator
  names the WHATWG encoding of the corresponding name and cite the standard.
  The paper's bibliography already has `@whatwg-encoding`; specgen's
  `\iref{stable.name}` is for standard subclauses, so a citation to a non-ISO
  document belongs in authored prose in the paper around the clause, not inside
  a generated element.  Decide and record how a normative reference to WHATWG is
  spelled; Step 10 has to render it.
- **`detail/labels.hpp` is generated** by `tools/generate_labels.py` and must not
  be hand-edited (`CLAUDE.md`).  If `get_encoding`'s declaration needs markup, it
  needs it in the *generator template*, not the output — or `get_encoding` moves
  to a hand-written header that includes the generated table.  Prefer the move:
  a generator that emits specification markup is a generator with two jobs.
- **`sniff_encoding` returns `optional<codec>`** and is specified against WHATWG
  §8.2.  It is a good candidate for `\returns-equiv` if the body is a single
  return, and a good candidate for an authored table of BOM → codec otherwise.

## Acceptance

As Step 4, for `papers/wording/transcode.codec.md`.
