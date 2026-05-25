# P3-Step 3: Benchmark Fixture Library and Result Schema

**Branch:** `p3-step3-benchmark-fixtures`
**Depends on:** [p3-step2-benchmark-data.md](p3-step2-benchmark-data.md)

---

## Goal

Build the reusable support layer that later benchmarks share: corpus loading,
chunking helpers, sink utilities that prevent dead-code elimination, metadata
capture, and a stable output schema for post-processing.

Because this step introduces reusable support code rather than only benchmark
cases, that support code should receive ordinary unit tests and be covered by
`make test`.

## Deliverables

- benchmark fixture helpers in a dedicated internal header/source set
- corpus descriptor types tied to the manifest from P3-Step 2
- eager and chunked input adapters
- result metadata writers (JSON and/or CSV)
- ordinary unit tests for the reusable helper code
- smoke benchmarks using the helpers

## Files Expected

- `benchmark/benchmark_fixture.hpp`
- `benchmark/benchmark_fixture.cpp` if needed
- `benchmark/result_schema.hpp`
- one or more unit-test files for reusable benchmark-support helpers
- a small fixture smoke benchmark source file under `benchmark/`

## Design Requirements

- setup/allocation should happen outside the timed region where possible
- sinks must make it impossible for the compiler to optimize the work away
- metadata must include corpus id, codec, topology, implementation, and build
  settings available at runtime
- helper logic that can be tested without executing benchmarks should be tested
  through ordinary unit-test binaries

## Procedure

1. Create branch `p3-step3-benchmark-fixtures` from `main`
2. Add fixture helpers for corpus lookup and loading
3. Add chunked adapters and reusable output sinks
4. Add schema writers for machine-readable output
5. Add ordinary unit tests for the reusable helpers and schema logic
6. Convert the trivial P3-Step 1 benchmark to use the shared fixture layer
7. Run `make test`
8. Run `make lint`
9. Run the benchmark smoke command and inspect emitted metadata
10. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the benchmark smoke command, confirming structured output exists
```

## Notes

This step is the last infrastructure-only step before real benchmark coverage.
Later steps should mostly add kernels and case tables, not new plumbing. Any
helper introduced here should behave like ordinary project code: always built
and validated by unit tests.
