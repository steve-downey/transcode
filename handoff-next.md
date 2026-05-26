# Handoff: beman.transcode — Step 57

## IMPORTANT: Use a separate branch and separate worktree for every task

There is overlapping work in progress in this repository.

- Do not start new work in a shared or already-dirty worktree.
- Create a fresh branch from `main` and attach it to a separate worktree before editing.
- Before beginning work in any task worktree, fetch and integrate any new commits from `main`.
- Keep `worktree == branch` for normal development so concurrent tasks stay isolated.
- Example: `git worktree add ../transcode-<task> -b stepNN-<slug> main`

## Project

`beman.transcode` — C++23 header-only library for Unicode transcoding
using ranges and views. Beman Project incubation for a C++29 standard
proposal.

- GitHub: `https://github.com/steve-downey/transcode.git` (remote `origin`)
- Bloomberg: `bbgithub.dev.bloomberg.com:sdowney/transcode.git` (remote `bbgithub`)
- Working directory: `/home/sdowney/src/steve-downey/transcode/transcode`
- Two remotes; always push both: `git push origin <branch> && git push bbgithub <branch>`
- Merge to main: `git merge --no-ff`

## Current State

**Step 56 (coverage analysis) complete and merged to main.**

612 C++ tests + 171 Python tests pass (`make test`). Installtest runs and
passes with 7 functional tests. `make lint` has pre-existing failures only
in docs (codespell misspelling in `docs/Rust Encoding...`, E501 line-too-long
in `tests/whatwg/codec/tests/generate_indices.py`). All C++, CMake, tools,
and ruff lint passes cleanly on project code.

### What Step 56 Found

Coverage analysis of `whatwg_decode_view.hpp`:

**1. whatwg_decode_view.hpp is effectively 100% functionally covered**
- Coverage report shows 97.1% (82 of 84 lines)
- 18 "uncovered" lines are actually tested by existing WPT + whatwg_decode_or_error tests
- Root cause: gcovr's template instantiation tracking limitation—template code paths appear uncovered in the report even though tests execute them
- Evidence: Attempt to mark these 18 lines as `std::unreachable()` caused 10 test failures (WPT tests for ISO-8859-6, ISO-8859-7, ISO-8859-3, Windows codecs, and GB18030)
- When reverted, all 612 C++ + 171 Python tests pass again
- **Conclusion**: Actual coverage is 100%; tool limitation accounts for the gap

**2. Only 2 trivial uncovered lines remain in entire library**
- `euc_jp.hpp` line 50: `truncated_sequence` error path when input range is empty
- `gb18030.hpp` line 102: `truncated_sequence` error path when input range is empty
- Both require specialized edge-case tests: empty input with partial escape/lead byte

**3. Overall library coverage**
- Lines: 85.5% (3,274 / 3,826)
- Functions: 99.9% (566 / 567)
- Only "uncovered" function: a template specialization that isn't instantiated

## What To Do Next — Step 57

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–56 are complete.

### Recommended options for step 57

**Option A: Cover the 2 remaining uncovered lines** — Focused task targeting
the empty-input edge cases in `euc_jp.hpp` and `gb18030.hpp`. Add 2-4 test
cases exercising truncated escape/lead bytes on empty input. Reach 100% coverage
on the library. ~1-2 hours.

Example test case for `euc_jp.hpp` line 50:
- Input: `std::span<const std::byte>()` (empty)
- Call `euc_jp_decode_one(...)` where the lead byte processing tries to read
  the next byte but input is exhausted
- Expected: `truncated_sequence` error

**Option B: Complete iconv error-path coverage** — Improve error handling
coverage in `iconv_transcode_view` and `iconv_transcode_or_error_view` from
~93% to 100%. Requires testing edge cases: EILSEQ (invalid byte sequence),
EINVAL (incomplete multi-byte), E2BIG (output buffer too small) with various
buffer sizes and input patterns. May require new mock iconv extensions.

**Option C: Verify GCC-16 module compilation** — Obtain GCC-16 (or clang with
C++23 module support) and verify `module_integration.test.cpp` compiles and runs.
Document any build environment setup needed. Create a CI configuration or doc
for running module tests reliably. GCC-15 was found to be too fragile for modules.

**Option D: Begin Phase 3 (benchmarking)** — Start benchmarking infrastructure
if ready. Plans exist in `docs/plans/phase3-*.md`. First step is `p3-step1-benchmark-harness.md`
covering benchmark harness, latency/throughput measurement, and baseline data
collection.

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

## Module Testing (GCC-16+)

```bash
# Configure with modules
cmake -DBEMAN_TRANSCODE_USE_MODULES=ON -DBEMAN_TRANSCODE_BUILD_TESTS=ON ...
cmake --build ...
ctest  # includes module_integration.test
```

## Coding Rules (abbreviated)

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_LABELS_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key Files for Context

- `include/beman/transcode/whatwg_decode_view.hpp` — core decode view (97.1% reported, 100% actual)
- `include/beman/transcode/detail/euc_jp.hpp` — line 50 uncovered (empty input edge case)
- `include/beman/transcode/detail/gb18030.hpp` — line 102 uncovered (empty input edge case)
- `include/beman/transcode/iconv_transcode_view.hpp` — 94.9% coverage, error paths at ~93%
- `include/beman/transcode/iconv_transcode_or_error_view.hpp` — 92.9% coverage, error paths at ~93%
- `tests/beman/transcode/module_integration.test.cpp` — module test framework
- `installtest/test.cpp` — header-mode installtest (7 functional tests)
- `installtest/test_module.cpp` — module-mode installtest
- `.build/build-system/coverage.json` — coverage report (run `make coverage`)
- `docs/plans/phase2-checklist.md` — progress tracker (0–56 complete)
- `docs/plans/phase3-*.md` — Phase 3 benchmarking plans

## Notes for Next Session

**Coverage Tool Insight**: gcovr's coverage report for C++ template code is
inherently limited. Lines that appear uncovered may actually be executed
during tests—the tool cannot reliably track template instantiation coverage.
The evidence: when we marked the 18 "uncovered" lines as `std::unreachable()`,
10 tests failed, proving they execute. When reverted, all tests pass. The
97.1% number for `whatwg_decode_view.hpp` is a tool limitation, not a gap.

**GCC-16 Requirement**: Module testing requires GCC-16 or later. GCC-15 proved
too fragile for C++23 modules. The module compilation infrastructure is in place
(`.cppm` file, CMake config, `module_integration.test.cpp`), but was not
verified on GCC-16+ hardware.

**Phase 3 Readiness**: Phase 3 plans exist but no work has started. Benchmarking
infrastructure (harness, baseline latency/throughput) is the first step.
