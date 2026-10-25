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
| `tools/process_benchmark_results.py` | Post-processing: XML + sidecar → Markdown, HTML, SVG, Vega-Lite |
| `infra/scripts/bench-matrix.sh` | Multi-toolchain matrix runner |
| `docs/benchmarks/RUNNING.md` | How to run each benchmark suite |

---

## Quick Start

```bash
# 1. Build and run smoke benchmark, then generate Markdown and HTML reports
make bench-report

# 2. Build GCC LTO benchmarks, save XML, then report with chart spec
make bench-report-lto

# 3. Build GCC-16 Perf, run the core suites, report with charts and SVGs
make bench-perf-report
```

`bench-report` uses the `RelWithDebInfo` config (optimized, no sanitizers) and
writes `data/benchmarks/results/smoke-RelWithDebInfo.{xml,meta.json,md,html}`.
Open the `.html` file in a browser.

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
checksums.

---

## Generating Reports

Run a benchmark binary with `--reporter xml --out FILE.xml`.  Besides the XML,
the benchmark fixture writes `FILE.meta.json`, a sidecar that records, for
every benchmark:

- what it converts, which library does the converting, and through which API;
- which input it read, whether that came from the downloaded corpus, the
  checked-in corpus, or was synthetic, and how many bytes it held;

and, once per run:

- the compiler, build configuration, commit (`git describe --always --dirty`,
  taken at build time) and sanitizers;
- the host, CPU model, logical CPU count, kernel version, CPU frequency
  governor, and whether turbo/boost is enabled;
- the load averages from `/proc/loadavg` as the run starts and as it ends.

Set `BEMAN_TRANSCODE_BENCH_META=PATH` to write the sidecar somewhere else.

The reporting tool reads each XML file together with its sidecar:

```bash
uv run python tools/process_benchmark_results.py \
    --label "GCC-16 Perf" \
    --markdown data/benchmarks/results/perf.md \
    --html data/benchmarks/results/perf.html \
    --svg-dir data/benchmarks/results/perf-svg \
    data/benchmarks/results/*-Perf.xml
```

