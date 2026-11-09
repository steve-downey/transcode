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

---

## Outcome (2026-09-07)

All three clauses are written, and `--validate` reports nothing about any of
their entities.

- **`[transcode.codec]`** — a `\ref` group header and one *Remarks* on the
  enumeration, saying that each enumerator names the encoding of the same name
  in the WHATWG Encoding Standard and that the table is not restated here.
  [encoding-standard-reference](../decisions.md#encoding-standard-reference)'s question -- how the normative reference is spelled -- is answered in
  `docs/wording-outline.md` where [encoding-standard-reference](../decisions.md#encoding-standard-reference) is: named in prose in the wording, a
  dated [intro.refs] entry in the paper, and whether SG16 accepts a normative
  reference to a living standard at all is a question for the paper's prose.

  The clause repeats all 39 enumerators above that one paragraph, because a
  documented entity renders its declaration.  The draft would print the
  declaration once, in the synopsis, and give the clause the prose alone; a
  one-line `\verbatim-itemdecl` (`enum class codec;`) would do that here at the
  price of one authored spelling.  Left as it is, and left to Step 10 to judge
  with the whole paper in front of it -- it is a presentation question, and it
  is the same question for every long declaration in the phase.

- **`[transcode.codec.label]`** — `get_encoding` moved out of the generated
  `detail/labels.hpp` into a hand-written `label.hpp`, which is what this
  step's plan preferred: a generator that emits specification markup is a
  generator with two jobs, and the one thing a generated file must never need
  is an edit.  `tools/generate_labels.py` now emits the table and nothing else,
  and its test says so rather than asserting the function is in there.  The
  generated header also stopped including `whatwg_decode_view.hpp` for the sake
  of the `codec` enum, which is now all it includes.

- **`[transcode.codec.sniff]`** — a *Returns* itemization over the three byte
  order marks, and a *Remarks* saying what the function deliberately does not
  do: the fallback half of WHATWG §8.2 is a question about the document and its
  transport, and the mark is left in the range because a decode view strips a
  leading U+FEFF itself.

Both functions carry their markup on the **definition**, not the declaration:
a docblock on a function declaration is an error, because the definition is
what places a function's wording (design §3.3).  In a followed header both are
in the same file, so this is easy to get wrong and loud when you do.

`get_encoding` moving is the only change to the library, and it is a file move:
no signature, no name, no namespace.  `label.hpp` joins the synopsis region in
`transcode.hpp` next to `codec.hpp`, and `detail/labels.hpp` leaves the
implementation list, since `label.hpp` includes it.

Step 6 is next, and is the largest of the markup steps.
