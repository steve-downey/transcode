# Phase 3: Benchmarking and Performance Characterization

## Overview

Builds a reproducible benchmarking program for `beman::transcode` that
measures the library's throughput, latency-sensitive boundary behavior,
and abstraction costs against the baselines called out in the benchmarking
report: raw `iconv`, `iconv_transcode_view`, `std::codecvt`, `encoding_rs`,
and `simdutf`. The success condition is a benchmark suite whose code is always
compiled in normal project builds, whose reusable support code is covered by
ordinary unit tests, and whose benchmark execution can be run in small smoke
mode during ordinary step work and in a full matrix mode for proposal-quality
reporting.

**Preconditions:** Phase 2 feature work is functionally stable on `main`.
Step 50 is complete. `make test` and `make lint` are green before starting.

**Workflow per P3 step:** Branch `p3-step<N>-<slug>` from `main` -> add the
smallest missing benchmark/test/tooling slice -> run `make test` and
`make lint` -> run the step's required benchmark smoke check through the
Makefile workflow -> update `handoff-next.md` with the next `P3-stepN` file,
expected `make` targets, and any environment caveats.

**Progress:** See [phase3-checklist.md](phase3-checklist.md) for status.

---

## Why Phase 3 Is Separate

Phase 2 is still free to grow with additional conformance and quality work.
This phase therefore uses an independent `P3-stepN` namespace instead of
continuing the Phase 2 numeric sequence. That keeps the benchmarking track
stable even if more P2 items are added later.

This phase also needs a different execution discipline than P2:

1. Ordinary step work should validate with a narrow smoke benchmark, not the
   entire hardware/compiler matrix.
2. Expensive comparative runs should be deferred until the harness, corpora,
   and reporting format are stable.
3. Performance regressions and correctness bugs found during benchmarking
   should usually be handed back to P2 as targeted quality steps instead of
   silently folded into benchmark work.
4. Benchmark code must compile continuously with the project, but benchmark
   execution must remain outside the fast `make test` path.
5. Reusable benchmark support code should have ordinary unit tests, just like
   any other non-trivial code.
6. Benchmark execution should be reachable through stable Makefile targets,
   not only through direct binary invocation or ad hoc shell commands.

---

## Phase Scope

The benchmarking report calls for four dimensions:

1. **Implementations:** `beman::transcode`, `iconv`, `iconv_transcode_view`,
   `std::codecvt`, `encoding_rs`, `simdutf`
2. **Corpora:** Wikipedia Mars language sets, WPT-derived edge vectors,
   fuzz/error corpora
3. **Pipeline topologies:** monolithic/eager versus chunked/lazy
4. **Toolchains:** GCC `-O3`, GCC `-O3 -flto`, Clang `-O3 -flto`

The plan below stages these dimensions so each step is isolated and leaves the
tree green.

---

## P3 Steps

### Harness and data foundations

#### [P3-Step 1: Benchmark harness scaffolding](p3-step1-benchmark-harness.md)
Create the dedicated top-level `benchmark/` build surface, CMake wiring, Catch2
benchmark executable pattern, benchmark labels, and smoke-run entry points.
This step establishes the rule that benchmark code is always built, benchmark
support code is testable, and benchmark runs do not occur in the default
`make test` path.

#### [P3-Step 2: Benchmark corpus acquisition and provenance](p3-step2-benchmark-data.md)
Acquire the real-world and synthetic corpora called for by the report,
including provenance, checksums, and a generated manifest. No benchmark logic
yet.

#### [P3-Step 3: Benchmark fixture library and result schema](p3-step3-benchmark-fixtures.md)
Add reusable corpus-loading, chunking, sink, and metadata helpers plus a
stable output schema for later post-processing to GiB/s and charts.

### Native `beman::transcode` measurement

#### [P3-Step 4: UTF-family and ASCII fast-path benchmarks](p3-step4-utf-benchmarks.md)
Measure the core algorithmic UTF paths for `beman::transcode`, including ASCII
fast paths and UTF-8 <-> UTF-16 conversions.

