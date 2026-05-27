# Handoff: Next Step

## Completed

- **P3-Step 9: `simdutf` ceiling baseline** — done on `p3-step9-simdutf-baseline` branch

## What was done

- Created `benchmark/simdutf_benchmarks.bench.cpp` — six Catch2
  `BENCHMARK_ADVANCED` cases tagged `[benchmark][simdutf]`:
  - `simdutf UTF-8→UTF-32: English` — aligned with Step 4 UTF-8 decode English
  - `simdutf UTF-8→UTF-32: Arabic` — aligned with Step 4 UTF-8 decode Arabic
  - `simdutf UTF-8 validate: English` — pure validation (fastest simdutf path)
  - `simdutf UTF-8 validate: Arabic` — multibyte validation
  - `simdutf UTF-8→UTF-16: English` — common runtime output format
  - `simdutf UTF-8→UTF-16: Arabic` — common runtime output format
- Modified `benchmark/CMakeLists.txt` — added optional `BEMAN_TRANSCODE_BENCHMARK_SIMDUTF`
  CMake option (default OFF); uses `FetchContent_Declare` for simdutf v5.6.4 from GitHub
  when enabled.
- Added `make bench-simdutf` Makefile target.
- Updated `docs/benchmarks/PLATFORM_NOTES.md` with a `simdutf` section.
- clang-format also reformatted pre-existing `encoding_rs_benchmarks.bench.cpp`
  and `encoding_rs_bench.h` (alignment changes only; lint was already passing for
  them, format just changed with the new clang-format run environment).

671 C++ + 189 Python tests pass; mypy + ruff + clang-format + gersemi all clean.

## Platform result (this machine: WSL2/Linux, AVX2 available)

All six benchmarks ran (Asan build — indicative only):

- English UTF-8→UTF-32: ~394 ns (corpus ~435 bytes)
- Arabic UTF-8→UTF-32: ~6 µs (corpus ~340 bytes) — multibyte decode overhead
- English UTF-8 validate: ~368 ns
- Arabic UTF-8 validate: ~4 µs
- English UTF-8→UTF-16: ~331 ns
- Arabic UTF-8→UTF-16: ~5.7 µs

Compare with Step 4 `whatwg_decode<utf_8>` on the same corpora — the gap
quantifies the mechanical-sympathy cost of standard C++ range composition
vs. hand-tuned SIMD intrinsics.

## Files created

- `benchmark/simdutf_benchmarks.bench.cpp`

## Files modified

- `benchmark/CMakeLists.txt` — optional simdutf FetchContent block
- `Makefile` — added `bench-simdutf` target
- `docs/benchmarks/PLATFORM_NOTES.md` — added simdutf section
- `benchmark/encoding_rs_benchmarks.bench.cpp` — clang-format alignment fix
- `benchmark/encoding_rs_bench/include/encoding_rs_bench/encoding_rs_bench.h` — clang-format alignment fix

## Implementation notes

- simdutf is fetched via `FetchContent_Declare` with `GIT_TAG v5.6.4`; the
  `SIMDUTF_BUILD_TESTS` and `SIMDUTF_BUILD_BENCHMARKS` cache vars are set to
  `OFF FORCE` to skip simdutf's own internal build targets.
- The option defaults to OFF so `make test` never triggers a network fetch.
- To enable: `cd .build/build-system && uv run cmake -DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON .`
  then `make bench-simdutf` or `make compile` + run manually.
- simdutf exposes the `simdutf` CMake target (not `simdutf::simdutf`) after
  FetchContent.
- simdutf's CMake emits a CMP0175 dev warning (add_custom_command in tests);
  this is harmless and internal to simdutf when tests are excluded.

## Next Step

Read `docs/plans/p3-step10-boundary-stress.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Key context for step 10 (boundary stress)

- Step 10 adds chunked/lazy boundary stress benchmarks for `beman::transcode`
  and deterministic mock-iconv `EINVAL`/`E2BIG` stress kernels.
- The iconv mock library is at `tests/beman/transcode/iconv_mock.hpp`.
- Synthetic failure/error corpora should go into `benchmark/corpus/`.
- Step 10 does not add a new external dependency.

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

## Branch State

Steps 3–9 are on top of each other and merged into `main`.
The user will merge `p3-step9-simdutf-baseline` to `main`.
After merging, step 10 should branch from `main`.
