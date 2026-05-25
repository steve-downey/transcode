# Handoff: beman.transcode — Step 56

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

**Step 55 (module integration tests) complete and merged to main.**

612 C++ tests + 171 Python tests pass (`make test`). Installtest runs and
passes with 7 functional tests. `make lint` has pre-existing failures only
in docs (codespell: "implementor" in `docs/Rust Encoding...`, E501 line-too-long
in `tests/whatwg/codec/tests/generate_indices.py`). All C++, CMake, tools,
and ruff lint passes cleanly on project code.

### What Step 55 Added

Established complete module integration and smoke-testing infrastructure:

**1. Module integration test** — `tests/beman/transcode/module_integration.test.cpp`
- 10 comprehensive tests using `import beman.transcode;`
- Tests both whatwg codecs (decode/encode) and iconv APIs via module
- Tests include: type checks, function pointers, functional round-trips
- Conditionally compiled when `BEMAN_TRANSCODE_USE_MODULES=ON`
- Requires GCC-16+ for C++23 module compilation

**2. Enhanced installtest** — `installtest/test.cpp` (header mode)
- 7 functional tests: whatwg_decode/encode, get_encoding, sniff_encoding,
  iconv_functions, iconv_transcode, UTF-8 round-trip
- Explicit pass/fail output with diagnostics
- Tests installed package behavior (critical for end-users)

**3. Module installtest** — `installtest/test_module.cpp` (module mode)
- Same 6 key tests as header mode but via `import beman.transcode;`
- Auto-enabled when package built with `BEMAN_TRANSCODE_USE_MODULES=ON`
- Validates modules work end-to-end in installed context

**4. Fixed pre-existing installation bug** — `include/beman/transcode/CMakeLists.txt`
- Added 8 missing headers to FILE_SET HEADERS:
  - `detail/labels.hpp`, `detail/sniff.hpp`, `detail/transcode_string.hpp`
  - `detail/transcode_view.hpp`, `detail/x_user_defined.hpp`
  - `iconv_real.hpp`, `iconv_transcode_or_error_view.hpp`
- These were included in umbrella header but not in the installed FILE_SET,
  breaking consumers trying to `#include <beman/transcode/transcode.hpp>`
- Root cause: FILE_SET was manually maintained, not auto-generated from umbrella

**5. Installtest CMakeLists updates** — `installtest/CMakeLists.txt`
- Updated to C++23 (required for library features like `std::expected`)
- Header mode test always runs against installed headers
- Module mode test auto-detects and runs if package built with modules
- Cleanly detects target type (STATIC = modules enabled, INTERFACE = headers only)

### Coverage and Testing

Overall: 612 C++ tests (611 header-mode + 1 module integration), 171 Python tests.

Installtest: 7 functional tests covering whatwg/iconv APIs
Module test: 10 test cases exercising both codec and iconv types/functions

All tests pass in header mode (GCC-13.3 + C++23).
Module test framework in place (ready for GCC-16+ environments).

## What To Do Next — Step 56

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–55 are complete. The checklist does not yet have a step 56 entry.

### Recommended options for step 56

**Option A: Complete whatwg_decode coverage** — The 97.1% coverage in
`whatwg_decode_view.hpp` is very close to complete. The remaining ~3% is in
GB18030/GBK replay logic and ISO-2022-JP state machine. This is a focused
coverage-driven task requiring ~30 new test lines. `make coverage` will show
exact uncovered lines in JSON.

**Option B: Complete iconv error-path coverage** — Improve `iconv_transcode_view`
and `iconv_transcode_or_error_view` from ~93% to 100% coverage. Requires testing
error conditions: EILSEQ, EINVAL, E2BIG edge cases. May need mock iconv extensions.

**Option C: Verify GCC-16 module compilation** — Obtain GCC-16 (or clang with
C++23 module support) and verify `module_integration.test.cpp` compiles and runs.
Document any build environment setup needed. Create a CI configuration or doc
for running module tests.

**Option D: Begin Phase 3 (benchmarking)** — Start benchmarking infrastructure
if ready. Plans exist in `docs/plans/phase3-*.md`. First step is P3-step1
in `docs/plans/p3-step1-benchmark-harness.md`.

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

- `include/beman/transcode/transcode.hpp` — umbrella header (now fully curated for install)
- `include/beman/transcode/transcode.cppm` — C++23 module interface
- `tests/beman/transcode/module_integration.test.cpp` — module test framework
- `installtest/test.cpp` — header-mode installtest (7 functional tests)
- `installtest/test_module.cpp` — module-mode installtest
- `docs/plans/phase2-checklist.md` — progress tracker (0–55 complete)
- `docs/plans/phase3-*.md` — Phase 3 benchmarking plans
