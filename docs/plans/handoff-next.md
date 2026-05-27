# Handoff: Next Step

## Completed

- **P3-Step 1: Benchmark harness scaffolding** — done on `worktree-pluggable-codec-protocol` branch

## What was done

- Added `BEMAN_TRANSCODE_BUILD_BENCHMARKS` option to `CMakeLists.txt` (default `${PROJECT_IS_TOP_LEVEL}`)
- Added `add_subdirectory(benchmark)` guarded by that option (after the examples block)
- Created `benchmark/CMakeLists.txt` — registers `beman.transcode.benchmarks.smoke` linked to `beman::transcode` and `Catch2::Catch2WithMain`
- Created `benchmark/smoke.bench.cpp` — one `BENCHMARK` case decoding a 4096-byte ASCII string via `whatwg_decode<codec::utf_8>`
- Added `make bench` target to `Makefile` — depends on `compile`, runs the smoke binary with `"[smoke]"` tag filter

## Files created

- `benchmark/CMakeLists.txt`
- `benchmark/smoke.bench.cpp`

## Next Step

Read `docs/plans/p3-step2-benchmark-data.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Current State

- `make test` passes (662 C++ + 171 Python tests)
- `make lint` passes
- `make bench` works: executable at `.build/build-system/benchmark/Asan/beman.transcode.benchmarks.smoke`
- Smoke benchmark reports ~127 µs/iter for 4096-byte ASCII UTF-8 decode (measured under Asan)

## Branch Discipline

This step was done on `worktree-pluggable-codec-protocol`. After merging to main, the next step
should follow the same pattern (work in this worktree, update from main first).

## Notes

- Benchmark executable path: `.build/build-system/benchmark/$(CONFIG)/beman.transcode.benchmarks.smoke`
- The `[smoke]` tag filter in `make bench` means only tests tagged `[smoke]` run; add this tag to any benchmark TEST_CASE you want included in the smoke run
- Catch2 benchmark macros are in `<catch2/catch_all.hpp>` (already available via FetchContent)
- Asan config is the default (`CONFIG ?= Asan` in Makefile); Release/RelWithDebInfo give more realistic perf numbers
