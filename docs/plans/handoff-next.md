# Handoff: Next Step

## Completed

- **P3-Step 6: iconv baselines** — done on `p3-step6-iconv-baselines` branch

## What was done

- Created `benchmark/iconv_benchmarks.bench.cpp` — four benchmark cases:
  - `Raw iconv: UTF-8 to UTF-32LE English` — `en_mars_utf8.txt`, handle pre-opened, reset per iteration
  - `iconv_transcode_view: UTF-8 to UTF-32LE English` — same corpus, view opened per iteration
  - `Raw iconv: Shift-JIS to UTF-8 Japanese` — `ja_mars_shiftjis.bin`, handle pre-opened
  - `iconv_transcode_view: Shift-JIS to UTF-8 Japanese` — same corpus via view
- Registered `beman.transcode.benchmarks.iconv` in `benchmark/CMakeLists.txt`, linked `Iconv::Iconv`
- Added `bench-iconv` Makefile target

671 C++ + 189 Python tests pass; mypy + ruff + clang-format + gersemi all clean.

## Files created

- `benchmark/iconv_benchmarks.bench.cpp`

## Files modified

- `benchmark/CMakeLists.txt` — added iconv benchmark executable and `find_package(Iconv REQUIRED)`
- `Makefile` — added `bench-iconv` target

## Next Step

Read `docs/plans/p3-step7-codecvt-baseline.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Benchmark Results (Asan config — indicative only, high overhead from sanitizer)

Observed on fallback corpus (tiny files, Asan build):

**Raw iconv (handle pre-opened, reset per iteration):**
- UTF-8 to UTF-32LE English (~435-byte corpus): ~696 ns
- Shift-JIS to UTF-8 Japanese (~340-byte corpus): ~784 ns

**iconv_transcode_view (handle opened/closed each iteration via begin()):**
- UTF-8 to UTF-32LE English: ~51 µs
- Shift-JIS to UTF-8 Japanese: ~21 µs

The large gap between raw and view is primarily:
1. Handle open/close per iteration in the view (raw reuses the handle)
2. Asan overhead magnifies absolute times
3. Small corpus sizes amplify fixed costs

For meaningful throughput comparisons, build in Release config and use
the full downloaded corpus (`uv run python tools/download_benchmark_corpora.py`).

The key structural insight: the view's `begin()` calls `iconv_open()` every
time, while raw iconv can reuse a single handle across many calls.  Any
optimization of the view would need to either cache the handle or separate
open from traversal.

## Current State

- `make test` passes (671 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi all clean)
- `make bench` runs smoke (ASCII corpus, `[smoke]` tag)
- `make bench-utf` runs UTF benchmarks (`[benchmark][utf]` tag)
- `make bench-whatwg` runs WHATWG legacy codec benchmarks (`[benchmark][whatwg]` tag)
- `make bench-pluggable` runs pluggable codec benchmarks (`[benchmark][pluggable]` tag)
- `make bench-iconv` runs iconv baseline benchmarks (`[benchmark][iconv]` tag)

## Branch State

Steps 3–6 are on top of `worktree-pluggable-codec-protocol` (steps 3–5) and
`p3-step6-iconv-baselines` (step 6).  The user will merge to `main`.
After merging, the next step should branch from `main`.
