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
   report.  There is no allowed exception: specgen#3 landed upstream on 2026-09-04,
   so the `ranges` findings that used to be excused no longer occur.
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
- **`null_term_view`** is the header Step 1 proved the harness on, and it
  already carries markup: `\expos` on `ptr_`, section `[null.term.view]` with
  the three member descriptions, and `\omit` on the deduction guide, on
  `detail::null_term_fn` / `null_term_adaptor`, and on `views::null_term`.  Two
  of those omissions are placeholders this step has to replace, and both are
  blocked upstream:
  - `null_sentinel_t`'s hidden-friend `operator==` is **done**.  The header
    carries the gathered `\rSec2[null.term.syn]` region, a
    `\rSec2[null.term.sentinel]` section and a `\ref{null.term.sentinel}`
    group in the class body; the clause renders `operator==` with its *Returns*
    and `--validate` is clean.  #45 is fixed too, so that clean result now
    means the clause is covered rather than unchecked.
  - `views::null_term` is `\omit`ted because its type is
    `detail::null_term_adaptor` and there is no way to render it as
    `inline constexpr unspecified null_term;`.  Both halves are fixed --
    specgen#24 for the marker, specgen#55 for applying it inside a gathered
    region -- so the `\omit` is gone and the synopsis carries
    `inline constexpr $unspecified$ null_term;`.  What is left for
    `[null.term.adaptor]` is its prose: a `\rSec2` section and the wording for
    what the adaptor does.  That is authorship, not tooling.

  Neither is a header problem, so do not refactor around them; land the rest of
  the clause and pick these up when the upstream items do.
- This step is also where the **`constify()` consteval discipline meets the
  wording**: every element that says an operation is usable in constant
  evaluation should have the corresponding consteval test already, per
  `CLAUDE.md`.  Where it does not, add it here rather than writing wording the
  tests do not back.

## Acceptance

- `papers/wording/transcode.errors.md`, `transcode.reqs.md` and `null.term.md`
  exist and are non-empty.
- `--validate` on the affected headers: no coverage findings, no `detail`
  leakage findings, and no `ranges` noise -- specgen#3 is fixed, so the clean bar is
  absolute.
- `make wording-check`, `make lint`, `make test` green.
- The paper builds and the three clauses appear in it.

---

## Outcome (2026-09-06)

All three clauses are written and generated.  Two of them had no representable
form in specgen when the step started; both gaps were filed and fixed upstream
rather than worked around here, so nothing in this step is a header problem and
no header was changed to suit the tool ([detail-in-wording](../decisions.md#detail-in-wording)).

- **`[transcode.reqs]` — done.**  `concepts.hpp` carries the `\rSec2` marker,
  `\expos` on `detail::legacy_byte_type`, and a *Remarks* on each concept.  The
  clause renders the exposition-only conjunct spelled out, then each concept
  with its paragraph; `--validate` is clean, and
  `papers/wording/transcode.reqs.md` is the first `<transcode>` fragment in
  `wording.mk`.

  Neither paragraph restates its definition — the definition is generated from
  the header and says what it says.  They carry what it cannot: why an array
  type is excluded (the terminating null character of a string literal is not
  transcoded with the rest of the array), and that `unicode_scalar_range`
  constrains the type of a range and not its values, so that each element being
  a Unicode scalar value is a precondition of the operations that encode it.
  That is the same order the negative compile tests state the constraints in,
  as this step asked.  Neither element claims anything about constant
  evaluation, so no `constify()` test is owed; `concepts.test.cpp` is already
  entirely `static_assert`s.

- **`[transcode.errors]` — done, at the price of a specgen fix.**  An
  enumeration could not be specified at all: a docblock on one was rejected as
  an unsupported entity kind, a gathered `.syn` region rendered the declaration
  and still refused the docblock, a detached docblock after the enum was
  dropped silently, and a `\verbatim-itemdecl` block before it attached to the
  enum and hit the same error.  There was no fourth spelling, and no way to
  write the clause.  specgen #70 makes an enumeration a documented namespace
  entity like the others, so `error.hpp` carries one docblock per enum and the
  clause renders all three declarations with their enumerator meanings.  The
  meanings are an authored `\item` list rather than the draft's two-column
  Constant/Meaning table: `\lib2dtab2` is a two-*dimensional* table, a
  row-heading column plus two columns, so it is the wrong shape.  A flat
  two-column table is specgen#74, filed on the recheck and not blocking -- the
  list says the same thing in the same order.

- **`[null.term.adaptor]` — done, at the price of the other one.**  A docblock
  on `views::null_term` rendered its declaration in the gathered synopsis and
  discarded its description without a diagnostic.  specgen #71 routes a
  folded-in declaration's wording the way a folded-in class's members are
  already routed, so the header now carries a `\ref{null.term.adaptor}` group
  header — which renders in the synopsis, as the draft writes it — and the
  prose reaches the clause.  The declaration stays where the draft puts it,
  `inline constexpr $unspecified$ null_term;` in `[null.term.syn]`.

  Writing that paragraph turned up one thing worth knowing: `views::null_term`
  applied to an array yields `null_term_view<const T*>`, not
  `null_term_view<T*>`, because the array overload casts to `const T*`.  The
  wording says so, since the wording says what the code does.  Whether the
  added `const` is deliberate is a question for the library, not for the
  clause.

Generating the clause also caught a drift the paper had already: its "Concepts"
section shows both concepts constrained on `ranges::input_range`, and
`legacy_byte_range` is defined on `ranges::range`.  The generated wording is now
the truth, so the authored prose is what is wrong; it is recorded as a task in
`docs/plans/p5-step10-paper-assembly.md` rather than fixed by changing a concept
to match a paragraph.

Both upstream items are described in `docs/plans/phase5-index.md` under
"External dependencies".  They merged on 2026-09-07, and regenerating needs a
specgen at or after `b746da6`; an installed one from current specgen `main`
reproduces every committed fragment byte for byte.  specgen#69 is the fifth instance of the
gathered-region pattern that section had recorded as closed out, which is worth
knowing for Steps 5-9: they gather a much larger surface, a dropped element is
silent, and a clean `--validate` does not prove an authored paragraph arrived.
Read the fragment.

**Step 5 can start.**
