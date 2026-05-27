# Running Benchmarks

This document describes how to run benchmarks for `beman.transcode`, both quick
smoke checks and the full compiler/optimization matrix.

## Quick Smoke Run (Default Build)

The default build (Asan config via `make compile`) is not suitable for
performance measurement but is useful as a smoke check that benchmarks work.

```bash
make bench           # smoke: ASCII corpus
make bench-utf       # UTF-family benchmarks
make bench-whatwg    # WHATWG legacy codec benchmarks
make bench-pluggable # pluggable codec benchmarks
make bench-iconv     # iconv baseline benchmarks
make bench-boundary  # boundary stress benchmarks
```

## Optimized Builds via CMake Presets

For meaningful performance numbers, use release presets that compile with `-O3`.
These produce separate build directories under `build/`.

### GCC -O3 (smoke)

```bash
make bench-matrix-gcc
```

Configures `gcc-release` preset (`build/gcc-release/`) if not already done, then
runs the smoke benchmark.

### GCC -O3 -flto (smoke)

```bash
make bench-lto
# or equivalently:
make bench-matrix-gcc-lto
```

Configures `gcc-release-lto` preset (`build/gcc-release-lto/`) if not already
done, then runs the smoke benchmark.

### Clang -O3 -flto (smoke)

```bash
make bench-matrix-llvm-lto
```

Configures `llvm-release-lto` preset (`build/llvm-release-lto/`) if not already
done.  **Skips cleanly** if `clang` is not found in `PATH`.

## Full Compiler Matrix

Run all supported matrix slices in sequence:

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

## Capturing Results

Redirect output to capture both metadata and results together:

```bash
make bench-matrix 2>&1 | tee results-$(date +%Y%m%d).txt
```

Metadata lines are prefixed with `# ` for easy grepping:

```bash
grep "^#" results-20260101.txt   # environment metadata only
grep -v "^#" results-20260101.txt  # benchmark data only
```

## Environment Metadata

Print just the environment metadata without running benchmarks:

```bash
make bench-env
```

Example output:

```
# TIMESTAMP: 2026-05-27T12:00:00Z
# HOSTNAME: buildhost
# OS: Linux 6.6.87 x86_64
# DISTRO: Ubuntu 24.04.1 LTS
# CPU: Intel(R) Core(TM) i9-13900K
# CPU_CORES: 32
# GCC: gcc (Ubuntu 13.3.0) 13.3.0
# CLANG: not found
```

## Manually Configuring a Preset

To configure a preset once and then build/run manually:

```bash
# Configure
uv run cmake --preset gcc-release-lto

# Build all benchmarks
cmake --build build/gcc-release-lto

# Run a specific benchmark binary
build/gcc-release-lto/benchmark/beman.transcode.benchmarks.utf "[benchmark][utf]"
```

## Matrix Slices

| Slice | Preset | Compiler | Flags |
|-------|--------|----------|-------|
| GCC -O3 | `gcc-release` | `gcc` | `-O3` |
| GCC -O3 -flto | `gcc-release-lto` | `gcc` | `-O3 -flto` |
| Clang -O3 -flto | `llvm-release-lto` | `clang` | `-O3 -flto` |

LTO presets set `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`, which CMake translates
to `-flto` flags at compile and link time for all targets.

## Notes

- Sanitizer builds (the default `Asan` config) inhibit optimization and are
  **not** suitable for benchmarking.  Always use a release preset for numbers.
- On WSL2/Linux, CPU frequency scaling may affect results; consider pinning
  frequency with `cpupower` if precise comparisons matter.
- The `gcc-release` and `gcc-release-lto` presets use single-config Ninja, so
  binaries land in `build/<preset>/benchmark/` (not under a config subdirectory
  like the Multi-Config builds).
