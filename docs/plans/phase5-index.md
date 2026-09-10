# Phase 5: Specification Wording from the Headers

## Context

`papers/transcode-view.md` (D4246R0) has an abstract, motivation, comparison
tables, design rationale and a P2728 comparison.  It has no **Wording**.
Writing that wording by hand would create a second source of truth for the API
that drifts from `include/` on the first refactor.

`beman.specgen` (`~/src/specgen/main`, installed as `~/.local/bin/specgen`,
0.1.0) exists to close that gap: it lowers a marked-up C++26 header to a
semantic IR and renders draft LaTeX, mpark/wg21 markdown, or org.  Phase 5
adopts it: the wording clauses of the paper are **generated from the shipping
headers** and transcluded into the paper by the mpark/wg21 build, and any drift
between them is a build failure.

Read `~/src/specgen/main/docs/user-guide.md` (markup vocabulary, commands,
diagnostics) and `~/src/specgen/main/docs/architecture.md` (§1 input contract,
§3.4 synopsis extraction, §3.5 namespace mapping, §9 validation) before
executing any step.

## Measurements taken before planning (2026-09-03)

These are facts from running the installed `specgen 0.1.0` against this
worktree, not estimates.  They are what the steps below are sized against.

- Every public header parses under specgen's Clang 22 front end in about a
  second with this tail — no compilation database involved:

  ```sh
  export LD_LIBRARY_PATH=/home/sdowney/install/gcc-16/lib64:$LD_LIBRARY_PATH
  specgen generate --emit-ir include/beman/transcode/whatwg_decode_view.hpp \
      --no-compile-commands -- \
      -std=c++2c --gcc-toolchain=/home/sdowney/install/gcc-16 -I include
  ```

  `whatwg_decode_view.hpp` 1.12 s, `whatwg_encode_view.hpp` 1.05 s,
  `decode_view.hpp` 0.86 s, `encode_view.hpp` 0.92 s, `detail/null_term.hpp`
  instant.  Parse cost is not a constraint.
- The umbrella `transcode.hpp` is the one header that fails to parse:
  `beman/transcode/config_generated.hpp` is generated into the build tree, so
  the tail needs `-I .build/build-system/include` as well.
- `render --validate` on today's `whatwg_decode_view.hpp` reports 64 findings.
  They are of exactly three kinds, all expected for an unmarked header:
  coverage (`X is declared in the synopsis but is not described`), and two
  leakage classes — `detail` (a qualifier resolving to
  `beman::transcoding::detail`) and `ranges` (a qualifier resolving to
  `std::ranges`).  There is no fourth surprise.  That number is the worklist
  size for one header, and it shrinks monotonically as markup lands.
- `include/` contains **no** `//!`, `/*!`, `///` or `/** */` comments today, so
  the specgen markup namespace is unoccupied and no comment has to be
  reclassified.  `.clang-format` has `ReflowComments: true` at
  `ColumnLimit: 119`; docblock lines stay under that so reflow never rewrites
  markup.
- The mpark backend emits `::: wording` divs with headings
  `## Title [stable.name]{- .sref} {-}`, starting at heading level 2
  (`base_heading_level = 2`, not exposed on the command line).
- specgen's own corpus has no header that uses a `std::` sub-namespace, so the
  `ranges::` leakage finding above is an **upstream gap**, not a transcode
  defect.  See "External dependencies".

## Decisions

**D1 — One specgen document per proposed standard header.**  The paper proposes
`<transcode>` and `<null_term>`.  Each maps to exactly one spec-facing header in
this repo, and that header is what specgen is run over.

