# Handoff: Next Step

## Completed

- **P3-Step 8: `encoding_rs` baseline** — done on `p3-step8-encoding-rs-baseline` branch

## What was done

- Created `benchmark/encoding_rs_bench/` — a Rust crate (staticlib) wrapping
  encoding_rs via three plain `extern "C"` functions:
  - `enc_rs_utf8_to_utf8` — WHATWG UTF-8 decode (BOM strip + U+FFFD substitution)
  - `enc_rs_windows1251_to_utf8` — Windows-1251 → UTF-8
  - `enc_rs_shift_jis_to_utf8` — Shift-JIS → UTF-8
- Created `benchmark/encoding_rs_bench/include/encoding_rs_bench/encoding_rs_bench.h`
  — C header for the shim.
- Created `benchmark/encoding_rs_benchmarks.bench.cpp` — four Catch2
  `BENCHMARK_ADVANCED` cases tagged `[benchmark][encoding_rs]`.
- Modified `benchmark/CMakeLists.txt` — `find_program(CARGO_EXECUTABLE cargo)`;
  if found, builds the crate via `add_custom_command` (target-dir into CMake
  binary dir) and links `beman.transcode.benchmarks.encoding_rs`.  If not
  found, silently skipped.
- Added `make bench-encoding-rs` Makefile target.
- Updated `docs/benchmarks/PLATFORM_NOTES.md` with an `encoding_rs` section.

671 C++ + 189 Python tests pass; mypy + ruff + clang-format + gersemi all clean.

## Platform result (this machine: WSL2/Linux, cargo 1.94.0)

All four benchmarks ran (Asan build — indicative only):

- English ASCII UTF-8→UTF-8: ~75 ns (corpus ~435 bytes) — near-zero work
- Japanese CJK UTF-8→UTF-8: ~350 ns (corpus ~340 bytes)
- Russian Windows-1251→UTF-8: ~810 ns (corpus ~340 bytes)
- Japanese Shift-JIS→UTF-8: ~693 ns (corpus ~340 bytes)

## Files created

- `benchmark/encoding_rs_bench/Cargo.toml`
- `benchmark/encoding_rs_bench/Cargo.lock`
- `benchmark/encoding_rs_bench/src/lib.rs`
- `benchmark/encoding_rs_bench/include/encoding_rs_bench/encoding_rs_bench.h`
- `benchmark/encoding_rs_benchmarks.bench.cpp`

## Files modified

- `benchmark/CMakeLists.txt` — optional Rust build and benchmark target
- `Makefile` — added `bench-encoding-rs` target
- `docs/benchmarks/PLATFORM_NOTES.md` — added encoding_rs section

## Implementation notes

- The Rust crate uses `panic = "abort"` (in both release and dev profiles) so
  no unwinding can cross the FFI boundary.
- `--target-dir` redirects cargo output to the CMake binary directory so the
  source tree stays clean.  The `Cargo.lock` lives in the source tree and is
  committed for reproducibility.
- `target/` is already covered by the global `.gitignore` `target/` pattern.
- On Linux, Rust static libs require `-ldl -lpthread`; CMake passes
  `${CMAKE_DL_LIBS}` and `Threads::Threads` accordingly.

## Next Step

Read `docs/plans/p3-step9-simdutf-baseline.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Key context for step 9 (simdutf)

- simdutf is a C++ library with a straightforward `find_package` or
  FetchContent path (no Rust involved).
- The step plan says to guard with a CMake option
  `BEMAN_TRANSCODE_BENCHMARK_SIMDUTF` and `#ifdef` in source.
- Benchmark workloads should align with Step 4 UTF cases for fair comparison.
- The step plan mentions `simdutf::convert_utf8_to_utf32` and
  `simdutf::validate_utf8` as primary entry points.

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

## Branch State

Steps 3–8 are on top of each other and merged into `main`.
The user will merge `p3-step8-encoding-rs-baseline` to `main`.
After merging, step 9 should branch from `main`.
