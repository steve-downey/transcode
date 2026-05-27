# Benchmarking Guide

This document is the entry point for the `beman.transcode` benchmark suite.  It
covers running the benchmarks, generating throughput reports, and reproducing the
published results.

See [RUNNING.md](RUNNING.md) for quick-reference make targets and
[PLATFORM_NOTES.md](PLATFORM_NOTES.md) for hardware-specific observations.

---

## Overview

The benchmark infrastructure comprises:

| Component | Purpose |
|-----------|---------|
| `benchmark/*.bench.cpp` | Catch2 benchmark sources |
| `benchmark/corpus/` | Small checked-in corpus files (smoke-scale) |
| `data/benchmarks/` | Large downloaded corpus files (full-scale) |
| `data/benchmarks/results/` | Generated XML results and reports |
| `tools/process_benchmark_results.py` | Post-processing: XML → Markdown + Vega-Lite |
| `infra/scripts/bench-matrix.sh` | Multi-toolchain matrix runner |
| `docs/benchmarks/RUNNING.md` | How to run each benchmark suite |

---

## Quick Start

```bash
# 1. Build and run smoke benchmark, then generate a Markdown report
make bench-report

# 2. Build GCC LTO benchmarks, save XML, then report with chart spec
make bench-report-lto
```

`bench-report` uses the `RelWithDebInfo` config (optimized, no sanitizers) and
writes results to `data/benchmarks/results/smoke-RelWithDebInfo.xml`.

`bench-report-lto` uses the `gcc-release-lto` CMake preset (GCC -O3 -flto).

---

## Corpus Files

### Small Corpus (checked-in)

Located in `benchmark/corpus/` — shipped with the repository for reproducible
smoke runs without network access.

| File | Language | Encoding | Size |
|------|----------|----------|------|
| `en_mars_utf8.txt` | English | UTF-8 | ~586 B |
| `ar_mars_utf8.txt` | Arabic | UTF-8 | ~640 B |
| `ru_mars_utf8.txt` | Russian | UTF-8 | ~802 B |
| `ja_mars_utf8.txt` | Japanese | UTF-8 | ~504 B |
| `ru_mars_windows1251.bin` | Russian | Windows-1251 | ~435 B |
| `ja_mars_shiftjis.bin` | Japanese | Shift-JIS | ~340 B |

Source: excerpts from the Mars Wikipedia article in each language.

### Large Corpus (downloaded)

Run once to download and write to `data/benchmarks/`:

```bash
uv run python tools/download_benchmark_corpora.py
```

This also writes `data/benchmarks/corpus_manifest.json` with file sizes and
checksums.  The reporting tool uses the manifest to compute throughput for
full-scale runs.

---

## Generating Reports

### From a Single XML File

```bash
uv run python tools/process_benchmark_results.py \
    --corpus-dir benchmark/corpus \
    data/benchmarks/results/smoke-RelWithDebInfo.xml
```

### With Large Corpus Sizes

```bash
uv run python tools/process_benchmark_results.py \
    --manifest data/benchmarks/corpus_manifest.json \
    data/benchmarks/results/smoke-RelWithDebInfo.xml
```

### With a Vega-Lite Chart

```bash
uv run python tools/process_benchmark_results.py \
    --manifest data/benchmarks/corpus_manifest.json \
    --vegalite data/benchmarks/results/smoke.vl.json \
    --label "GCC -O3 -flto" \
    data/benchmarks/results/smoke-lto.xml
```

Paste the `.vl.json` contents into <https://vega.github.io/editor/> to render
an interactive bar chart.

### Comparing Multiple Toolchains

Pass multiple XML files to the tool — it merges all results into one table:

```bash
uv run python tools/process_benchmark_results.py \
    data/benchmarks/results/smoke-gcc-release.xml \
    data/benchmarks/results/smoke-lto.xml
```

---

## Throughput Formula

```
throughput_MiBs = corpus_bytes / (mean_time_s) / 2^20
```

where `mean_time_s = mean_ns × 10⁻⁹` is the Catch2 mean per iteration.

All values in the Markdown table use 95% confidence interval bounds from
Catch2's bootstrap analysis.

---

## Reproducing Published Results

```bash
# Step 1: Download full corpus (one-time, requires network)
uv run python tools/download_benchmark_corpora.py

# Step 2: Run the full matrix
make bench-matrix

# Step 3: Generate reports for each toolchain slice
make bench-report-lto   # GCC -O3 -flto (primary results)
make bench-report        # GCC RelWithDebInfo (baseline)

# Step 4: Save environment metadata
make bench-env > data/benchmarks/results/env.txt
```

To capture all output together:

```bash
{
  make bench-env
  make bench-matrix
} 2>&1 | tee data/benchmarks/results/matrix-$(date +%Y%m%d).txt
```

Metadata lines in the output are prefixed with `# ` and can be grepped:

```bash
grep "^#" data/benchmarks/results/matrix-20260527.txt
```

---

## Benchmark Suites

| Make Target | Benchmark Binary | Tag Filter |
|------------|-----------------|------------|
| `make bench` | `benchmarks.smoke` | `[smoke]` |
| `make bench-utf` | `benchmarks.utf` | `[benchmark][utf]` |
| `make bench-whatwg` | `benchmarks.whatwg` | `[benchmark][whatwg]` |
| `make bench-pluggable` | `benchmarks.pluggable` | `[benchmark][pluggable]` |
| `make bench-iconv` | `benchmarks.iconv` | `[benchmark][iconv]` |
| `make bench-codecvt` | `benchmarks.codecvt` | `[benchmark][codecvt]` |
| `make bench-encoding-rs` | `benchmarks.encoding_rs` | `[benchmark][encoding_rs]` |
| `make bench-simdutf` | `benchmarks.simdutf` | `[benchmark][simdutf]` |
| `make bench-boundary` | `benchmarks.boundary` | `[benchmark][boundary]` |
| `make bench-lto` | LTO smoke | `[smoke]` |
| `make bench-matrix` | all slices | `[smoke]` |
| `make bench-report` | smoke → Markdown | — |
| `make bench-report-lto` | LTO smoke → Markdown + chart | — |