**How a document reaches more than one file, settled 2026-09-07.**  specgen used
to process only declarations located in the main file, which would have made
this decision "put the whole of `<transcode>` in one 3,500-line file".  It does
not: a gathered `.syn` region now gathers the declarations of the headers
`#include`d inside it (specgen#77, PR #78), so `transcode.hpp` stays the
umbrella it already was and the sixteen component headers stay where they are.
The region is the statement of which includes are the specification surface;
`detail/` includes sit outside it.  This is D7 again -- the tool renders what
the library writes -- and it is why Step 3's task 4 cost a rewrite of one
umbrella rather than a rewrite of the library.  Confirmed working
on 2026-09-05: `<null_term>` renders a real gathered `[null.term.syn]` beside
its per-class clauses, validating clean.  The fallback this decision carried --
one document per header family with a hand-authored `[transcode.syn]` -- is
retired; it existed because a deduction guide corrupted the gathered region
(specgen#22), and that is fixed.

**D2 — Generated fragments are committed and checked.**  `make wording`
regenerates `papers/wording/*.md`; `make wording-check` regenerates into a
temporary directory and diffs.  CI runs the check.  Committing the fragments
keeps the paper buildable on a machine with no specgen and no Clang 22, which
is the normal case for a co-author.

**D3 — Transclusion is native pandoc multi-file input.**  `base.mk` builds its
pandoc command from `$(filter %.md, $^)`, so a target with several markdown
prerequisites is concatenated in prerequisite order.  The paper targets get the
fragment list as ordered prerequisites; the order comes from the manifest
`specgen render --split` prints.  No new markdown preprocessor, no include
syntax to invent.

**D4 — Fragments live in `papers/wording/`.**  `flat.mk` turns every
`papers/*.md` into its own paper; a subdirectory is invisible to that wildcard.

**D5 — Stable names root at `transcode.`**, with `null.term` for the second
header.  `--split` derives the root fragment name from the common prefix, so
the header synopsis fragment is `transcode.syn.md` — spelled explicitly with
`--root` rather than left to derivation.

**D6 — No `--paper` mode.**  `--paper` wraps everything in `::: add` and
underlines it.  This paper adds whole new clauses; an authored editing
instruction ("Add a new clause [transcode] as follows:") reads better than
several hundred underlined lines.  The switch is one flag if that judgement
changes.

**D7 — `detail::` never appears in a spec-visible signature, and the headers
do not move to achieve that.**  A `detail::` name reaching the wording is a
real finding: the published text must not name something the reader cannot see.
What was wrong in the original decision is the remedy it accepted.  It said such
a name "either moves out of `detail` ... or is marked `\expos`", and relocating
a helper out of `detail` changes name lookup, ADL and the library's effective
public surface — a real change to the library, taken on for a documentation
tool.  It was recorded as the single largest source of the refactoring in
Step 3, which is the clearest sign it was pointed the wrong way.

The rule now: a `detail::` name in a spec-visible signature renders
exposition-only or `unspecified`, and the header keeps the spelling it wants.
`\expos` covers it, and since specgen#36 it reaches entities declared in an
included `detail/` header too.  Measured on 2026-09-05: marking the six concepts
`\expos` in the headers that declare them takes the qualifier findings from
nine to three, with nothing moved and nothing renamed.  The three survivors are
class-head constraints (N6) and wait for specgen#48.  The pattern of a synopsis
plus out-of-line definitions is all a spec-facing header should have to be.

## Step index

| Step | Branch | Deliverable | Depends on |
|------|--------|-------------|-----------|
| 1 | `p5-step1-specgen-harness` | `papers/wording/`, pinned generate script, `make wording` / `wording-check`, paper transclusion rule, proved end to end on one header | — |
| 2 | `p5-step2-wording-outline` | [`docs/wording-outline.md`](../wording-outline.md): clause tree, stable names, header→clause map, fragment order | — |
| 3 | `p5-step3-spec-header-shape` | Headers refactored so each spec-facing header's main-file decls are exactly the spec surface | 2 |
| 3b | `p5-step3b-codec-pushdown` | Codec implementation out of the WHATWG view headers; partly done, remainder ordered behind the `_or_error` unification | 3 |
| 4 | `p5-step4-errors-concepts-null-term` | Markup for the error enums, the range concepts, and `null_term` | 1, 3 |
| 5 | `p5-step5-codec-labels-sniff` | Markup for `codec`, `get_encoding`, `sniff_encoding` | 4 |
| 6 | `p5-step6-whatwg-views` | Markup for the four WHATWG decode views and four encode views | 5 |
| 7 | `p5-step7-pluggable-codecs` | Markup for the codec concepts and the pluggable decode/encode views | 6 |
| 8 | `p5-step8-transcode-pipeline` | Markup for `transcode`, `pluggable_transcode`, `transcode_string` | 7 |
| 9 | `p5-step9-iconv` | Markup for the iconv views and bulk helpers, or a recorded decision to omit them from the wording | 8 |
| 10 | `p5-step10-paper-assembly` | Wording section of the paper: editing instructions, fragment order, clean `--validate`, CI drift gate | 4-9 |

Steps 4-9 are the same loop nine times: mark up a clause, regenerate, drive
`--validate` findings for that clause to zero, commit headers and fragments
together.  They are separated by clause so a step is reviewable and so a
mid-phase stop still leaves the paper buildable.

**Step 3's task 4 is landed** (2026-09-07): `transcode.hpp` is the document for
`<transcode>`, the sixteen component headers stay where they are, and
`[transcode.syn]` is a generated 1,160-line header synopsis holding all of them.
Step 3b's remainder -- the stateful codec pushdown -- landed with it.  See
`docs/plans/p5-step3-spec-header-shape.md`, "Outcome".

**Step 4 is landed** (2026-09-06), all three clauses, at the price of two
upstream fixes: `[transcode.errors]` needed specgen to be able to specify an
enumeration at all, and `[null.term.adaptor]` needed a folded-in declaration's
description to reach its clause.  Both are merged (specgen #70 and #71); see
`docs/plans/p5-step4-errors-concepts-null-term.md`, "Outcome".

## Standing conventions

- Branch and worktree per step, rooted from `main`, per `CLAUDE.md`.
- `make lint` and `make test` green before every commit; `make lint` covers the
  markdown as well as the C++.
- `make wording-check` green before every commit from Step 1 onward.
- `papers/wording/*.md` is generated output.  Never hand-edit it; edit the
  header markup and regenerate.
- The specgen invocation lives in exactly one place, `papers/wording/generate.sh`.
  No step adds a second spelling of the parse tail.
- Markup is `//!` and `/*! */` only, with one exception: a gathered header
  synopsis is closed by a `/// END [x.syn]` fence, which specgen requires and
  which nothing else in the repo spells.  `/** */` stays unused, and `///` is
  reserved for that fence alone, so no comment is ambiguous between Doxygen and
  specgen.
- Docblock lines stay under 119 columns so `ReflowComments` never touches them.

## External dependencies (work in `~/src/specgen/main`)

These are upstream changes to specgen.  They are tracked here because they gate
transcode steps, but they are executed in the specgen repository.

**Re-measured 2026-09-07** against specgen at `1b4e10f`.  **Every defect this
project has filed is closed**, including the two Step 4 filed -- #68 (an
enumeration produced no wording) and #69 (a folded-in declaration's description
was dropped), fixed by specgen PRs #70 and #71, both merged.  The same round
brought one change of specgen's own, #72, which sizes each sentinel to the text
that replaces it; it moves no fragment of this paper.

**163 findings across the seventeen spec-facing headers**, and the *same* 163
the previous binary reports on the same headers: the update unblocked two
clauses and moved nothing else.  The 114 this section used to quote counted a
smaller set of headers, so the two numbers are not comparable -- what the count
is for is the size of Steps 5-9's worklist, which makes the denominator worth
writing down.  All 163 are coverage or leakage findings against clauses nobody
has written yet: 117 "declared in the synopsis but is not described", 42 "used
in wording but is not a documented entity", 4 unmarked private members.  **No
qualifier finding anywhere**: `detail::` still reaches no spec-facing header's
wording.  The three headers Step 4 marked up -- `error.hpp`, `concepts.hpp`,
`null_term.hpp` -- validate clean, at zero.

**Note for anyone regenerating**: the committed fragments require a specgen at
or after `c881026`, which is where a document reaches the headers it includes
(specgen#77) -- what makes `transcode.hpp` a document at all.  An installed
specgen from that commit or later regenerates every fragment byte for byte,
with no `SPECGEN=` override; an older one renders `[transcode.syn]` as an empty
heading.  0.1.0 at `b90faa7` cannot generate
`transcode.errors.md` or `null.term.adaptor.md` at all and reports them stale.
A specgen installed from current `main` regenerates every committed fragment
byte for byte, so no `SPECGEN=` override is needed.

The recheck also found three things and filed all three, none of which moves a
fragment of this paper: specgen PR #75 (a route this project's own clause
depends on could name a section that does not exist and lose its paragraph
silently -- the check a class member already had), PR #76 (`\expos` and
`\seebelow` on an enumeration, the two markers #70 left behind), and issue #74
(the draft's enumerator table is two columns flat and only the
two-dimensional table exists).  The first two are defects in what this project
asked for last round, which is what a recheck is for; the third is why
`[transcode.errors]` states its enumerator meanings as an `\item` list.

- **N6 closed** (#48, `fa9af1c`).  The exposition-only rename now reaches a
  class template's own requires-clause, which was the last three qualifier
  findings.  D7 is satisfied in full, by six `\expos` comments and nothing
  else -- no declaration moved, no name changed.
- **N8 closed** (#55, `5b04bf2`, apply the declaration masks inside a gathered
  region).  `views::null_term` drops its `\omit` and renders
  `inline constexpr $unspecified$ null_term;` in the header synopsis, which is
  what the draft writes.  `[null.term.adaptor]` is unblocked; only its prose is
  outstanding, and that is authorship, not tooling.

The same round also tightened the template head's namespace drop (`fa9af1c`),
so `template<std::contiguous_iterator I>` renders as
`template<contiguous_iterator I>`.  That is the draft's spelling, and it moved
the committed fragments -- regenerated and committed here, which is D2 working
as intended.

### The gathered-region pattern

**Five** defects turned out to be one shape: a marker or a check that works at
namespace scope and is skipped for a declaration folded into a gathered region.
N3 (#34, a routed member's description), N7 (#45, coverage checking), N8 (#55,
declaration masks), the class-head half of N6 (#48), and #69 (a namespace
entity's whole description).  All five are fixed.  Each of the first four was
recorded here as the last of them, which is the reason to keep the shape
written down rather than the count: Steps 5-9 gather `<transcode>`, a far
larger surface than `<null_term>`, and the failure mode is silent.  A dropped
entity has an empty roster, so coverage has nothing to report about it, and a
clean `--validate` does not prove an authored paragraph arrived.  Read the
fragment.

### Closed

- **U1 / U9 — leakage checker discriminator.**  #3-era fix, `b1054dd`.
- **U4 / N2 — a deduction guide corrupted a gathered `.syn` synopsis.**  #22.
- **U5 — a docblock on an in-class hidden friend is not attached.**  #20.  Never
  about hidden friends: the trigger was a requires-clause holding a
  requires-expression.
- **U7 — no way to mask a variable's type.**  #24, completed by #55.
- **U8 — `\expos` on class and alias templates.**  #23.
- **N1 — a constructor's member-initializer list rendered into the synopsis.**
  #21.
- **N3 — a routed member description dropped inside a gathered region.**  #34.
- **N4 — the private-member check keyed by bare name.**  #35.
- **N5 — `detail::` could not render exposition-only from an included header.**
  #36.  The one that made D7 cheap.
- **N6 — the exposition-only rename skipped a class template's own
  requires-clause.**  #48.
- **N7 — a class folded into a gathered synopsis was not coverage-checked.**
  #45.
- **N8 — bare `\seebelow` on a variable was not applied inside a gathered
  region.**  #55.
- **specgen#68 — a documented enumeration produced no wording.**  #70,
  `f64f043`.  A docblock on an `enum class` was rejected as an unsupported
  entity kind, and there was no other spelling: a gathered region rendered the
  declaration and still refused the docblock, a detached docblock was dropped,
  and a `\verbatim-itemdecl` before the enum attached to it and hit the same
  error.  An enumeration is now a documented namespace entity like the others,
  its declaration is the itemdecl and its description says what the enumerators
  mean.  `[transcode.errors]` could not be written at all without it.
- **specgen#69 — a folded-in declaration's description was dropped, and `\at`
  did not route it.**  #71, `b746da6`.  A described namespace entity's wording
  now travels to the section `\at` names, else to the one its `\ref` group
  header names, else out beside the synopsis.  `[null.term.adaptor]` is the
  clause that needed it, and a range adaptor object is the case with no way
  around it: a variable with an initializer has no out-of-line definition to
  carry a description into a later clause, and moving its declaration out of
  the region would take it out of the header synopsis.

### Open

- **U2 — `--base-heading-level` on the command line.**  Still absent, and
  **filed at last as
  [specgen#97](https://github.com/steve-downey/specgen/issues/97)** on
  2026-09-10.  This entry tracked it from the first measuring round without
  ever reaching specgen's tracker, which is why it is the one U-item here that
  cited no issue number.  The option exists in all three backends'
  `Options` -- `mpark`'s and `org`'s `base_heading_level`, LaTeX's
  `base_section_depth` -- and the CLI passes `paper_mode` and `new_roots`
  through to the backend while leaving these at their defaults.  **Step 10 accepted flat headings**: the
  clauses are `##`, which is the level the paper's own sections use, so they
  are siblings of "Design" rather than children of "Wording".  The table of
  contents reads as a clause list under the Wording heading, which is close
  enough to right that spending an upstream flag on it was not worth another
  cross-repository dependency.  `toc-depth: 2` is what makes it read that way;
  a nested rendering would need the flag *and* `toc-depth: 3`.
- ~~**specgen#93 — a documented enumerator is reported as a foreign name when a
  generated table shares its spelling.**~~  **Closed.**  Filed 2026-09-09 against
  the bare-name leakage check in specgen `8caa569`, which text-matched names
  declared outside the run and so could not tell `codec::windows_1252` from
  `detail::tables::windows_1252`.  `--validate` is back to **zero findings**
  over this document, and nothing was marked `\expos` to get there.
- ~~**specgen#94 — `--new-root` takes one name.**~~  **Closed and adopted.**
  The flag repeats now, so `generate.sh` renders with `--new-root transcode
  --new-root null.term` and the `sed` that stripped the `.sref` class is gone.
  The fragments came out byte-identical to what the `sed` produced, which is
  the check that the two mechanisms agreed before one replaced the other.
- **U3 — namespace mapping is automatic.**  Nothing to do; recorded so no step
  goes looking for a mapping option that does not exist.
- ~~**specgen#74 — an enumerator table is two columns flat.**~~  **Closed and
  adopted.**  `\libtab2` landed upstream (specgen `93c248c`), and all three
  enumerations in `[transcode.errors]` now print a Constant/Meaning table --
  [fs.enum.file.type]'s shape, which is what an enumeration's meanings look
  like in the draft.  The `\item` list they used while the two-dimensional
  table was the only one is gone.
- **U6 — every generated clause heading warns at paper-build time.**  An mpark
  warning, not a specgen finding: `stable name <x> not found`, once per clause.
  It is what a stable name that is not in the draft yet looks like, so it is
  one per clause the phase adds -- seventeen by Step 10.

  **Worked around downstream in Step 10, and still open upstream.**  The
  warning was the visible half; the other half is that mpark emits a link
  anyway, so every clause heading and every cross-reference in the published
  paper pointed at a `c++draft` page that does not exist.  `generate.sh` now
  drops the `.sref` class from names under this paper's own two stable-name
  roots, after rendering and before the fragments are committed, which leaves
  `[transcode.iconv]` -- what the draft itself prints.  It is keyed on the
  roots on purpose: a citation of a clause that *is* in the draft keeps its
  `.sref` and still resolves.

  Filed as [specgen#89](https://github.com/steve-downey/specgen/issues/89) and
  fixed upstream as `render --new-root`, which needed a second round
  ([#94](https://github.com/steve-downey/specgen/issues/94)) before it could
  take both of this paper's roots.  **The `sed` is gone**; `generate.sh` passes
  the roots instead.  The build is warning-free either way.


## Risks

- **The refactor in Step 3 is the real cost of the phase.**  Everything else is
  comment authoring.  If Step 3 stalls, Steps 4-10 still work per header family
  under D1's fallback, at the price of a hand-written synopsis.
- **Clang 22 and GCC 16 are required to regenerate**, but not to build the
  paper (D2).  A contributor without them can still edit prose and build a PDF;
  they cannot change the wording.
- **specgen is under development.**  Its IR and output can change under us.
  The committed fragments plus `make wording-check` make that visible as a diff
  rather than as a silent divergence.
