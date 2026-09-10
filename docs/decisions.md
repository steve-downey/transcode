# Decision Log — generated wording in beman.transcode

Convention: each section answers one **question**, identified by a slug named
for the question, never for the chosen answer (so the slug survives its answer
reversing: `#detail-in-wording`, not `#expos-everything`). All references
anywhere in the repo render as links to these anchors. Open questions live in
the same namespace with `Status: OPEN`; answering one graduates it in place —
the slug and every existing link stay valid. Divergences append dated entries
to the implicated question's Log; they do not get their own files, and they do
not get their own numbers.

Entry shape: Question / Status / Decided by / Decision / Why / Log.

Defects in `beman.specgen`, which this phase depends on, are **not** tracked
here. They have real identifiers already — `steve-downey/specgen` issue numbers
— and those are what `docs/plans/phase5-index.md` cites. An external number is
a name a reader can follow; an internal serial is not.

---

## document-unit

**Question:** What is one specgen document — a proposed standard header, or a
file?
**Status:** DECIDED 2026-09-03, amended 2026-09-07
**Decided by:** Phase 5 planning; amendment forced by specgen#77.
**Decision:** One document per **proposed standard header**. The paper proposes
`<transcode>` and `<null_term>`; each maps to one spec-facing header, and that
header is what specgen runs over.
**Why:** The draft's unit is the header — `[transcode.syn]` lists everything
`<transcode>` declares — so the tool's unit has to be the same or the synopsis
cannot be generated at all.
**Log:**
- 2026-09-03 — Adopted, carrying a fallback: one document per header *family*
  with a hand-authored `[transcode.syn]`, because a deduction guide corrupted a
  gathered region (specgen#22).
- 2026-09-05 — Fallback retired; specgen#22 fixed. `<null_term>` renders a real
  gathered `[null.term.syn]` beside its per-class clauses, validating clean.
- 2026-09-07 — **How a document reaches more than one file.** specgen used to
  process only declarations located in the main file, which would have made this
  decision "put the whole of `<transcode>` in one 3,500-line file". specgen#77
  makes a gathered `.syn` region gather the declarations of the headers
  `#include`d inside it, so `transcode.hpp` stays the umbrella it already was
  and the sixteen component headers stay where they are. The region is the
  statement of which includes are the specification surface; `detail/` includes
  sit outside it. That is [detail-in-wording](#detail-in-wording) again — the
  tool renders what the library writes — and it is why Step 3's task 4 cost a
  rewrite of one umbrella rather than a rewrite of the library.

---

## fragment-checkin

**Question:** Are the generated fragments committed, or built on demand?
**Status:** DECIDED 2026-09-03
**Decided by:** Phase 5 planning.
**Decision:** Committed, and checked. `make wording` regenerates
`papers/wording/*.md`; `make wording-check` regenerates into a temporary
directory and diffs.
**Why:** Committing keeps the paper buildable on a machine with no specgen and
no Clang — the normal case for a co-author — and makes a change to the wording
show up as a diff in a pull request rather than only as a different PDF.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-07 — Working as intended: an upstream change to the template head's
  namespace drop moved the committed fragments, and the move was visible in the
  diff.
- 2026-09-09 — The CI half of the check is not `wording-check`; see
  [drift-gate](#drift-gate).

---

## transclusion-mechanism

**Question:** How does generated wording get into the paper?
**Status:** DECIDED 2026-09-03
**Decided by:** Phase 5 planning.
**Decision:** Native pandoc multi-file input. `base.mk` builds its pandoc
command from `$(filter %.md, $^)`, so a target with several markdown
prerequisites is concatenated in prerequisite order. The paper targets take the
fragment list as ordered prerequisites; the order comes from the manifest
`specgen render --split` prints.
**Why:** No new markdown preprocessor, and no include syntax to invent.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-08 — Consequence realised in Step 10: prerequisite order *is* document
  order, so a section cannot be both authored in `transcode-view.md` and printed
  after the generated clauses. Everything that must follow the wording moved to
  `papers/parts/tail.md`.

---

## fragment-location

**Question:** Where do the generated fragments live?
**Status:** DECIDED 2026-09-03
**Decided by:** Phase 5 planning.
**Decision:** `papers/wording/`.
**Why:** `flat.mk` turns every `papers/*.md` into its own paper; a subdirectory
is invisible to that wildcard.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-08 — The same wildcard reasoning applies to `papers/parts/`, which
  holds the authored tail for the same reason.
- 2026-09-09 — The directory also holds authored files (`README.md`,
  `specgen-ref`). `generate.sh --authored` is the one list saying which files
  are not output; both its own cleanup and `wording-check`'s diff read it,
  after a wildcard cleanup deleted the README and the diff then called the
  fragments stale.

---

## stable-name-roots

**Question:** What do the proposed clauses' stable names root at?
**Status:** DECIDED 2026-09-03
**Decided by:** Phase 5 planning.
**Decision:** `transcode.` for `<transcode>` and `null.term` for `<null_term>`,
spelled explicitly with `--root` rather than left to specgen's derivation from
the common prefix.
**Why:** Derivation is implicit and would move the root fragment's filename when
the clause tree changes.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-10 — The roots gained a second consumer: `generate.sh` passes each to
  `specgen render --new-root`, which is what keeps a clause this paper *adds*
  from rendering as an mpark `.sref` and linking to a c++draft page that does
  not exist. Adding a third proposed header means adding its root to
  `paper_roots` beside its entry in `spec_headers`.

---

## editing-instruction-form

**Question:** Are the editing instructions generated (`--paper` mode) or
authored?
**Status:** DECIDED 2026-09-03
**Decided by:** Phase 5 planning.
**Decision:** Authored. No `--paper` mode.
**Why:** `--paper` wraps everything in `::: add` and underlines it. This paper
adds whole new clauses; an authored instruction ("Add a new clause [transcode]
as follows:") reads better than several hundred underlined lines. The switch is
one flag if that judgement changes.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-08 — Carried out in Step 10: the paper carries the header additions
  for [headers], the WHATWG entry for [intro.refs], the [version.syn] macro, and
  the instruction to add clause [transcode]. Those are the parts specgen cannot
  know.

---

## detail-in-wording

**Question:** What happens when a `detail::` name reaches a spec-visible
signature?
**Status:** DECIDED 2026-09-03, **reversed 2026-09-05**
**Decided by:** Phase 5 planning; reversal by Steve Downey during Step 3.
**Decision:** The name renders exposition-only or `unspecified`, and **the
header keeps the spelling it wants**. The tool renders what the library writes;
the library does not move for the tool.
**Why:** A `detail::` name reaching the wording is a real finding — the
published text must not name something the reader cannot see. What was wrong in
the original decision was the *remedy* it accepted: it said such a name "either
moves out of `detail` … or is marked `\expos`", and relocating a helper out of
`detail` changes name lookup, ADL and the library's effective public surface —
a real change to a library, taken on for a documentation tool. It was recorded
as the single largest source of the refactoring in Step 3, which is the clearest
sign it was pointed the wrong way.
**Log:**
- 2026-09-03 — Adopted in the form that permitted relocation.
- 2026-09-05 — Reversed. Marking the six concepts `\expos` in the headers that
  declare them took the qualifier findings from nine to three, with nothing
  moved and nothing renamed. specgen#36 is what made it cheap: `\expos` reaches
  entities declared in an included `detail/` header.
- 2026-09-05 — The three survivors were class-head constraints, closed by
  specgen#48. Satisfied in full by six `\expos` comments and nothing else.
- 2026-09-07 — Applied a second time, to a different question: see
  [document-unit](#document-unit)'s 2026-09-07 entry. The umbrella moved, the
  library did not.
- 2026-09-09 — Applied a third time, and declined a fixit that would have broken
  it: specgen#93 offered `\expos` on a generated codec table as the way to
  silence a false positive, which would have asserted that a file the document
  deliberately does not reach is an exposition-only part of the specification.
  Filed instead; fixed upstream.

---

## random-access-specialization

**Question:** Are the `random_access_` view specializations separately
specified entities?
**Status:** DECIDED 2026-09-03, carried out 2026-09-08
**Decided by:** Wording outline; carried out in Step 6.
**Decision:** No. Each view's clause says instead that it models
`random_access_range` when the codec decodes one byte to one scalar value and
the base range does.
**Why:** They exist so a single-byte codec gets O(1) indexing. That is a
property of the view, not a second view, and an implementation gets there
however it likes. Specifying eight view templates instead of four doubles the
clause for no reader benefit. Omitting them also removes
`detail::random_access_decode_codec` from every spec-visible signature, which
[detail-in-wording](#detail-in-wording) requires anyway. If SG16 wants the
distinction visible it comes back as a *Remarks* paragraph, not as a type.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-08 — Carried out in Step 6: both views `\omit`ted. That removed the
  last `detail::` qualifier from the document, which had been reaching the
  wording through those views' requires-clauses.
- 2026-09-08 — Reached further than it had been applied, found in Step 9: the
  `enable_borrowed_range` partial specializations *written for* those views were
  still in the synopsis, declaring borrowed-range support for views that are not
  there. Omitted too.

---

## encoding-standard-reference

**Question:** How does the wording say what each `codec` enumerator means?
**Status:** DECIDED 2026-09-03, spelling settled 2026-09-07
**Decided by:** Wording outline; spelling settled in Step 5.
**Decision:** By reference to the WHATWG Encoding Standard. `enum class codec`
has 39 enumerators; the wording says each names the WHATWG encoding of the
corresponding name rather than restating a table that is normative elsewhere and
changes there.
**Why:** Restating a living table in the working draft guarantees the two drift.
**Log:**
- 2026-09-03 — Adopted.
- 2026-09-07 — Spelling settled, in two parts because the question has two
  halves. *In the generated wording*, the standard is named in prose — "the
  WHATWG Encoding Standard" — and nothing else: specgen's `\iref` points at a
  stable name in this draft and there is none to point at, and a bibliography
  key is the paper's and would not survive into the working draft. *In the
  paper, and in the eventual working draft*, it is a normative reference in the
  [intro.refs] sense, carrying a date because a living standard has no edition
  to cite.
- 2026-09-08 — Step 10 wrote that entry, and put the question of whether a
  normative reference to a living standard is acceptable at all to SG16, where
  it belongs.

---

## null-term-header

**Question:** Does `views::null_term` get its own header?
**Status:** DECIDED 2026-09-03
**Decided by:** Wording outline; it was already the paper's position.
**Decision:** `<null_term>` stays a separate header.
**Why:** `views::null_term` has nothing to do with encodings. It is a range
adaptor over a C string that this proposal happens to need.
**Log:**
- 2026-09-03 — Recorded.
- 2026-09-06 — Consequence: it is a second specgen document, with its own root
  ([stable-name-roots](#stable-name-roots)), and the two documents
  cross-reference each other — which is why `--new-root` has to be given twice.

---

## drift-gate

**Question:** What stops the committed wording drifting from the headers, and
where does that check run?
**Status:** DECIDED 2026-09-09
**Decided by:** Steve Downey — "specgen is not available in CI, at least not at
reasonable cost."
**Decision:** Two checks. `make wording-inputs-check` runs on every pull
request and needs no specgen: it hashes the document extent — each root plus the
headers `#include`d inside its gathered region — against a committed
`papers/wording/inputs.sha256`. `make wording-check` is the real check,
regenerating every fragment and diffing, and it runs **locally**, before a paper
revision goes out.
**Why:** specgen links LLVM's Clang front end, so a job that runs
`wording-check` downloads LLVM and builds a tool this repository does not
otherwise depend on, on every push. The cheap check answers the staleness
question — the fragments were generated from these headers and not from others
— and over-reports by construction, because an edit that changes no wording
still needs a regeneration to say so. That is the trade for a check that costs
nothing.
**Log:**
- 2026-09-08 — First attempt: a workflow that installed LLVM and built specgen
  per pull request. It failed three ways. It **did not work** — specgen built and
  ran, then died on `wchar.h: fatal error: 'stddef.h' file not found`, because a
  specgen built against an unpacked LLVM tarball derives its resource directory
  from the running binary's path rather than from the LLVM it links. It **should
  not work** — CodeQL raised three high-severity
  `actions/cache-poisoning/poisonable-step` alerts, correctly: checking out a
  second repository at a ref read from a file, building it, and running it, in a
  job that can write the default-branch cache, is executing someone else's code
  with this repository's privileges. And it **cost too much** regardless.
- 2026-09-09 — Replaced by the hash check. Two bugs of its own, both invisible
  locally: `--inputs` sat below `generate.sh`'s specgen `PATH` lookup, so the
  check that exists to need no specgen needed one; and the input list was sorted
  in the developer's locale, so the file was reproducible only on a machine
  whose collating order matched the one that wrote it. Both found in CI, neither
  reproducible on a machine with a specgen on `PATH` and a punctuation-ignoring
  locale. Check it with `env PATH=/usr/bin:/bin`.
- 2026-09-09 — A released specgen binary, or a container image with one in it,
  is what would make the strong check affordable and safe at the same time.

---

## clause-heading-level

**Question:** At what heading level do the generated clauses render, relative to
the paper's own sections?
**Status:** DECIDED 2026-09-08 — accepted as-is, upstream fix wanted
**Decided by:** Step 10.
**Decision:** Flat. The clauses come out at `##`, which is the level the paper's
own sections use, so they are siblings of "Design" rather than children of the
"Wording" heading that introduces them.
**Why:** specgen's `base_heading_level` is fixed at 2 and not reachable from the
command line (specgen#97). With `toc-depth: 2` the contents read as a clause
list under the Wording heading, which is close enough to right that spending a
cross-repository dependency on it was not worth it. A nested rendering needs the
flag *and* `toc-depth: 3`.
**Why it is not just a defect entry:** the accept is the decision; the flag is
the thing that would let it be revisited.
**Log:**
- 2026-09-03 — Measured: the option exists in all three backends' `Options`
  structs and the CLI passes `paper_mode` through while leaving this at its
  default.
- 2026-09-08 — Accepted flat, and recorded as accepted rather than pending.
- 2026-09-10 — Filed upstream at last as specgen#97. It had been tracked here
  from the first measuring round without ever reaching specgen's tracker, which
  is why it was the one dependency in the index citing no issue number.