| Option | Output |
|--------|--------|
| *(none)* | Markdown report on stdout |
| `--markdown PATH` | Markdown report, one table per conversion |
| `--html PATH` | Self-contained HTML report: environment, warnings, one chart per conversion, the full results table |
| `--svg-dir DIR` | One standalone SVG chart per conversion, named for it (`shift-jis-to-utf-8.svg`), for figures in papers |
| `--vegalite PATH` | Vega-Lite spec; paste into <https://vega.github.io/editor/> |
| `--run LABEL PATH...` | A labelled run, for comparing runs; see [Comparing runs](#comparing-runs). With more than one run, `--svg-dir` writes one subdirectory per run, and one ratio chart subdirectory, `LATER-vs-FIRST`, per later run |
| `--reference-library NAME` | The library the others are compared with in a single-run report; see [Ratios](#ratios). Default `beman.transcode` |

`make bench-perf-report` writes all four into `data/benchmarks/results/`.

### Reading the charts

Each chart shows one conversion.  Rows are grouped under the input they read,
smallest first; within an input, each row is one benchmark.  The mark is the
mean throughput and the whisker is Catch2's 95% confidence interval on it.  The
axis is logarithmic, because throughputs across APIs span two orders of
magnitude.  Colour and marker shape both identify the library: beman.transcode
(circle), iconv (square), encoding_rs (diamond), simdutf (triangle), the C/C++
standard library (inverted triangle).

### Flags

A result is flagged instead of trusted when:

| Flag | Meaning | Plotted? |
|------|---------|----------|
| `no-metadata` | no sidecar entry, so the input size is unknown | no |
| `empty-input` | the input was empty, usually a missing corpus file | no |
| `implausible` | under 1 ns per call, or over 32 bytes/ns (~30 GiB/s): the work was almost certainly optimized away | no |
| `few-instructions` | counted at under 0.01 instructions per input byte: the work was optimized away | no |
| `noisy` | Catch2's outlier variance is above 0.5 ("severely inflated") | yes |

An unplotted result keeps its row in the chart, with the reason where the value
would be.  The report also warns when a suite was built with a sanitizer or
without optimization, or when suites were built with different compilers.

It also warns when a suite ran on a busy machine.  The *other load* is the
1-minute load average from work other than the benchmark.  It is the larger of
the load as the suite started and the load as it ended less one, the
benchmark's own thread.  A suite is busy when its other load is above 2.0.  The
benchmarks are single-threaded, but other work still shares the CPU's power
budget and cache.  A busy suite is reported, not dropped: the warning stops a
noisy run being read as a quiet one.

Passing several XML files merges them into one report.  Several files of the
same suite are [repeats](#repeated-runs) of it.

### Repeated runs

One run on a busy machine can be unlucky.  `make bench-repeat` builds the
GCC-16 Perf configuration and runs the core suites `BENCH_REPEATS` times
(default 5).  It writes `SUITE.N.xml` and `SUITE.N.meta.json` into
`BENCH_REPEAT_DIR` (default `data/benchmarks/results/repeats/`), then reports
on that directory.  Each repeat runs every suite before the next repeat
starts, so the libraries in one repeat see much the same machine state.  The
target refuses to write into a directory that already holds results.

| Variable | Default |
|----------|---------|
| `BENCH_REPEATS` | `5` |
| `BENCH_REPEAT_SUITES` | `smoke utf whatwg pluggable iconv boundary` |
| `BENCH_REPEAT_DIR` | `$(BENCH_RESULTS_DIR)/repeats` |

The report treats several files of one suite in one run as repeats, ordered by
when they ran.  Each benchmark becomes one result:

- its value is the median of the repeats' means, and the fastest repeat is
  shown beside it;
- its interval is the range across repeats, from fastest to slowest, in place
  of Catch2's CI, and the charts' whiskers show that range;
- a benchmark missing from some repeats is noted, as in `3 of 5 repeats`.

Libraries are compared within each repeat, then the ratios are combined: the
value is their median and the interval their range.  Load on the machine slows
every library in a repeat together, so a ratio taken within a repeat holds
still while the times move.  Two runs are compared on their ranges, so a
change is reported only when the ranges of the two runs don't overlap.

`make bench-compare` takes repeat directories as they are.

### Ratios

A ratio is one throughput as a multiple of another, measured on the same input.
Above 1× is faster.  Its interval runs between the far ends of the two 95% CIs,
so it is wider than either.  The report calls a difference real only when the
interval excludes 1×: "2.50× faster" or "1.43× slower".  Otherwise it says *no
clear change* (between runs) or *no clear difference* (between libraries),
rather than showing a small win or loss.

In a single-run report, a conversion run by more than one library gets a `vs
beman.transcode` column.  The reference library's fastest result on each input
reads `reference`, and every other result on that input is a multiple of it.
Choose another reference with `--reference-library`.  Results on an input the
reference library didn't run show `—`.  The Vega-Lite spec carries the same
number as `vs_reference`.

### Comparing runs

A *run* is the set of suites from one benchmarking session: a directory of XML
files and sidecars, or a list of XML files.  Give each run with `--run LABEL
PATH...`, the first being the reference:

```bash
uv run python tools/process_benchmark_results.py \
    --run gcc-16 results-gcc/ \
    --run clang-22 results-clang/ \
    --html compare.html --markdown compare.md
```

or, for two directories, `make bench-compare BENCH_BASELINE=DIR
BENCH_CANDIDATE=DIR`, which writes `compare.{md,html,vl.json}` into
`data/benchmarks/results/`.  Positional XML files, if any, form one more run,
first in order.  An empty label (`--run "" DIR`) is filled in from whatever
tells the runs apart, such as the compiler or the commit.

Results are matched across runs by suite and benchmark name.  The report
has a table describing each run, then one table per conversion with a
throughput column per run.  After each run but the first comes its
[ratio](#ratios) to the first.  The HTML report, the `--svg-dir` charts and the
Vega-Lite spec also draw each ratio as a dot plot, on a log axis centred on 1×.  A benchmark missing from a run shows `not run`
there, and every such benchmark is also listed under "Not in every run".  Each
row's notes give the flags of each run's result, and say `input differs` when
the runs read different inputs (a corpus downloaded for one run only, say),
because their throughputs then measure different work.

The run table shows each run's highest other load, marked `busy` above the
threshold.  The report warns when the runs differ in more than one of compiler,
build config, commit, sanitizers, host, CPU, kernel, governor and turbo, since a difference between them then
can't be put down to any one of these; when the suites within one run differ in
any of them; and when a run was built from a modified tree.  Comparison reports
never fail on a slowdown: the tool exits 0 whatever the numbers say.

### Instruction counts

Wall time moves with the machine's load; an instruction count barely does.
`make bench-instructions` builds the GCC-16 Perf configuration and runs each
core suite once under Valgrind's callgrind, with the fixture in *count mode*.
It writes `SUITE.xml`, its sidecar, and callgrind dumps
`SUITE.callgrind.PID.N` into `BENCH_INSTRUCTION_DIR` (default
`data/benchmarks/results/instructions/`), then reports on that directory.
Like `bench-repeat`, it refuses a directory that already holds results.

| Variable | Default |
|----------|---------|
| `BENCH_COUNT_RUNS` | `3` |
| `BENCH_INSTRUCTION_SUITES` | `$(BENCH_REPEAT_SUITES)` |
| `BENCH_INSTRUCTION_DIR` | `$(BENCH_RESULTS_DIR)/instructions` |

Count mode is on when `BEMAN_TRANSCODE_BENCH_COUNT=N` is set.  The first time
each benchmark's body reaches `bench::measure`, it runs once with nothing
counted, to warm up lazy setup such as `iconv_open`; then `N` times with
callgrind collecting; then callgrind dumps the counts under the benchmark's
name.  After that the body is timed as usual, so Catch2 can finish, but with
the minimal settings the target passes, and the times mean nothing.  The
sidecar records `count_runs`, which marks the suite as counted.  Outside
Valgrind, count mode runs the extra calls and counts nothing.

The report divides each count by `N` and by the input size, giving
instructions per input byte, lower being better.  A counted suite gives counts
only.  Passed in one run beside timed suites, each count joins the timed
result with the same suite and benchmark name:

```bash
uv run python tools/process_benchmark_results.py \
    data/benchmarks/results/repeats data/benchmarks/results/instructions \
    --html report.html
```

Each table gains an `Instr/B` column, and a conversion run by several libraries
gets `instr vs beman.transcode` too.  The HTML report and `--svg-dir` draw an
instructions chart beside each throughput chart (`CONVERSION.instructions.svg`).
Comparing runs adds each run's count, and each later run's count ratio to the
first, with a ratio chart of its own.  A count ratio is above 1× when there are
fewer instructions, as a throughput ratio is when faster.  Counts barely vary
between runs, so a count ratio is a change once it is more than 1% from 1×.
The Vega-Lite spec carries `instructions_per_byte`, `instructions_vs_reference`
and `instructions_ratio`.  A report of counts alone leaves out the time
columns.

A count is not a time.  Cache misses, branch mispredictions and memory stalls
cost no instructions, and SIMD code does more work in each one, so the count
flatters simdutf.  It finds algorithmic costs instead: a lookup that scans a
table shows up directly as instructions per byte.  The dumps open in
`kcachegrind`, which shows which functions spend them.  A count under 0.01 per
byte is flagged `few-instructions`, checking `implausible` from the other
side.

### Describing a benchmark

Put a `describe` call on the line before each `BENCHMARK_ADVANCED`, and time
its body with `bench::measure(meter, fn)` (`benchmark/benchmark_measure.hpp`)
rather than `meter.measure(fn)`, so that count mode can count it.  Benchmarks
with the same conversion and input are drawn together, so name conversions
consistently (`Shift_JIS → UTF-8`):

```cpp
describe({.conversion = "Shift_JIS → UTF-8", .library = "beman.transcode", .api = "transcode view"});
BENCHMARK_ADVANCED("Shift-JIS -> UTF-8 streaming (transcode pipe)")
(Catch::Benchmark::Chronometer meter) {
    measure(meter, [&] {
        return count_elements(corpus_span("ja_mars_shiftjis.bin") | transcode<codec::shift_jis, codec::utf_8>);
    });
};
```

A plain `BENCHMARK` is timed by Catch2 directly, and count mode never sees it.

`corpus_span` and `load_corpus` record their input.  A benchmark over generated
data calls `record_input(label, bytes)` from its body instead.  A test case
whose benchmarks need the downloaded corpus starts with
`require_corpus("file")`, which skips the test case when the file is absent, so
a missing file is reported rather than timed as an empty input.

To time one stage of a conversion, such as an encode on its own, prepare its
input in the test case body, before the `BENCHMARK`, where no benchmark is
running and nothing is recorded or timed.  Then call `record_input` in the
benchmark with a label that says where the input came from, such as
`"genji_monogatari_shiftjis.bin as UTF-32"`.  Benchmarks in different libraries
that encode the same input must record the same label and size, so the report
draws them together.

---

## Throughput Formula

```
throughput_MiBs = input_bytes / (mean_time_s) / 2^20
```

where `mean_time_s = mean_ns × 10⁻⁹` is the Catch2 mean per iteration, and
`input_bytes` is the size the fixture recorded for the benchmark's input.  The
confidence interval on throughput comes from the bounds of Catch2's 95%
bootstrap interval on the mean.

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
| `make bench-report` | smoke → Markdown + HTML | — |
| `make bench-report-lto` | LTO smoke → Markdown + HTML + Vega-Lite | — |
| `make bench-perf-report` | core suites, GCC-16 Perf → Markdown + HTML + SVG + Vega-Lite | — |
| `make bench-compare` | two result directories → comparison Markdown + HTML + Vega-Lite | — |
| `make bench-repeat` | core suites, GCC-16 Perf, `BENCH_REPEATS` times → Markdown + HTML + SVG + Vega-Lite | — |
| `make bench-instructions` | core suites, GCC-16 Perf, under callgrind → instruction counts, Markdown + HTML + SVG + Vega-Lite | — |
