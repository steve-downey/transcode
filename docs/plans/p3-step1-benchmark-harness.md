# P3-Step 1: Benchmark Harness Scaffolding

**Branch:** `p3-step1-benchmark-harness`
**Depends on:** Phase 2 stability on `main`

---

## Goal

Create the dedicated top-level `benchmark/` build surface for the project:
CMake wiring, Catch2 benchmark executable pattern, benchmark labels/tags, and
one trivial smoke benchmark proving the harness works.

This step is intentionally about structure, not real benchmark coverage.

## Why This Step Exists

Benchmarking needs a different execution model than ordinary tests:

1. benchmark binaries must compile in normal CI/dev workflows
2. default `make test` must stay fast and deterministic
3. step work still needs a narrow, reproducible smoke benchmark
4. later steps need a stable place to add benchmark cases without redesigning
   build plumbing each time
5. reusable benchmark support code still needs normal unit tests

## Deliverables

- New top-level `benchmark/` directory at the repo root, alongside `tests/`
- A dedicated benchmark executable or small set of executables using Catch2 v3
  benchmark support
- Build wiring that compiles benchmark code in ordinary project builds without
  executing the benchmarks from `make test`
- At least one Makefile target for benchmark execution so the workflow is
  accessible without manual binary discovery
- A unit-test strategy for any reusable benchmark support code
- A smoke command documented in the step and in `handoff-next.md`
- One trivial benchmark that measures a tiny fixed transcoding kernel and proves
  output generation works

## Files Expected

- top-level `CMakeLists.txt`
- `Makefile`
- new `benchmark/CMakeLists.txt`
- new benchmark source file under `benchmark/`
- test registration for benchmark-support helpers if any support code is added
- one initial benchmark source file
- optional README for benchmark commands

## Constraints

- Do not add benchmark cases to the default `make test` path
- Ensure benchmark sources are still compiled in ordinary project builds
- Expose the smoke benchmark through a `make` target rather than only a raw
  executable path
- If this step introduces reusable helper code, add ordinary unit tests for it
- Do not require external dependencies yet
- Prefer one executable validation command such as a dedicated benchmark smoke
  target exposed through the Makefile workflow

## Procedure

1. Create branch `p3-step1-benchmark-harness` from `main`
2. Add a benchmark-only CMake subtree
3. Wire Catch2 benchmark support into a dedicated executable that is compiled in normal builds
4. Add a Makefile target for the benchmark smoke run
5. If any reusable support code is needed, add ordinary unit tests for it
6. Add one tiny benchmark kernel over an already-supported conversion path
7. Document the smoke `make` target
8. Run `make test`
9. Run `make lint`
10. Run the benchmark smoke Makefile target
11. Update `handoff-next.md` with the next step and the verified `make` target

## Verification

```bash
make test
make lint
# plus the dedicated benchmark smoke Makefile target introduced in this step
```

## Notes

Use this step to settle naming and directory layout. Every later P3 step should
extend the top-level `benchmark/` tree without revisiting global build
structure. The key outcome is: benchmark code builds all the time, benchmark
support code can be unit-tested, and benchmark execution remains a separate
operation reachable through the Makefile workflow.
