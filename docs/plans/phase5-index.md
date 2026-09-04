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
a real header synopsis rather than a per-class accident.  The fallback, if
co-location proves too disruptive, is one document per header family with a
hand-authored `[transcode.syn]`; it is a fallback because it reintroduces a
hand-maintained synopsis.

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

**D7 — `detail::` never appears in a spec-visible signature.**  specgen's
leakage checker errors on any surviving implementation-namespace qualifier,
including in a synopsis, and it is right to.  Every concept, alias and helper
named in a spec-facing declaration either moves out of `detail` (because it is
really part of the specification) or is marked `\expos` (because it is
exposition-only).  This is the single largest source of the refactoring in
Step 3.

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
- Markup is `//!` and `/*! */` only.  `///` and `/** */` stay unused in this
  repo so that no comment is ambiguous between Doxygen and specgen.
- Docblock lines stay under 119 columns so `ReflowComments` never touches them.

## External dependencies (work in `~/src/specgen/main`)

These are upstream changes to specgen.  They are tracked here because they gate
transcode steps, but they are executed in the specgen repository.

**Re-measured 2026-09-04** against specgen at `af60209`, which landed eight
fixes.  Every item below carries its verified state; the probes are in the
review that produced them.  Headline: `whatwg_decode_view.hpp` went from 64
findings to 29, and no finding anywhere is `ranges` noise any more.  Across all
eleven spec-facing headers the worklist is 128 findings, and every one of them
is real work rather than something to allowlist.

- **U1 — `ranges::` reported as leakage.  FIXED** (specgen `b1054dd`).  The
  leakage checker skipped a foreign qualifier when its namespace was declared
  outside the main file, which was the wrong discriminator; it now skips a
  chain link whose qualified path is rooted in `std::` and reports every other
  surviving qualifier wherever it was declared.  Verified: zero `ranges`
  findings across all eleven headers, and the `ranges::view_interface`
  base-clauses that produced them render unchanged.  **Steps 6-10 no longer
  need the known-noise allowlist**; drive `--validate` to zero.

  The same fix cuts the other way and is why the remaining counts are worth
  trusting: `detail::` reached through an included header used to pass
  silently, and Step 3 moved exactly that machinery into
  `detail/range_traits.hpp` and `detail/whatwg_{decode,encode}_select.hpp`.
  Those leaks are now reported.  They are D7 work, not noise.
- **U2 — `--base-heading-level` on the command line.  NOT fixed.**  The mpark
  backend hard codes level 2, so generated clause headings are `##` and land as
  siblings of the paper's own `##` sections.  A flag would let the wording nest
  under a `## Wording` heading.  Not blocking: Step 10 can accept flat headings
  or post-process, and the plan says which it did.
- **U3 — namespace mapping is automatic.**  `build_namespace_drop_set` derives
  the drop set from the header's own top-level namespaces, so
  `beman::transcoding` maps to `std` with no configuration.  Nothing to do;
  recorded so no step goes looking for a mapping option that does not exist.
- **U4 — a deduction guide corrupts a gathered `.syn` synopsis.  NOT fixed,
  and now silent.**  With a `\rSec2[x.syn]` region closed by a
  `/// END [x.syn]` fence, the rendered synopsis still repeats the class body
  with `<deduction guide for holder>` substituted for the class name, and
  `\omit` on the guide does not suppress it.  What changed is that
  `--validate` now reports **nothing** for the corrupted output, so the drift
  gate will not catch it.  Still **gates the `<null_term>` header synopsis**,
  and the corruption is now something a human has to see.

  Two further notes for whoever picks this up.  Closing the region needs a
  `/// END [x.syn]` fence, and this repo's standing conventions above reserve
  `///` as unused — adopting a gathered synopsis means amending that rule.  And
  outside a gathered region the guide is fine: `\omit` works and the output is
  clean, which is still what Step 1 relies on.
