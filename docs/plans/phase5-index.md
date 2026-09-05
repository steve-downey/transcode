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
`\expos` covers the entities declared in the spec-facing header itself.  For
the ones declared in an included `detail/` header the marker cannot reach them
today, which is specgen#36 (see N5); those findings wait for it rather than
being refactored away.  The pattern of a synopsis plus out-of-line definitions
is all a spec-facing header should have to be.

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

**Re-measured 2026-09-05** against specgen at `0c82726`.  Every defect this
project filed is now closed: #20, #21, #22, #23 and #24, on top of the eight
fixes measured on 2026-09-04.  The worklist across the eleven spec-facing
headers is 126 findings, down from 128, and `whatwg_decode_view.hpp` is at 28,
down from 64 when Phase 5 was planned.  The small drop in the total is the
point: the earlier fixes removed noise, and what is left is nearly all ordinary
markup that Steps 5-9 write.  Of the 126, 34 are members not yet described
(`begin`, `end`, `base`, `size`), 80 are declarations not yet described, nine
are `detail::` qualifiers waiting on specgen#36, and two are false positives
(N4).  `papers/wording/*.md` regenerates byte-identical, so nothing already
committed drifted.

The consequential change is not in the counts.  **A gathered header synopsis
now works.**  With #22 and #31 fixed, `<null_term>` renders as three fragments
in document order — a real `[null.term.syn]` carrying `null_sentinel_t`,
`null_sentinel` and `null_term_view`, then the per-class clauses — and
`--validate` is clean on it.  D1's fallback, a hand-authored `[transcode.syn]`,
is no longer needed.  Two things follow: this is the arrangement Step 4 should
adopt, and the standing convention above that reserves `///` has to be amended,
because closing the region needs a `/// END [x.syn]` fence.

### Closed

- **U1 — `ranges::` reported as leakage.**  Fixed 2026-09-04 (`b1054dd`); no
  `ranges` finding occurs anywhere.  Steps 6-10 need no allowlist.
- **U4 — a deduction guide corrupts a gathered `.syn` synopsis.**  Fixed
  (specgen#22, `4ae6398`): Clang's implicit deduction guides are no longer
  collected as top-level items.  The synopsis renders once, cleanly, and the
  `\omit`ted guide does not appear.
- **U5 — a docblock on an in-class hidden friend is not attached.**  Fixed
  (specgen#20, `199e291`).  The mis-diagnosis is worth remembering: the trigger
  was never hidden friends but a requires-clause holding a requires-expression,
  which is how `null_sentinel_t::operator==` is spelled.  Verified end to end —
  the sentinel clause renders `operator==` with its *Returns* — but see N3
  below for the gathered-region case.
- **U7 — no way to mask a variable's type.**  Fixed (specgen#24, `953e7d4`):
  bare `\seebelow` on a namespace-scope variable now renders
  `inline constexpr $unspecified$ thing;`, type masked and initializer dropped.
  **`[null.term.adaptor]` is unblocked**, and so are the eight closure objects.
- **U8 — `\expos` does not apply to class templates or alias templates.**
  Fixed (specgen#23, `64267e8`); the alias half landed 2026-09-04.  A marked
  class template renders `$box$` with `// exposition only` and its uses are
  rewritten, so `[transcode.reqs]`'s const-compatibility chain is unblocked
  whichever way it is spelled.
- **U9 — identifiers inside a string literal are scanned for leakage.**  Fixed
  2026-09-04 (`b1054dd`).
- **N1 — a constructor's member-initializer list renders into the synopsis.**
  Fixed (specgen#21, `619311f`): the body splice starts at a written
  ctor-initializer list.  The `\expos` rename now also reaches the description
  text.
- **N2 — U4's corruption was invisible to `--validate`.**  Closed with U4.

### Open

- **U2 — `--base-heading-level` on the command line.**  Still absent; the mpark
  backend hard codes level 2.  Not blocking: Step 10 can accept flat headings or
  post-process, and the plan says which it did.
- **U3 — namespace mapping is automatic.**  Nothing to do; recorded so no step
  goes looking for a mapping option that does not exist.
- **U6 — every generated clause heading warns at paper-build time.**  A
  paper-build warning from mpark, not a specgen finding; not re-measured.
  **Gates Step 10's acceptance criterion** of a warning-free build.
- **N3 — a routed member description is dropped inside a gathered synopsis
  region.**  Filed as specgen#34.  A member *defined in class* inside the
  region, carrying a docblock and a `\ref{stable.name}` route to a section
  declared after the `END` fence, renders in the synopsis but its description is
  discarded and the target clause comes out empty.  Silent under `--validate`.
  Members defined *outside* the region are unaffected, which is why
  `[null.term.view]` renders all three of its members and `[null.term.sentinel]`
  renders none: the sentinel's `operator==` is the one member defined in class.
  **Gates `[null.term.sentinel]` in Step 4** — but only that clause, and the
  header synopsis it sits beside is now sound.
- **N5 — a `detail::` name in a spec-visible signature cannot be rendered
  exposition-only from an included header.**  Filed as specgen#36.  `\expos`
  does the right thing when the entity is declared in the spec-facing header,
  and is silently ignored when the identical declaration sits in an included
  one — so the only remedies specgen offers are to rewrite the name or move the
  entity out of `detail`, both of which change the library.  Six concepts
  account for all nine qualifier findings:
  `const_iterator_compatible_range` and `const_sentinel_compatible_range` from
  `detail/range_traits.hpp`, and `whatwg_encode_input`, `whatwg_encode_codec`,
  `random_access_encode_codec`, `random_access_decode_codec` from the two
  select headers.  See D7; do not refactor around this.
- **N4 — the private-member check is keyed by bare name.**  Filed as
  specgen#35.  A private member in one class makes an unrelated *public* member
  of the same name in another class report as "an unmarked private member".
  Two of the 126 findings are this: `decode_closure` and `encode_closure` are
  `struct`s whose public `Codec codec_;` collides with the private
  `Codec codec_;` in the corresponding view's iterator.  They are false
  positives; do not mark or mask anything in response to them.


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
