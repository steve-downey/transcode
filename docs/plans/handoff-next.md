# Handoff: Next Step

## Completed

- **P3-Step 7: `std::codecvt` negative baseline** — done on `p3-step7-codecvt-baseline` branch

## What was done

- Created `benchmark/codecvt_benchmarks.bench.cpp` — two benchmark cases:
  - `codecvt UTF-8→char32_t: English (ASCII-heavy)` — English Mars corpus
  - `codecvt UTF-8→char32_t: Japanese (CJK-heavy)` — Japanese Mars corpus
- Both cases guarded by `#if __has_include(<codecvt>)`.  Where absent,
  a `TEST_CASE` calls `SKIP()` with an explanatory message.
- Deprecation warnings suppressed inside the `#if` block via per-compiler
  diagnostic pragmas (`#pragma GCC diagnostic` / `#pragma clang diagnostic`).
- Registered `beman.transcode.benchmarks.codecvt` in `benchmark/CMakeLists.txt`
  (no extra `find_package` needed — standard library feature).
- Added `bench-codecvt` Makefile target.
- Added `docs/benchmarks/PLATFORM_NOTES.md` documenting per-platform availability.
- clang-format also reformatted two lines in `benchmark/iconv_benchmarks.bench.cpp`
  that were slightly over the line limit (included in the commit).

671 C++ + 189 Python tests pass; mypy + ruff + clang-format + gersemi all clean.

## Platform result (this machine: libstdc++/GCC)

`<codecvt>` is **present**.  Both benchmarks ran.  Observed timings (Asan
build — indicative only):

- English (ASCII-heavy corpus ~435 bytes): ~11.8 µs — dominated by
  `std::u32string` heap allocation per call.
- Japanese (CJK-heavy corpus ~340 bytes): ~4.0 µs.

Contrast with `whatwg_decode<utf_8>` on the same corpora: ≪ 1 µs (no
allocation).  This confirms the "negative baseline" characterization.

## Files created

- `benchmark/codecvt_benchmarks.bench.cpp`
- `docs/benchmarks/PLATFORM_NOTES.md`

## Files modified

- `benchmark/CMakeLists.txt` — added codecvt benchmark executable
- `Makefile` — added `bench-codecvt` target
- `benchmark/iconv_benchmarks.bench.cpp` — clang-format reformatting only

## Next Step

Read `docs/plans/p3-step8-encoding-rs-baseline.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Key context for step 8 (encoding_rs)

Step 8 integrates encoding_rs as an optional baseline.  The architecture
document at `docs/Rust Encoding for C++ Transcode.md` covers the FFI
strategy in depth.  Key points:

- encoding_rs is a Rust crate with a C FFI layer (`encoding_c`) and a
  C++ header (`encoding_rs_cpp.h`).
- The step should keep the Rust dependency **completely optional** —
  don't touch the main CMakeLists or library build surface.
- encoding_rs has a non-streaming mode (contiguous buffers, zero allocation)
  and a streaming mode (stateful Decoder/Encoder).  For benchmarking, the
  non-streaming path on contiguous corpus spans is the right starting point.
- The corpora (`en_mars_utf8.txt`, `ja_mars_utf8.txt`, `ru_mars_windows1251.bin`)
  are loaded via `corpus_span()` from `benchmark_fixture.hpp`, giving
  contiguous `std::span<const char>` inputs — ideal for encoding_rs's
  non-streaming mode.
- The step plan says to align with "existing oracle direction" — that means
  the approach described in `docs/Rust Encoding for C++ Transcode.md` and
  any in-repo tooling under `tools/` or `infra/`.

## Current State

- `make test` passes (671 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi all clean)
- `make bench` runs smoke (ASCII corpus, `[smoke]` tag)
- `make bench-utf` runs UTF benchmarks (`[benchmark][utf]` tag)
- `make bench-whatwg` runs WHATWG legacy codec benchmarks (`[benchmark][whatwg]` tag)
- `make bench-pluggable` runs pluggable codec benchmarks (`[benchmark][pluggable]` tag)
- `make bench-iconv` runs iconv baseline benchmarks (`[benchmark][iconv]` tag)
- `make bench-codecvt` runs std::codecvt negative baseline (`[benchmark][codecvt]` tag)

## Branch State

Steps 3–7 are on top of `worktree-pluggable-codec-protocol` (steps 3–5),
`p3-step6-iconv-baselines` (step 6), and `p3-step7-codecvt-baseline` (step 7).
The user will merge to `main`.  After merging, the next step should branch
from `main`.
