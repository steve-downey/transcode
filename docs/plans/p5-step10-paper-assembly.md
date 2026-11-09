# P5-Step 10: Paper Assembly and the Drift Gate

**Branch:** `p5-step10-paper-assembly`
**Depends on:** Steps 4-9
**Read first:** `docs/plans/phase5-index.md`, `papers/wg21/MANUAL.md`

---

## Goal

Turn a directory of validated fragments into the paper's Wording section, and
make drift between the headers and the paper a CI failure.

## Tasks

1. **Write the Wording section of `papers/transcode-view.md`.**  The generated
   fragments carry the clauses; the paper carries the editing instructions
   around them — "Add a new header `<transcode>` to [headers], Table 1", "Add a
   new clause [transcode] as follows", the feature-test macro row for
   `__cpp_lib_transcode_view` in [version.syn].  Those are the parts specgen
   cannot know.  Per [editing-instruction-form](../decisions.md#editing-instruction-form) this is authored prose, not `--paper` mode.
2. **Fix the section's position.**  Wording goes after "Impact on the Standard"
   and before the bibliography.  Because transclusion is prerequisite order
   ([transclusion-mechanism](../decisions.md#transclusion-mechanism)), everything that must follow the wording — Acknowledgements, Prior
   Art, Relationship to Other Proposals, References — moves out of
   `transcode-view.md` into a second authored file, listed after `$(WORDING_MD)`
   in the paper's prerequisites.  `flat.mk` globs `papers/*.md` and would build
   that file as a paper of its own, so it lives in a subdirectory:
   `papers/parts/tail.md`.  The YAML metadata block stays at the top of
   `transcode-view.md`, which remains the first input.
3. **Resolve heading levels.**  Generated clause headings are `##`
   ([clause-heading-level](../decisions.md#clause-heading-level)).  Either accept them as siblings of the paper's own sections, or
   take the upstream `--base-heading-level` flag if it has landed by now.  State
   which in the commit message.
4. **Resolve the bibliography placement** decided in Step 1 task 5: with an
   explicit `::: {#refs}` div if citeproc would otherwise append it after the
   wording.
5. **The drift gate.**  Add `make wording-check` to the CI workflow that already
   runs `make lint` (`.github/workflows/`).  A pull request that edits a header
   without regenerating fails; a pull request that edits a fragment by hand
   fails.  The job needs specgen, Clang 22 and GCC 16 — if that is too much for
   the existing runners, run it in the same job that already builds with GCC 16
   and skip it with an explicit, loud message elsewhere, rather than silently
   passing.
6. **Full-document validation.**  `specgen render --validate` over every
   spec-facing header must be clean, with no allowlist: specgen#3 landed upstream on
   2026-09-04 and the `ranges` noise it excused is gone.
7. **Reconcile the design sections against the generated clauses.**  The
   authored prose was written before the wording existed and has drifted from
   it in at least one place: the "Concepts" section shows both
   `legacy_byte_range` and `unicode_scalar_range` constrained on
   `ranges::input_range`, and `legacy_byte_range` is defined on
   `ranges::range` (found while writing `[transcode.reqs]` in Step 4).  Read
   every hand-written code block in the paper against its generated clause; the
   generated one is the truth.
8. **Bump the paper's revision** and update the abstract to say the wording is
   generated from the reference implementation — that is a claim reviewers
   should be able to check, and it is the phase's headline.

## Acceptance

- `make -C papers transcode-view.pdf` produces a paper whose Wording section
  contains every clause in `docs/wording-outline.md`, in outline order.
- `make wording-check` runs in CI and fails on a hand-edited fragment.
- No pandoc warnings about unresolved `.sref` spans.
- `make lint` and `make test` green.

---

## Outcome (2026-09-08)

The paper builds, in HTML and PDF, with its Wording section in the right place
and every clause in `docs/wording-outline.md` present and in outline order --
23 of them, checked mechanically rather than by eye.

**Two corrections to what this section first claimed.**

`make wording-check` did not pass when it was written.  Adding `README.md` and
`specgen-ref` to `papers/wording/` broke it: the check regenerates into a
scratch directory and diffs, a scratch directory holds only output, and the two
authored files came back as "Only in papers/wording", which it reports as
stale.  It was last run *before* those files existed and was not run again.  The
diff now excludes the authored files, from a list `generate.sh` owns and shares
with its own cleanup.

And `wording-check` is not a CI job.  It was, briefly; see the drift gate
below.

**Heading levels: flat, deliberately** (task 3).  [clause-heading-level](../decisions.md#clause-heading-level)'s
`--base-heading-level` is still absent upstream, and the clauses come out at
`##`, which is the level the paper's own sections use, so they are siblings of
"Design" rather than children of "Wording".  With `toc-depth: 2` the contents
read as a clause list under the Wording heading, which is close enough to right
that spending an upstream flag and another cross-repository dependency on it
was not worth it.  A nested rendering needs the flag *and* `toc-depth: 3`.

**specgen#89 is worked around here and still open upstream.**  The warning was the
visible half.  The other half is that mpark emits the link anyway, so every
clause heading and every cross-reference in the published paper pointed at a
`c++draft` page that does not exist -- 23 distinct names, all of them clauses
this paper adds.  `generate.sh` now drops the `.sref` class from names under
this paper's two stable-name roots, which leaves `[transcode.iconv]`, what the
draft itself prints.  Keyed on the roots on purpose: a citation of a clause
that *is* in the draft keeps its `.sref` and still resolves.  Filed as
[specgen#89](https://github.com/steve-downey/specgen/issues/89); the `sed`
comes out when that lands.  The paper build is warning-free apart from a
pre-existing duplicate `email` key in the author block, which is authorship
metadata and not this step's to decide.

### Three things that were wrong and are not any more

- **The bibliography resolved nothing.**  The paper carried
  `bibliography: [../docs/*.bib]`, which mpark overrides with its own
  `csl.json` through `-d doc`, so all seven non-WG21 citations --
  `whatwg-encoding` included, cited in the abstract -- silently resolved to
  nothing.  The framework's mechanism is a `references` metadata block, which
  now lives in `parts/tail.md` beside the `::: {#refs}` div that places the
  bibliography (task 4).  `P1629R1` is deliberately not in it: it is a WG21
  paper and resolves from wg21.link on its own.
- **`.gitignore` ignored the paper's tail.**  `parts/` comes from the Python
  template, where it is buildout's top-level directory, but the pattern is
  unanchored and matched `papers/parts/` -- the directory task 2 puts the
  paper's tail in.  Committing would have silently dropped it and the paper
  would not have built from a fresh clone.
- **Two headings were swallowed.**  Splitting the file left "Questions for
  SG16" and "Wording" glued to the paragraph above them, so pandoc read them as
  text.  Both were caught by rebuilding and reading the heading tree, which is
  the check worth keeping: the build does not fail on this, it just quietly
  produces a paper with two fewer sections.

### Task 7: the code blocks, and one thing this step did not fix

Two real drifts, both fixed:

- **`legacy_byte_range`** is `ranges::range`, not `ranges::input_range`, and it
  is written on the exposition-only `$legacy-byte-type$` over
  `remove_cv_t<range_value_t<R>>` rather than a four-way `same_as` disjunction.
  The paper showed the pre-Step-4 form.  (The asymmetry with
  `unicode_scalar_range`, which *does* require `input_range`, is real and
  unexplained anywhere in the repo.  It is left alone here rather than given an
  invented rationale.)
- **`enum class codec`** had all 40 enumerators in the wrong order: the paper
  put `utf_16be`/`utf_16le` after `utf_8`, the header puts them after
  `x_mac_cyrillic`, and everything between shifted by two.  Enumerator order is
  something this paper fixes even though it fixes no values, so the two had to
  agree.

Both error enumerations already matched exactly.

**The one this step did not fix**, because fixing it is a scope decision and
not a transcription:  `decode_to`, `encode_to`, `decode_into` and `encode_into`
are proposed by the paper in five places -- the "Bulk conversion to owned
storage" section that argues the case and concludes "However, we propose the
names", the Overview bullets, four ticked rows of the API surface table,
"Questions for SG16", and the header's own comment ("They are proposed") --
and they have **no wording**, because `docs/wording-outline.md` recorded them
as "Not proposed at all" the day *after* the commit that proposed them, citing
a header that says the opposite.  Step 3 then kept
`detail/bulk_transcode.hpp` outside the gathered region on that authority.
They are public API in a `detail/`-pathed file that Step 3's promotion passed
over.  Recorded in full in `docs/wording-outline.md`; the fix is a header
promotion plus a `[transcode.bulk]` clause, which is a step of its own.

### The drift gate, and the workflow that did not survive

The first attempt was a workflow that installed LLVM, built specgen from
`papers/wording/specgen-ref`, and ran `make wording-check` and `--validate` on
every pull request.  It was written, pushed, and removed, and it failed twice
over before it was.

**It does not work.**  specgen built and ran, then died on
`/usr/include/wchar.h: fatal error: 'stddef.h' file not found` -- a specgen
built against an unpacked LLVM tarball derives its resource directory from the
running binary's path rather than from the LLVM it links, so Clang's own
builtin headers are not found.  The parse tail would need `-resource-dir` or
`SPECGEN_GCC_TOOLCHAIN`.  It works on a developer machine because Clang finds
the system GCC by itself, which is exactly the kind of difference that only
shows up in a container.

**It should not work.**  CodeQL raised three high-severity
`actions/cache-poisoning/poisonable-step` alerts against it, and they are
right: checking out a second repository at a ref read from a file, building it,
and running it, in a job that can write the default-branch Actions cache, is
executing someone's code with this repository's privileges.

**And it costs too much anyway.**  An LLVM download and a from-source build of
a tool this repository does not otherwise depend on is not a reasonable price
for a check on a paper.

What runs on every pull request instead is `make wording-inputs-check`, a step
in the Makefile workflow.  It hashes the document extent -- each root plus the
headers included inside its gathered region -- against a committed
`inputs.sha256`, needs no specgen, and takes no measurable time.  It catches a
header edited without regenerating, which is the drift that actually happens.
It cannot catch a fragment stale in a way the headers do not show; `make
wording-check` answers that, locally, before a revision goes out.

The specgen revision stays committed in `papers/wording/specgen-ref`, so what
generated the fragments is a fact in the tree.  It names a branch today, which
is right while the two repositories move together; it should become a SHA when
they stop.  A released specgen binary, or a container with one in it, is what
would make the strong check affordable and safe at the same time.

`papers/wording/README.md` -- Step 1 task 6, which never landed -- says the
fragments are generated, how to regenerate them, and what the gate checks.
