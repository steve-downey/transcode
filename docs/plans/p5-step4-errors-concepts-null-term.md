# P5-Step 4: Markup — Errors, Range Requirements, `null_term`

**Branch:** `p5-step4-errors-concepts-null-term`
**Depends on:** Steps 1 and 3
**Read first:** `docs/plans/phase5-index.md`, `docs/wording-outline.md`,
`~/src/specgen/main/docs/user-guide.md` (element vocabulary, placement markers)

---

## Goal

Write the first real wording markup, for the three smallest clauses:
`[transcode.errors]`, `[transcode.reqs]`, `[null.term]`.  This step also
establishes the markup loop that Steps 5-9 repeat, so it is written out here
once and referred to afterwards.

## The markup loop

Steps 4 through 9 are all the same six moves.  Later step documents say only
what is different.

1. **Place the section markers.**  A draft-form `// \rSec2[stable.name]{Title}`
   comment above the definition region for the clause; `// \ref{stable.name},
   group` comments inside a class body to route its members.  Depths and names
   come from `docs/wording-outline.md` and are not invented at the keyboard.
2. **Write the description elements** in `//!` docblocks on the out-of-line
   definitions: `\constraints`, `\mandates`, `\expects`, `\effects`, `\ensures`,
   `\returns`, `\throws`, `\complexity`, `\remarks`.  Authored order does not
   matter (specgen canonicalizes and notes), but write them in canonical order
   anyway.  Prefer `\effects-equiv` / `\returns-equiv` where the body already
   *is* the specification — the point of the tool is that the wording cannot
   drift from the code.
3. **Mark the non-wording declarations**: `\expos` for exposition-only,
   `\omit` for what the specification does not mention at all, `\merge` for a
   declaration another entity already specifies, `\seebelow` / `\impdef` for
   masked types and conditions, `\also` / `\group` to join overload signatures
   into one item.
4. **Regenerate**: `make wording`.
5. **Drive validation to zero for this clause**:
   `specgen render --from-ir <ir> --backend mpark --validate`.  Coverage errors
   mean a declaration has no home; leakage errors mean the wording names
   something the reader cannot see.  Both are real; fix the header, not the
   report.  The `ranges` findings from index U1 are the one allowed exception,
   and they are listed in the step's commit message so the allowlist stays
   visible.
6. **Commit headers and `papers/wording/*.md` together.**  A commit where they
   disagree fails `make wording-check`, which is the point.

## What is different here

- **`whatwg_error` and `iconv_error`** are enumerations; their wording is a
  table of enumerator meanings rather than per-function elements.  Check whether
  a two-column `\lib2dtab2` is the right rendering for "enumerator / meaning"
  before hand-rolling prose.
- **`legacy_byte_range` and `unicode_scalar_range`** are namespace-scope
  concepts.  They render as ordinary synopsis declarations; the wording is the
  paragraph explaining what a conforming range must satisfy.  The negative
  compile tests in `tests/beman/transcode/` already state these constraints
  precisely — the wording should say the same thing in the same order, and any
  place it cannot is a sign the concept is doing something the specification
  cannot express.
- **`null_term_view`** is the header that was proved end to end in Step 1;
  replace the placeholder docblock from that step with the real wording.
  `null_sentinel_t`'s hidden-friend `operator==` is a customization point and is
  defined in-class by design (`CLAUDE.md`); confirm specgen routes it as a
  hidden friend rather than reporting it.
- This step is also where the **`constify()` consteval discipline meets the
  wording**: every element that says an operation is usable in constant
  evaluation should have the corresponding consteval test already, per
  `CLAUDE.md`.  Where it does not, add it here rather than writing wording the
  tests do not back.

## Acceptance

- `papers/wording/transcode.errors.md`, `transcode.reqs.md` and `null.term.md`
  exist and are non-empty.
- `--validate` on the affected headers: no coverage findings, no `detail`
  leakage findings, only index U1's `ranges` noise.
- `make wording-check`, `make lint`, `make test` green.
- The paper builds and the three clauses appear in it.
