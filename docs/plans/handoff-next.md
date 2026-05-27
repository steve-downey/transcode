# Handoff: Next Step

## Completed

- **P3-Step 11: Toolchain matrix automation** — done on `p3-step11-toolchain-matrix` branch

## What was done

- Added `_lto-base`, `gcc-release-lto`, and `llvm-release-lto` presets to
  `CMakePresets.json` (configure, build, test, and workflow presets for each).
  LTO is enabled via `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`.
- Created `infra/scripts/bench-matrix.sh` — matrix runner that:
  - Prints environment metadata (timestamp, hostname, OS, CPU model, core count,
    compiler versions) prefixed with `# ` for easy grepping
  - Runs three slices in order: GCC -O3, GCC -O3 -flto, Clang -O3 -flto
  - Skips any slice whose compiler is absent (`SKIP: <reason>`)
  - Accepts an optional tag filter argument (defaults to `[smoke]`)
- Added Makefile targets: `bench-env`, `bench-lto`, `bench-matrix-gcc`,
  `bench-matrix-gcc-lto`, `bench-matrix-llvm-lto`, `bench-matrix`, `bench-matrix-full`
- Created `docs/benchmarks/RUNNING.md` with smoke/matrix instructions, per-slice
  targets, metadata capture pattern, and preset build-dir layout notes
- clang-format also fixed alignment in `benchmark/boundary_stress_benchmarks.bench.cpp`
  (from step 10)

## Platform results (WSL2/Linux, GCC 13, no system `clang`)

- `make bench-lto` — GCC -O3 -flto smoke: ~4 µs/iter (whatwg_decode utf_8)
- `make bench-matrix` — GCC -O3 and GCC -O3 -flto both ran; Clang slice
  printed `SKIP: clang not found` and exited cleanly

## Files created

- `infra/scripts/bench-matrix.sh`
- `docs/benchmarks/RUNNING.md`

## Files modified

- `CMakePresets.json` — `_lto-base`, `gcc-release-lto`, `llvm-release-lto` presets
- `Makefile` — new bench-matrix targets
- `benchmark/boundary_stress_benchmarks.bench.cpp` — clang-format alignment fix

## Implementation notes

- LTO presets use single-config Ninja (via `_root-config`), so binaries land in
  `build/<preset>/benchmark/` (not under a config subdirectory like the
  Multi-Config default build).
- The matrix script uses `set -euo pipefail` but each slice is wrapped in a
  function with `return 0` on failure, so one slice failing never aborts the run.
- `cmake --preset gcc-release -q` was attempted initially but cmake has no `-q`
  flag; fixed to `cmake --preset gcc-release`.
- The `gcc-release` preset configures into `build/gcc-release/` (single-config),
  distinct from `.build/build-system/` (the Multi-Config default used by `make compile`).

## Next Step

Read `docs/plans/p3-step12-reporting-and-repro.md` (if it exists) for the next step.

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Current State

- `make test` passes (671 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi, shellcheck all clean)
- `make bench` runs smoke (ASCII corpus, `[smoke]` tag)
- `make bench-utf` runs UTF benchmarks (`[benchmark][utf]` tag)
- `make bench-whatwg` runs WHATWG legacy codec benchmarks (`[benchmark][whatwg]` tag)
- `make bench-pluggable` runs pluggable codec benchmarks (`[benchmark][pluggable]` tag)
- `make bench-iconv` runs iconv baseline benchmarks (`[benchmark][iconv]` tag)
- `make bench-codecvt` runs std::codecvt negative baseline (`[benchmark][codecvt]` tag)
- `make bench-encoding-rs` runs encoding_rs baseline (`[benchmark][encoding_rs]` tag, requires cargo)
- `make bench-simdutf` runs simdutf ceiling baseline (`[benchmark][simdutf]` tag, requires `-DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON`)
- `make bench-boundary` runs boundary stress benchmarks (`[benchmark][boundary]` tag)
- `make bench-lto` configures gcc-release-lto (if needed) + runs smoke
- `make bench-matrix` runs all available matrix slices (smoke), skips missing compilers
- `make bench-matrix-full` runs all available matrix slices (full `[benchmark]` filter)
- `make bench-env` prints environment metadata

## Branch State

Steps 3–10 are merged into `main`; step 11 is on `p3-step11-toolchain-matrix`.
The user will merge `p3-step11-toolchain-matrix` to `main`.
After merging, step 12 should branch from `main`.
