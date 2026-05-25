# Handoff: beman.transcode — Step 55

## Project

`beman.transcode` — C++20 header-only library for Unicode transcoding
using ranges and views. Beman Project incubation for a C++29 standard
proposal.

- GitHub: `https://github.com/steve-downey/transcode.git` (remote `origin`)
- Bloomberg: `bbgithub.dev.bloomberg.com:sdowney/transcode.git` (remote `bbgithub`)
- Working directory: `/home/sdowney/src/steve-downey/transcode/transcode`
- Two remotes; always push both: `git push origin <branch> && git push bbgithub <branch>`
- Merge to main: `git merge --no-ff`

## Current State

**Step 54 (module audit) complete and merged to main.**

611 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in docs (codespell: "implementor" in
`docs/Rust Encoding for C++ Transcode.md`, "bu" in
`papers/wg21/generated/TEST.html`, etc.). All C++, CMake, tools, and ruff
lint passes cleanly.

### What Step 54 Added

Completed C++23 module support audit by adding three iconv public API headers
to the umbrella header that were missing:

**`include/beman/transcode/transcode.hpp`** — added three iconv headers:
- `iconv_transcode_view.hpp` (added in step 13, but missing from umbrella)
- `iconv_transcode_or_error_view.hpp` (added in step 12, but missing from umbrella)
- `iconv_real.hpp` (added in step 11, but missing from umbrella)

These headers are public APIs at the top level of `include/beman/transcode/`
(not in `detail/`), so they were never intended to be internal. They provide
real OS iconv integration and should have been in the umbrella header since
step 42 added it. The module file (`transcode.cppm`) re-exports the umbrella
header, so all three are now properly exported by the C++23 module interface.

**`tests/beman/transcode/transcode.test.cpp`** — added three new tests:
- Verify `iconv_functions` is accessible from umbrella
- Verify `iconv_transcode_view<iconv_functions, ...>` is accessible
- Verify `iconv_transcode_or_error_view<iconv_functions, ...>` is accessible
- Verify `make_real_iconv_fns()` is accessible and returns valid function pointers

### Coverage Summary

Overall unchanged from step 53b: 85.4% lines, 99.9% functions.

Remaining coverage gaps (opportunities for step 55+):
- `whatwg_decode_view.hpp`: 97.1% covered; remaining gaps in GB18030/GBK replay
  logic and ISO-2022-JP state machine transitions (~30 lines)
- `iconv_transcode_view.hpp`: ~5% uncovered (error paths: EILSEQ, EINVAL, E2BIG)
- `iconv_transcode_or_error_view.hpp`: ~7% uncovered (error path coverage)

## What To Do Next — Step 55

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–54 are complete. The checklist does not yet have a step 55 entry.

### Recommended options for step 55

**Option A: `whatwg_decode_view` final coverage push** — Target the remaining
~3% of uncovered lines in `whatwg_decode_view.hpp`. Focus on:
- GB18030/GBK replay logic edge cases (state machine in `load()`)
- ISO-2022-JP state transitions not yet covered

Run `make coverage` and inspect the JSON report at `.build/build-system/coverage.json`
to find exact uncovered lines. This is a focused coverage-driven task (~30 lines to test).

**Option B: iconv error path coverage** — Add targeted tests for iconv error
conditions (`EILSEQ`, `EINVAL`, `E2BIG`) to improve `iconv_transcode_view.hpp`
and `iconv_transcode_or_error_view.hpp` coverage from ~93% → ~100%. Requires
understanding mock iconv interactions and edge cases. May require extending
`iconv_mock.hpp` if needed.

**Option C: Begin Phase 3 (benchmarking)** — Start benchmarking infrastructure
(plan docs exist in `docs/plans/phase3-*.md`). This is a multi-step track
setting up: (1) benchmark harness, (2) test corpus, (3) baseline collection.
First step is P3-step1 in `docs/plans/p3-step1-benchmark-harness.md`. Option C
is recommended if benchmarking is a near-term priority (e.g., for the WG21
standard proposal review cycle).

**Option D: Pop a defect or cleanup task** — Check `docs/plans/phase2-index.md`
or the checklist for any deferred TODO items or known defects that may have
been flagged during previous steps. These often become apparent after the
umbrella/module audit.

## TDD Process

Each step is a separate branch: `step<N>-<slug>`, branched from `main`.

1. Write failing tests (RED) → commit → push both remotes
2. Implement (GREEN) → `make test` (all pass) → `make lint` → commit → push both remotes
3. `git checkout main && git merge --no-ff step<N>-...` → push both remotes
4. Update `docs/plans/phase2-checklist.md` — mark completed items `[x]`

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report (JSON at .build/build-system/coverage.json)
```

## Coding Rules (abbreviated)

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_LABELS_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key Files for Context

- `include/beman/transcode/transcode.hpp` — umbrella header (now includes all iconv headers)
- `include/beman/transcode/transcode.cppm` — C++23 module interface (exports umbrella)
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view (97.1% covered)
- `include/beman/transcode/iconv_transcode_view.hpp` — iconv view (~93% covered)
- `include/beman/transcode/iconv_transcode_or_error_view.hpp` — iconv_or_error view (~93% covered)
- `docs/plans/phase2-checklist.md` — progress tracker (0–54 complete)
- `docs/plans/phase3-*.md` — Phase 3 benchmarking plans
