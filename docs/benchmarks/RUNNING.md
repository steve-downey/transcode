# Running Benchmarks

This document describes how to run the benchmarks for `beman.transcode`.

## The Benchmark Run

```bash
make bench
```

One target runs everything: every suite of this library, and the same
conversions in every baseline library, at the configuration that gives this
library its best numbers.  It writes a report at the end.

- **Configuration.**  GCC-16 `Perf`: `-O3 -march=native -mtune=native -flto`,
  no sanitizers, `NDEBUG`.  It is built in its own tree, `.build/build-perf/`,
  so the `CONFIG` the tests use is left alone.
- **Baselines.**  iconv, `std::codecvt`/`mbstowcs`, Rust's `encoding_rs`
  (built when `cargo` is on `PATH`), and simdutf (fetched on the first
  configure).  A baseline that was not built is reported as skipped.
- **Corpora.**  The full corpora are downloaded into `data/benchmarks/` on the
  first run.  The checked-in samples in `benchmark/corpus/` are a few hundred
  bytes, too small to time well.
- **Warm-up and repeats.**  Catch2 warms each benchmark up for one second
  before sampling it.  The suites then run five times over.  Each repeat runs
  every suite before the next repeat starts, and the report combines the
  repeats (see [README.md](README.md#repeated-runs)).
- **Output.**  Each run writes a new directory,
  `data/benchmarks/results/<UTC timestamp>/`, holding the XML, `report.md`,
  `report.html`, the SVG charts and a Vega-Lite spec.
  `data/benchmarks/results/latest` points at the newest run.

These variables override the defaults, though the defaults are meant to be
left alone:

| Variable | Default |
|----------|---------|
| `BENCH_REPEATS` | `5` |
| `BENCH_WARMUP_MS` | `1000` |
| `BENCH_SETTLE_LOAD` | `2.0`, the 1-minute load the report calls busy; timing waits up to five minutes for it |
| `BENCH_SUITES` | `utf whatwg pluggable boundary iconv codecvt encoding_rs simdutf` |
| `BENCH_DIR` | `$(BENCH_RESULTS_DIR)/<UTC timestamp>` |

## Comparing Compilers and Flags

Holding the library constant and varying the toolchain is a separate axis.
`make bench-compare` reports any two result directories against each other:

```bash
make bench-compare BENCH_BASELINE=data/benchmarks/results/A \
                   BENCH_CANDIDATE=data/benchmarks/results/B
```

The matrix runner builds each compiler slice from a CMake preset and runs it:

```bash
make bench-matrix       # smoke filter per slice (fast)
make bench-matrix-full  # full [benchmark] filter per slice (slow)
```

The matrix runner (`infra/scripts/bench-matrix.sh`) will:

1. Print environment metadata (timestamp, hostname, OS, CPU, compiler versions).
2. Attempt each slice in order: GCC -O3, GCC -O3 -flto, Clang -O3 -flto.
3. **Skip** any slice whose compiler is absent, printing `SKIP: <reason>`.
4. Print results for available slices.

You can also invoke the script directly with a custom tag filter:

```bash
infra/scripts/bench-matrix.sh "[benchmark][utf]"
```

### Matrix Slices

| Slice | Preset | Compiler | Flags |
|-------|--------|----------|-------|
| GCC -O3 | `gcc-release` | `gcc` | `-O3` |
| GCC -O3 -flto | `gcc-release-lto` | `gcc` | `-O3 -flto` |
| Clang -O3 -flto | `llvm-release-lto` | `clang` | `-O3 -flto` |

LTO presets set `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`, which CMake translates
to `-flto` flags at compile and link time for all targets.  The `gcc-release`
and `gcc-release-lto` presets use single-config Ninja, so binaries land in
`build/<preset>/benchmark/`, not under a config subdirectory.

## Environment Metadata

Every XML result has a `.meta.json` sidecar that records the machine, compiler
and commit.  To print the environment without running anything:

```bash
make bench-env
```

## Running One Suite by Hand

After `make bench` has built the Perf tree, any suite can be run directly with
a Catch2 filter:

```bash
.build/build-perf/benchmark/Perf/beman.transcode.benchmarks.whatwg "[benchmark][label]"
```

## Notes

- Sanitizer builds (the default `Asan` config) inhibit optimization and are
  **not** suitable for benchmarking.
- On WSL2/Linux, CPU frequency scaling may affect results; consider pinning
  frequency with `cpupower` if precise comparisons matter.
