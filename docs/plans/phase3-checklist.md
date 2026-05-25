# Phase 3 Progress Checklist

Mark items `[x]` as they complete. Read this file first when resuming work.

**Current state:** Not started. Phase 2 remains the correctness track. Phase 3
is the benchmarking track and uses `P3-stepN` identifiers.

---

## P3-Step 1: Benchmark harness scaffolding (`p3-step1-benchmark-harness`)

- [ ] Create branch `p3-step1-benchmark-harness` from `main`
- [ ] Add dedicated top-level `benchmark/` CMake surface and Catch2 benchmark executable pattern
- [ ] Ensure benchmark code is compiled in normal project builds/CI
- [ ] Add Makefile benchmark smoke target that does not affect default `make test`
- [ ] Add unit-test coverage strategy for reusable benchmark support code
- [ ] Add one trivial benchmark proving the harness works and output is stable
- [ ] `make test` + `make lint`
- [ ] Run the benchmark smoke Makefile target
- [ ] Push both remotes + merge to `main`

## P3-Step 2: Benchmark corpus acquisition and provenance (`p3-step2-benchmark-data`)

- [ ] Create branch `p3-step2-benchmark-data` from `main`
- [ ] Add typed download/materialization tooling for benchmark corpora
- [ ] Add pristine source data + `SOURCE.md`/`source.bib` provenance
- [ ] Generate a corpus manifest with sizes, encodings, and tags
- [ ] Add Python tests for the tooling
- [ ] `make test` + `make lint`
- [ ] Run the data-tool smoke check
- [ ] Push both remotes + merge to `main`

## P3-Step 3: Benchmark fixture library and result schema (`p3-step3-benchmark-fixtures`)

- [ ] Create branch `p3-step3-benchmark-fixtures` from `main`
- [ ] Add benchmark fixture helpers for corpus loading, chunking, sinks, and metadata
- [ ] Add stable JSON/CSV result schema and writer helpers
- [ ] Add ordinary unit tests for reusable benchmark fixture/schema helpers
- [ ] Add smoke benchmarks using the helpers
- [ ] `make test` + `make lint`
- [ ] Run the benchmark smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 4: UTF-family and ASCII fast-path benchmarks (`p3-step4-utf-benchmarks`)

- [ ] Create branch `p3-step4-utf-benchmarks` from `main`
- [ ] Add UTF-8/UTF-16 eager benchmarks for `beman::transcode`
- [ ] Add ASCII fast-path corpus cases
- [ ] Add smoke benchmark subset and expected labels
- [ ] `make test` + `make lint`
- [ ] Run the UTF benchmark smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 5: Legacy WHATWG codec benchmarks (`p3-step5-legacy-whatwg-benchmarks`)

- [ ] Create branch `p3-step5-legacy-whatwg-benchmarks` from `main`
- [ ] Add single-byte representative benchmarks
- [ ] Add multi-byte/CJK representative benchmarks
- [ ] Ensure corpus-to-codec mapping is encoded in metadata
- [ ] `make test` + `make lint`
- [ ] Run the legacy benchmark smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 6: `iconv` baselines (`p3-step6-iconv-baselines`)

- [ ] Create branch `p3-step6-iconv-baselines` from `main`
- [ ] Add raw `iconv` C baseline benchmark kernels
- [ ] Add `iconv_transcode_view` benchmark kernels with comparable inputs
- [ ] Add smoke comparisons for at least UTF-8 and one legacy codec
- [ ] `make test` + `make lint`
- [ ] Run the `iconv` benchmark smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 7: `std::codecvt` negative baseline (`p3-step7-codecvt-baseline`)

- [ ] Create branch `p3-step7-codecvt-baseline` from `main`
- [ ] Add conditional `std::codecvt` benchmark support or explicit skip path
- [ ] Document compiler/library limitations in the benchmark README
- [ ] Keep non-supporting environments green
- [ ] `make test` + `make lint`
- [ ] Run the `codecvt` smoke command or verify documented skip
- [ ] Push both remotes + merge to `main`

## P3-Step 8: `encoding_rs` baseline (`p3-step8-encoding-rs-baseline`)

- [ ] Create branch `p3-step8-encoding-rs-baseline` from `main`
- [ ] Add optional build/invocation path for `encoding_rs` benchmarks
- [ ] Isolate the FFI or helper-binary boundary from core library code
- [ ] Add smoke comparisons on matching corpora/codecs
- [ ] `make test` + `make lint`
- [ ] Run the `encoding_rs` smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 9: `simdutf` ceiling baseline (`p3-step9-simdutf-baseline`)

- [ ] Create branch `p3-step9-simdutf-baseline` from `main`
- [ ] Add optional `simdutf` dependency wiring
- [ ] Add UTF benchmark kernels aligned with P3-Step 4 cases
- [ ] Add smoke comparisons and clear feature gating
- [ ] `make test` + `make lint`
- [ ] Run the `simdutf` smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 10: Chunked/lazy boundary stress benchmarks (`p3-step10-boundary-stress`)

- [ ] Create branch `p3-step10-boundary-stress` from `main`
- [ ] Add chunked-range benchmark cases for `beman::transcode`
- [ ] Add deterministic mock-iconv `EINVAL` / `E2BIG` stress kernels
- [ ] Add synthetic failure/error corpora to the benchmark fixture set
- [ ] `make test` + `make lint`
- [ ] Run the boundary benchmark smoke command
- [ ] Push both remotes + merge to `main`

## P3-Step 11: Toolchain matrix automation (`p3-step11-toolchain-matrix`)

- [ ] Create branch `p3-step11-toolchain-matrix` from `main`
- [ ] Add scripts/presets and Makefile targets for GCC `-O3`, GCC `-O3 -flto`, Clang `-O3 -flto`
- [ ] Capture compiler, CPU, OS, and build metadata alongside results
- [ ] Add reproducible matrix runner documentation
- [ ] `make test` + `make lint`
- [ ] Run at least one automated matrix smoke slice
- [ ] Push both remotes + merge to `main`

## P3-Step 12: Reporting, dashboard, and reproducibility pack (`p3-step12-reporting-and-repro`)

- [ ] Create branch `p3-step12-reporting-and-repro` from `main`
- [ ] Add result post-processing from Catch2 timing to GiB/s tables
- [ ] Add Vega-Lite or equivalent chart generation
- [ ] Write reproducibility/reporting docs for future reruns
- [ ] `make test` + `make lint`
- [ ] Run reporting generation on a smoke dataset
- [ ] Push both remotes + merge to `main`

---

## Notes

- Each step's detailed plan: `docs/plans/p3-step<N>-<slug>.md`
- Phase 3 overview: `docs/plans/phase3-index.md`
- Branch naming convention: `p3-step<N>-<slug>`
- Keep full matrix runs out of routine step validation unless the step says otherwise
- `make test` should include unit tests for benchmark-support code but should not execute benchmark workloads
- Benchmark smoke and matrix runs should be exposed via documented `make` targets
- Record smoke commands and dependency caveats in `handoff-next.md`
