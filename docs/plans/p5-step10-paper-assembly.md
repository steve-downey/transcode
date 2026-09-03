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
   cannot know.  Per index D6 this is authored prose, not `--paper` mode.
2. **Fix the section's position.**  Wording goes after "Impact on the Standard"
   and before the bibliography.  Because transclusion is prerequisite order
   (index D3), everything that must follow the wording — Acknowledgements, Prior
   Art, Relationship to Other Proposals, References — moves out of
   `transcode-view.md` into a second authored file, listed after `$(WORDING_MD)`
   in the paper's prerequisites.  `flat.mk` globs `papers/*.md` and would build
   that file as a paper of its own, so it lives in a subdirectory:
   `papers/parts/tail.md`.  The YAML metadata block stays at the top of
   `transcode-view.md`, which remains the first input.
3. **Resolve heading levels.**  Generated clause headings are `##`
   (index U2).  Either accept them as siblings of the paper's own sections, or
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
   spec-facing header must be clean except for index U1's known `ranges` noise;
   when U1 lands upstream, remove the allowlist and make the check absolute.
7. **Bump the paper's revision** and update the abstract to say the wording is
   generated from the reference implementation — that is a claim reviewers
   should be able to check, and it is the phase's headline.

## Acceptance

- `make -C papers transcode-view.pdf` produces a paper whose Wording section
  contains every clause in `docs/wording-outline.md`, in outline order.
- `make wording-check` runs in CI and fails on a hand-edited fragment.
- No pandoc warnings about unresolved `.sref` spans.
- `make lint` and `make test` green.
