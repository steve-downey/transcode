# Handoff: Next Step

## Completed

- **P3-Step 10: Boundary stress benchmarks** — done on `p3-step10-boundary-stress` branch

## What was done

- Created `benchmark/boundary_stress_benchmarks.bench.cpp` — three Catch2
  `BENCHMARK_ADVANCED` test cases tagged `[benchmark][boundary]`:
  - `Chunked whatwg decode: restart overhead` (3 sub-benchmarks):
    - Full Arabic corpus (non-chunked baseline)
    - 4-byte chunks: friendly (aligns on 2-byte Arabic sequences, no splits)
    - 3-byte chunks: hostile (splits sequences at boundaries, triggers error recovery)
  - `Mock-iconv EINVAL stress: pairwise staging` `[smoke]` — `mock_iconv_pairwise`
    with 64-byte corpus; ~32 EINVAL staging events per iteration
  - `Mock-iconv E2BIG stress: buffer exhaustion recovery` `[smoke]` — `mock_iconv_e2big`
    with 48-byte corpus; 48 E2BIG recovery loops per iteration (1 byte per iconv call)
- Modified `benchmark/CMakeLists.txt` — added `beman.transcode.benchmarks.boundary`
  executable; links `beman::transcode`, `benchmark_fixture`, `Catch2`, `Iconv`;
  adds `PROJECT_SOURCE_DIR` as include path so `<tests/beman/transcode/iconv_mock.hpp>`
  resolves without a separate include-dir variable.
- Modified `Makefile` — added `make bench-boundary` target.
- clang-format also reformatted `benchmark/simdutf_benchmarks.bench.cpp` (alignment
  only; not a logic change).

671 C++ + 189 Python tests pass; mypy + ruff + clang-format + gersemi all clean.

## Platform results (WSL2/Linux, Asan build — indicative only)

EINVAL stress (pairwise, 64 bytes): ~4.8 µs / iteration
E2BIG stress (1-byte/call, 48 bytes): ~5.2 µs / iteration
Chunked Arabic benchmarks ran; overhead vs full-corpus baseline is visible.

## Files created

- `benchmark/boundary_stress_benchmarks.bench.cpp`

## Files modified

- `benchmark/CMakeLists.txt` — boundary benchmark executable block
- `Makefile` — `bench-boundary` target
- `benchmark/simdutf_benchmarks.bench.cpp` — clang-format alignment fix

## Implementation notes

- The benchmark includes `<tests/beman/transcode/iconv_mock.hpp>` via the
  `PROJECT_SOURCE_DIR` include path (same mechanism tests use, already PUBLIC
  on `benchmark_fixture`; the boundary executable adds an explicit private
  include for that path since it is not a direct dependent of `benchmark_fixture`
  for this purpose).
- `mock_out_buf` is an `alignas(64) std::array<char,256>` in an anonymous
  namespace; safe because Catch2 benchmarks are single-threaded.
- The smoke filter (`[smoke]` tag on EINVAL and E2BIG cases) lets the boundary
  binary double as a quick sanity check:
  `beman.transcode.benchmarks.boundary "[smoke]"` runs only those 2 cases.
- No new external dependency; iconv is already required for `benchmarks.iconv`.

## Next Step

Read `docs/plans/p3-step11-toolchain-matrix.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Key context for step 11 (toolchain matrix)

- Step 11 automates the compiler/optimization matrix (GCC -O3, GCC -O3 -flto,
  Clang -O3 -flto) so benchmark runs are reproducible from `make` targets.
- The project already has CMakePresets.json with `gcc-debug`, `gcc-release`,
  `llvm-debug`, `llvm-release` presets. Toolchain files are in
  `infra/cmake/gnu-toolchain.cmake` and `infra/cmake/llvm-toolchain.cmake`.
- Step 11 should add a `gcc-release-lto` preset (inheriting gcc-release, adding
  `-flto`) and a `make bench-lto` target.
- Also creates `docs/benchmarks/RUNNING.md` with instructions for smoke vs
  full-matrix runs, and captures environment metadata (compiler version, CPU, OS,
  build flags) alongside results.
- Matrix slices that are unsupported on a given machine should skip cleanly and
  visibly (not error out).

## Current State

- `make test` passes (671 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi all clean)
- `make bench` runs smoke (ASCII corpus, `[smoke]` tag)
- `make bench-utf` runs UTF benchmarks (`[benchmark][utf]` tag)
- `make bench-whatwg` runs WHATWG legacy codec benchmarks (`[benchmark][whatwg]` tag)
- `make bench-pluggable` runs pluggable codec benchmarks (`[benchmark][pluggable]` tag)
- `make bench-iconv` runs iconv baseline benchmarks (`[benchmark][iconv]` tag)
- `make bench-codecvt` runs std::codecvt negative baseline (`[benchmark][codecvt]` tag)
- `make bench-encoding-rs` runs encoding_rs baseline (`[benchmark][encoding_rs]` tag, requires cargo)
- `make bench-simdutf` runs simdutf ceiling baseline (`[benchmark][simdutf]` tag, requires `-DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON`)
- `make bench-boundary` runs boundary stress benchmarks (`[benchmark][boundary]` tag)

## Branch State

Steps 3–10 are on top of each other and merged into `main`.
The user will merge `p3-step10-boundary-stress` to `main`.
After merging, step 11 should branch from `main`.