#### [P3-Step 5: Legacy WHATWG codec benchmarks](p3-step5-legacy-whatwg-benchmarks.md)
Measure representative single-byte and multi-byte WHATWG codecs, with corpora
chosen to expose table lookups, branch pressure, and trie/state-machine costs.

### Baseline implementations

#### [P3-Step 6: `iconv` baselines](p3-step6-iconv-baselines.md)
Add raw `iconv` and `iconv_transcode_view` benchmarks so the zero-cost wrapper
claim can be quantified directly.

#### [P3-Step 7: `std::codecvt` negative baseline](p3-step7-codecvt-baseline.md)
Add the deprecated standard-library comparison path as a conditional benchmark
surface with clear skip behavior when unavailable or broken.

#### [P3-Step 8: `encoding_rs` baseline](p3-step8-encoding-rs-baseline.md)
Integrate an `encoding_rs` benchmark baseline through an isolated FFI or helper
binary path, matching the existing oracle direction in repository docs.

#### [P3-Step 9: `simdutf` ceiling baseline](p3-step9-simdutf-baseline.md)
Add the hardware-accelerated ceiling baseline, gated behind an optional build
switch so the normal tree stays portable.

### Stress paths and final reporting

#### [P3-Step 10: Chunked/lazy boundary stress benchmarks](p3-step10-boundary-stress.md)
Add chunked-range, `EINVAL`, `E2BIG`, and mock-iconv stress benchmarks to
measure the real abstraction penalty of lazy pipelines at boundaries.

#### [P3-Step 11: Toolchain matrix automation](p3-step11-toolchain-matrix.md)
Automate the compiler/flag matrix and capture environment metadata so results
are comparable across runs and machines.

#### [P3-Step 12: Reporting, dashboard, and reproducibility pack](p3-step12-reporting-and-repro.md)
Turn Catch2 timing output plus benchmark metadata into proposal-ready tables,
GiB/s summaries, and reproducible documentation for future reruns.

---

## Key Architecture Decisions

| Decision | Rationale |
|----------|-----------|
| Separate top-level `benchmark/` tree from library code while still compiling it in normal builds | Keeps benchmark implementation out of the shipped library surface without letting it rot unbuilt |
| Smoke mode first, full matrix last | Prevents routine step work from turning into multi-hour hardware experiments |
| Provenanced corpora under `docs/` + derived manifests under `data/` | Matches the repository's existing WHATWG/WPT data discipline |
| Reusable benchmark fixture layer before implementation-specific suites | Keeps later baseline additions narrow and consistent |
| Benchmark-support helpers get ordinary unit tests | Keeps non-trivial support code in the same quality loop as the rest of the project |
| Benchmark runs exposed through Makefile targets | Keeps the workflow discoverable and consistent with the rest of the project |
| External baselines are optional CMake features | Preserves portability and prevents missing dependencies from breaking normal work |
| Performance findings that imply functional bugs go back to P2 | Avoids hiding correctness fixes inside benchmark-only steps |
| Stable JSON/CSV output schema before charting | Makes later reporting scripts mechanical rather than ad hoc |

---

## Handoff Notes For This Phase

Your Phase 2 kickoff style still works, but benchmarking needs two extra rules:

1. The prompt should name the active `P3-stepN` file explicitly, because the
   checklist alone no longer determines the numeric next step globally.
2. The prompt should ask for the step's **smoke benchmark** only, unless the
   step explicitly says to run the full matrix.
3. The prompt should preserve the rule that `make test` covers benchmark-support
   unit tests but does not execute benchmark workloads.
4. The prompt should prefer the documented `make` benchmark targets over raw
   benchmark-binary commands.

`handoff-next.md` should always record:

- active `P3-stepN` file
- exact `make` targets expected to be green
- whether any optional baseline dependency is required for that step
- whether a follow-up should stay in P3 or be kicked back to P2