- **U5 — a docblock on an in-class hidden friend is not attached.  FIXED for
  the general case; a narrower bug remains.**  The docblock now attaches and
  renders as an itemdecl with its elements, provided the member is routed —
  a `\rSec` section plus a `\ref{stable.name}` group in the class body, or an
  `\at`.  Unrouted, the diagnostic is no longer "is not described" but the far
  more useful "the description of `operator==` is routed to no section".

  What still fails is narrow and was mis-diagnosed as being about hidden
  friends: the docblock is dropped only when the declaration carries a
  **requires-clause holding a requires-expression**.  Probes isolate it —
  a constrained template parameter (`template <std::input_iterator I>`) is
  clean, a named constraint (`requires std::equality_comparable<I>`) is clean,
  and `requires requires(I i) { { *i == 0 }; }` is not.  `null_sentinel_t`'s
  `operator==` is spelled the one way that fails.

  **`null.term.sentinel` is unblocked**, at the price of naming the concept —
  which is better wording material anyway, and which fix `1a6728f` now renders
  as a documented item in its own right.  Step 4 owns the choice of where that
  concept lives, because `detail::` in the signature would violate D7.
- **U6 — every generated clause heading warns at paper-build time.**  mpark
  prints `stable name null.term.view not found` for each `{- .sref}` span whose
  name is not in its stable-names database, which by definition is every clause
  a paper proposes.  specgen already emits the unnumbered form to avoid this and
  it warns anyway.  **Gates Step 10's acceptance criterion** of a warning-free
  build; the fix may belong in mpark/wg21 rather than specgen.  Not re-measured
  on 2026-09-04: it is a paper-build warning, not a specgen finding.
- **U7 — no way to mask a variable's type.  NOT fixed.**  A customization point
  object is spelled `inline constexpr unspecified null_term;` in the draft.
  `\seebelow` on a namespace-scope variable is still accepted with no effect:
  a probe renders `inline constexpr detail::adaptor thing{};` verbatim,
  initializer and all, and the `detail` qualifier is then reported as leakage.
  So `views::null_term` can still only be `\omit`ted, which is what Step 1
  does.  Note that `1a6728f` did make documented namespace-scope *variables*
  render as wording items; it is the type masking that is missing, not the
  rendering.  Wanted for Step 4.
- **U8 — `\expos` does not apply to class templates or alias templates.  HALF
  fixed** (specgen `26b7b7d`).  Alias templates now work: a marked one renders
  as `using $maybe-const$ = ...; // exposition only` with its uses rewritten to
  the exposid.  **Class templates still ignore the marker** — the name renders
  verbatim with no exposition-only comment.  The draft's exposition-only
  helpers include both kinds, so `[transcode.reqs]`'s const-compatibility chain
  is unblocked exactly to the extent it is spelled as aliases; see
  `docs/wording-outline.md`, "The `detail::` audit".
- **U9 — identifiers inside a string literal are scanned for leakage.  FIXED**
  (specgen `b1054dd`, same change as U1).  The WHATWG closures'
  `static_assert` diagnostic still says "use
  `beman::transcoding::views::null_term` …" and no longer draws a `beman`
  qualifier finding.

Two defects found by this review and not yet filed upstream:

- **N1 — a constructor's member-initializer list renders into the class
  synopsis.**  Fix `f45e53e` splices in-class *bodies* out of the synopsis
  unconditionally, but not the ctor-init-list that precedes one, so
  `random_access_whatwg_decode_view` renders as
  `constexpr explicit random_access_whatwg_decode_view(R base) : base_(move(base));`
  where the draft would write the declaration alone.  Fix `eedc9ad`'s new
  private-member check then correctly reports the `base_` it exposes, which is
  how this surfaced: four findings across the view headers, naming `base_` and
  `codec_`.  The finding is right and the rendering is what is wrong.
- **N2 — U4's corruption is invisible to `--validate`.**  Recorded under U4
  above; separate from the corruption itself, because a silent drift gate is
  the part that would let a bad synopsis reach the paper.


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
