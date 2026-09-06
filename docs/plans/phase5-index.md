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
this repo, and that header is what specgen is run over.  specgen only processes
declarations located in the main file, so "what is in the file" *is* "what is in
the synopsis"; one document per proposed header is what makes the root fragment
a real header synopsis rather than a per-class accident.  Confirmed working
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

**Re-measured 2026-09-05 (evening)** against specgen at `4579887`.  Every defect
this project has filed is closed: #20-#24, then #34, #35 and #36.  124 findings
across the eleven spec-facing headers, from 126.

Two results matter more than the count.

**`<null_term>` is fully generable.**  With #34 fixed, the gathered arrangement
renders all three clauses -- `[null.term.syn]` as a real header synopsis,
`[null.term.sentinel]` with `operator==` and its *Returns*, `[null.term.view]`
with all three members -- and validates clean.  Step 4 has nothing left to wait
for.

**D7 costs six comments, not a refactor.**  #36 landing means `\expos` on a
concept declared in an included `detail/` header is honoured at its use sites in
the spec-facing header.  Measured: marking the six concepts takes the qualifier
findings from nine to three, with no declaration moved and nothing renamed --
`//! \expos` lines only.  The three survivors are all class-head constraints,
which is N6 below.  That markup belongs to the steps that write those clauses,
so it is not applied here.

### A caveat on "clean"

`--validate` is **not** a completeness signal inside a gathered region.  Delete
the `//! \returns` from `null_term_view::end()` in the gathered arrangement and
the rendered wording silently loses that *Returns* -- the declaration still
renders, the element is gone, and validation reports nothing.  That is upstream
issue #45, a class folded into a gathered synopsis is not coverage-checked.

It bears directly on D2 and on Step 10's acceptance criterion: a clean
`--validate` over a gathered header does not currently mean the clause is
complete, and `make wording-check` only catches drift between the headers and
the committed fragments, not a description that was never rendered.  Until #45
lands, treat a gathered clause's completeness as something to read, not
something the gate proves.

### Closed

- **U1 / U9 — leakage checker discriminator.**  Fixed 2026-09-04 (`b1054dd`).
  No `ranges` finding occurs anywhere; Steps 6-10 need no allowlist.
- **U4 / N2 — a deduction guide corrupted a gathered `.syn` synopsis, silently.**
  Fixed (#22, `4ae6398`).
- **U5 — a docblock on an in-class hidden friend is not attached.**  Fixed
  (#20, `199e291`).  It was never about hidden friends: the trigger was a
  requires-clause holding a requires-expression, which is how
  `null_sentinel_t::operator==` is spelled.
- **U7 — no way to mask a variable's type.**  Fixed (#24, `953e7d4`); a CPO
  renders `inline constexpr $unspecified$ name;`.  `[null.term.adaptor]` and the
  eight closure objects are writable.
- **U8 — `\expos` on class templates and alias templates.**  Fixed (#23,
  `64267e8`, with the alias half on 2026-09-04).
- **N1 — a constructor's member-initializer list rendered into the synopsis.**
  Fixed (#21, `619311f`).
- **N3 — a routed member description was dropped inside a gathered region.**
  Fixed (#34, `68bbd69`).  This is what was emptying `[null.term.sentinel]`.
- **N4 — the private-member check was keyed by bare name.**  Fixed (#35,
  `2e65aa4`).  The two `codec_` false positives on `decode_closure` and
  `encode_closure` are gone.
- **N5 — a `detail::` name could not be rendered exposition-only from an
  included header.**  Fixed (#36, `08178e5`).  See D7.

### Open

- **U2 — `--base-heading-level` on the command line.**  Still absent.  Not
  blocking: Step 10 can accept flat headings or post-process, and the plan says
  which it did.
- **U3 — namespace mapping is automatic.**  Nothing to do; recorded so no step
  goes looking for a mapping option that does not exist.
- **U6 — every generated clause heading warns at paper-build time.**  An mpark
  warning, not a specgen finding.  **Gates Step 10's** warning-free build.
- **N6 — an exposition-only rename is not applied in a class template's own
  requires-clause.**  Filed as specgen#48.  The rename reaches every member
  requires-clause and stops at the class head, so three `detail::` spellings
  survive into the synopses of `random_access_whatwg_decode_view`,
  `random_access_whatwg_encode_view` and `whatwg_encode_view`.  These are the
  last three qualifier findings; do not move the concepts to clear them.
- **N7 — a class folded into a gathered synopsis is not coverage-checked.**
  Upstream #45, already open.  See "A caveat on 'clean'" above.  **Weakens
  Step 10's acceptance criterion** until it lands.


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
