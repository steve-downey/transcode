# Handoff: beman.transcode — Step 54

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

**Step 53b (unreachable code annotations) complete and merged to main.**

608 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in docs (codespell: "implementor" in
`docs/Rust Encoding for C++ Transcode.md`, "bu" in
`papers/wg21/generated/TEST.html`). All C++, CMake, tools, and ruff
lint passes cleanly (papers/wg21 now excluded from ruff via
`extend-exclude` in pyproject.toml).

### What Step 53b Added

Backfilled `std::unreachable()` annotations for proven dead code paths
that had been identified in step 53 coverage analysis:

**`gb18030.hpp`** — 4 locations marked unreachable:
- Lines 70, 72: Pre-check on line 51 prevents pointer overflow conditions
  (`pointer > 39419 && pointer < 189000` || `pointer > 1237575`)
- Line 76: Binary search can't fail — range 0 starts at pointer 0
- Line 96: gb18030 covers all Unicode — encode always finds a match in ranges table
- Line 172: GBK table has no zero entries (WHATWG-normative)

**`whatwg_decode_view.hpp`** — 1 location marked unreachable:
- Line 869: windows_1252 error branch — WHATWG normative table has no null entries

Also added `#include <utility>` to gb18030.hpp to support `std::unreachable()`.

### Coverage Summary

Overall: 85.4% lines, 99.9% functions (unchanged from step 53).

Remaining meaningful coverage gaps (not dead code):
- `whatwg_decode_view.hpp`: some paths in GBK/GB18030 replay logic
  and ISO-2022-JP state machine
- `iconv_transcode_view.hpp`: ~5% uncovered (error paths in iconv
  interaction)
- `iconv_transcode_or_error_view.hpp`: ~7% uncovered

## What To Do Next — Step 54

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–53b are complete. The checklist does not yet have a step 54 entry.

### Recommended options for step 54

**Option A: C++23 module support update** — ensure `transcode.cppm`
exports all headers added since step 42 (umbrella header). Run:
```bash
grep '#include' include/beman/transcode/transcode.hpp
```
and compare with what's exported in `transcode.cppm`. Any missing
includes = missing module exports. This is a quick audit with potentially
one line of adds.

**Option B: `whatwg_decode_view.hpp` remaining coverage** — the replay
logic for GB18030/GBK and some ISO-2022-JP state machine paths still
have gaps. Requires understanding the view's iterator states and writing
targeted edge-case tests. Run `make coverage` and inspect the JSON for
details on uncovered lines.

**Option C: Begin Phase 3 (benchmarking)** — plan docs exist in
`docs/plans/phase3-*.md`. This is a multi-step track setting up
benchmarking infrastructure (harness, corpora, baselines). First step
is P3-step1 in `docs/plans/p3-step1-benchmark-harness.md`. Option C is
recommended if the benchmarking report is a near-term priority.

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
make coverage  # gcovr coverage report
```

## Coding Rules (abbreviated)

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_LABELS_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files for context

- `include/beman/transcode/transcode.hpp` — umbrella header (added in step 42)
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view (97.1% covered, replay/state gaps)
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view (100% covered)
- `include/beman/transcode/detail/gb18030.hpp` — GB18030 codec (now with unreachable markers)
- `transcode.cppm` — C++23 module interface (optional, may need header audit)
- `docs/plans/phase3-*.md` — Phase 3 benchmarking plans
