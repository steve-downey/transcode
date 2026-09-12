# P6-Step 10: Verification

**Branch:** `p6-step10-verification`
**Depends on:** every other step
**Closes:** the review's "Verification results and gaps" section

---

## Goal

Answer the questions the review left open, and confirm that the nine steps
before this one did what they said.

## Why a step of its own

The review is explicit that its own verification was partial:

> A fresh C++ test build was not completed.  CMake stalled while cloning
> Catch2.  Existing CTest executables predate recent public-header changes, so
> their results must not be treated as verification of the current tree.

So the review's clean-passes list -- 252 Python tests, mypy, formatting, the
examples under `g++-16`, the paper render, `wording-inputs-check` -- was
measured against a tree whose C++ tests had not been rebuilt.  None of it is
wrong; it is just narrower than it looks.  This step makes the checks match the
claim.

## What to run

### Build matrix

Fresh configure and build from a clean tree, not an incremental one:

```sh
make realclean
make test
```

Then each configuration that CI covers, and both module settings:

```sh
uv run cmake --build .build/build-system/ --config Debug
uv run cmake --build .build/build-system/ --config Asan
uv run cmake --build .build/build-system/ --config RelWithDebInfo
uv run ctest --test-dir .build/build-system/ -C Debug --output-on-failure
```

`BEMAN_TRANSCODE_USE_MODULES=ON` and `OFF` both configure, build and test.

### Install and consume

From a clean prefix, both modes.  Build a small consumer that includes
`<beman/transcode/transcode.hpp>` and one that imports the module, against the
*installed* package rather than the build tree.  This is what D-12 was about
and it has never been run.

### Examples

Every file under `examples/` compiles and runs against the changed headers.
The review did this with `g++-16` and it passed; Steps 3, 5 and 6 change what
the examples see, so it has to be redone rather than inherited.

### Wording

```sh
make wording-check      # regenerate and diff — the real check
make wording-inputs-check
make wording-validate
```

`papers/wording/PENDING` must be empty.  Read the `wording-validate` findings;
do not merely count them.  A coverage finding means a declaration has no home
and a leakage finding means the wording names something the reader cannot see,
and both are real.

### The tools the review could not run

- `codespell` was unavailable.  Install it and run it.
- `specgen` was unavailable, so full `make wording-check` never ran.  Step 8
  builds it; confirm here.

### Cross-implementation iconv

The review asks for the C-01 through C-05 regressions on "at least glibc iconv
and one other implementation."  glibc is what the development machine has.
musl in a container is the cheap second — Alpine, build, run the iconv tests.
This matters more than it sounds: `papers/transcode-view.md:52` argues that
glibc, musl and the BSD libraries disagree about error recovery, and Steps 2
and 3 changed how the library handles exactly that disagreement.

If a second implementation cannot be arranged, say so plainly in the report
rather than leaving the item ticked.

### Coverage

`make coverage`.  Steps 2 and 3 add error branches; those should be covered.
Branch coverage is not the target — line and function coverage is — but an
uncovered *new* error path is a missing test, not an artefact.

## Report

Write `docs/review-findings-2026-09-11-closeout.md` beside the original: each
of the 30 findings, what was done, and the commit that did it.  For anything
deferred, say what and why, so the next review does not rediscover it as new.

Findings expected to close as "documented, not fixed": S-03 (the U+FFFD
reservation, referred to SG16), P-01 and P-02 (scoped as
implementation-defined, with the split referred to SG16).  Those are decisions,
not omissions, and the closeout should read that way.

## Done when

- Every check above has been run and its result recorded.
- The closeout document exists and accounts for all 30 findings.
- Anything still open is named, with a reason.
