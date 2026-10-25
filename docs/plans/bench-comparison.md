# Benchmark Comparison

**Branch:** `bench-comparison`
**Depends on:** the benchmark metadata sidecar and report from PR #109
(`bench-reports`). The stages below need its `describe()` records and its
`process_benchmark_results.py` data model. Start them after #109 lands on
`main`, then merge `main` into this branch.
**Follow-ups this serves:** #108 (EUC-JP → Shift_JIS is about 30× slower than
iconv). It also checks #107 (single-byte decode benchmarks optimized away).

---

## Goal

There are two comparisons to support. They are different questions, but both
read from one table.

1. **One library across builds.** Hold the library, API and input fixed, and
   vary the run: compiler, flags, LTO, build config, or the commit under test
   (before/after a change). The question is whether this build made it faster
   or slower.
2. **Different libraries in one build.** Hold the run fixed (same machine, same
   process, same minutes) and vary the library: beman.transcode, iconv,
   encoding_rs, simdutf, the C/C++ standard library. The question is how far
   apart the libraries are on the same work. #108 is this comparison.

PR #109 draws the second comparison within one run as absolute throughput. It
can't load more than one run, it shows no ratios, and it has only one
measure, wall time.

## Constraints

- **No performance gates on the green/red path.** No timing or instruction
  threshold in `make test`, pre-commit, `Required CI`, or any check a PR must
  pass. pwyll is a known fixed quantity only when it is quiet, and it is often
  busy building other projects. A gate would fail for reasons unrelated to the
  change. Comparison tools print a report and exit 0 whatever the numbers say.
- **Benchmark CI jobs are optional.** They run on `workflow_dispatch` or a
  schedule, are never part of `Required CI`, and upload their reports as
  artifacts.
- **Don't trust a GitHub Actions baseline until it has been measured.** Hosted
  runners are shared VMs, and the CPU model can change between jobs. The
  [runner-stability](#runner-stability) stage measures how much this matters
  before anything relies on it.
- **One PR per stage.** Each stage below is a complete piece of work.

## Model

The table has one row per measurement:

| Field | Source |
|---|---|
| run | one sidecar (`FILE.meta.json`): its environment block, plus a label |
| suite, name | Catch2 XML |
| conversion, library, api | `describe()` in the sidecar |
| input, input source, input bytes | the corpus loader or `record_input()` in the sidecar |
| wall time: mean, 95% CI, std dev, outlier variance | Catch2 XML |
| instructions (Ir) | callgrind, from [instruction-counts](#instruction-counts) |
| flags | report (`no-metadata`, `empty-input`, `implausible`, `noisy`, plus `busy`, from [machine-load](#machine-load)) |

Each comparison is a different reading of the same table:

| Comparison | Grouped by | Varies along | Ratio against |
|---|---|---|---|
| run vs run | match key (see [match-key](#match-key)) | run | the baseline run |
| library vs library | run, conversion, input | library and API | the reference library (see [reference-library](#reference-library)) |

Ratios between libraries are the robust number on a noisy machine. If beman
and iconv run in the same process minutes apart, "30× slower than iconv"
survives load on pwyll much better than "5 MiB/s" does.

## Stages

Ordinals give the reading order. Refer to a stage by its slug.

### multi-run

*Stage 1.*

Load several runs into one report.

- `process_benchmark_results.py` accepts runs as groups of XML files sharing a
  sidecar environment. Each run gets a label: `--run LABEL=FILE...`, or a
  label derived from the environment (compiler, build config, optional
  commit).
- Record the commit in the sidecar: `git describe --always --dirty`, passed in
  as a compile definition the way `BENCHMARK_BUILD_CONFIG` is.
- Measurements are matched across runs by the match key. A benchmark present
  in only some runs is listed as unmatched, not dropped silently.
- Compare runs only when their environments are comparable on the axes the
  comparison is not about. Warn when, for example, two runs differ in both
  compiler and host.

**Status:** implemented on branch `bench-multi-run`. Two changes from the
outline above:
- The option is `--run LABEL PATH...`, where each PATH is an XML file or a
  directory of them. An empty label is derived.
- The commit comes from a header generated at build time, not a configure-time
  compile definition. A configure-time value would go stale on the next
  checkout, which is exactly the before/after case.

It also adds `make bench-compare`, and notes `input differs` on a row whose
runs read different inputs.

### machine-load

*Stage 2.*

Record how busy the machine was, and flag runs made while it was busy.

- Sidecar additions:
  - `/proc/loadavg` at the start and end of the run;
  - the logical CPU count;
  - the CPU frequency governor, and whether turbo/boost is enabled;
  - the kernel version.
- The report flags a run as `busy` when its load average exceeds a fraction of
  the core count (see [busy-threshold](#busy-threshold)). Like a sanitizer
  build, the flag is a report-level warning, not an exclusion.
- This doesn't fix a noisy run. It stops a noisy run being read as a quiet
  one.

**Status:** implemented on branch `bench-machine-load`. Two changes from the
outline above:
- `busy` is a warning on the suite, not a flag on each result, and the run
  table shows each run's load. One process sees one machine state, so a
  per-result flag would repeat the same fact on every row.
- The load compared with the threshold is the *other load*. It is the larger
  of the 1-minute load at the start, and the load at the end less one for the
  benchmark's own thread.

The kernel, governor and turbo setting join the run axes, so comparing runs
that differ in them is warned about like a change of compiler.

### ratio-views

*Stage 3.*

Show the comparisons as ratios.

- **Run vs run:** one table per conversion. Rows are the matched
  benchmarks. Columns are each run's throughput and the ratio to the baseline
  run. The chart is a dot plot of the ratio on a log axis centred on 1×, with
  the CI as whiskers.
- **Library vs library:** for each conversion and input in one run, each
  library's throughput as a multiple of the reference library's.
- Treat a change as real only when its CI excludes 1×. Otherwise show it as
  "no clear change", not as a small win or loss (see
  [change-significance](#change-significance)).
- The output formats stay the same: Markdown, HTML, standalone SVG, and
  Vega-Lite.

**Status:** implemented on branch `bench-ratio-views`.
- Run vs run interleaves the ratio column after each later run's throughput,
  and draws one ratio chart per conversion for each later run.
- Library vs library is a column in the single-run report, not a separate
  chart. The throughput chart already sets the libraries side by side on a
  log axis, where the distance between marks *is* the ratio.
- A ratio is computed only when both results are plotted and read the same
  input. A row whose runs read different inputs says so in place of a ratio.

### repeated-runs

*Stage 4.*

Make one comparison less sensitive to a single unlucky run.

- A make target runs the selected suites *k* times (default 5). It writes
  `RUN-1 ... RUN-k` sidecars and XML into one directory.
- The report collapses the repeats to one value per match key (see
  [aggregate-statistic](#aggregate-statistic)), and also shows the spread
  across repeats.
- For library comparisons, compute the ratio within each repeat, then
  aggregate the ratios. Don't take the ratio of aggregated times. Each repeat
  sees the same machine state for both libraries, and this keeps that.

**Status:** implemented on branch `bench-repeated-runs`. The target is
`make bench-repeat`, writing `SUITE.N.xml` with `SUITE.N.meta.json`.
- The report needs no option for repeats. Several files of one suite in one
  run are repeats, ordered by timestamp, then by file name. This changes how
  such files are read: before this stage they were duplicates, and only the
  first was kept.
- A combined result's interval is the range across repeats. The ratio
  arithmetic from [ratio-views](#ratio-views) is unchanged, so run vs run
  calls a change only when the two runs' ranges don't overlap.
- Library ratios are paired per repeat. On real repeats, two libraries whose
  times overlap across repeats still give a ratio with a narrow interval.
- The make loop runs repeats outside and suites inside. Suites are separate
  processes, so a library's "same repeat" partner ran minutes away, not in
  the same process. That is still far closer in time than the same suite one
  repeat later.

### cjk-corpora

*Stage 5.*

Give #108 inputs large enough to measure throughput rather than setup cost.

- The novel-size inputs already exist: `tools/download_benchmark_corpora.py`
  derives `genji_monogatari_shiftjis.bin` and `genji_monogatari_eucjp.bin`
  from the downloaded Tale of Genji. Today the EUC-JP → Shift_JIS benchmark
  uses only the 94-byte `ja_mars_eucjp.bin` excerpt.
- Add benchmarks that separate the stages of the conversion:
  - Shift_JIS encode only (UTF-32 → Shift_JIS), for beman and iconv;
  - EUC-JP decode only, for beman and iconv;
  - the existing EUC-JP → Shift_JIS transcode, on the large input.
- Each new benchmark gets a `describe()` and a `require_corpus()`.
- Expected result: the encode-only benchmark should show whether Shift_JIS
  encode accounts for the 30×. The fix for #108 itself is a separate PR.

**Status:** implemented on branch `bench-cjk-corpora`.
- Two of the large-input benchmarks were already there. EUC-JP →
  Shift_JIS on Genji was measured by beman's `decode | to, encode | to` and
  by iconv, and beman's EUC-JP decode on Genji was also measured. Only the
  94-byte excerpt lacked a large twin for the `transcode view` path.
- Added on Genji:
  - Shift_JIS encode from UTF-32, for beman's `whatwg_encode` view and
    `encode_to`, and for iconv;
  - EUC-JP → UTF-32 decode for iconv;
  - EUC-JP → Shift_JIS for beman's `transcode view`.
- The encode benchmarks decode the novel's Shift_JIS file once, in the test
  case body and outside any benchmark. Both libraries encode those same
  code points, recorded as `genji_monogatari_shiftjis.bin as UTF-32`.
  iconv gets them as explicit little-endian bytes.
- Result: Shift_JIS encode is the whole of #108. It was measured as a
  RelWithDebInfo build on pwyll, with the load falling from 15 to 8, on
  1,185,133 code points:

  | Stage | beman.transcode | iconv |
  |---|---:|---:|
  | EUC-JP decode | 1.7 ms | 8.4 ms |
  | Shift_JIS encode, view | 301 ms | 9.2 ms |
  | EUC-JP → Shift_JIS, `transcode view` | 304 ms | 11.6 ms |

  beman's decode is about 5× faster than iconv's, but its encode is about
  33× slower, and the encode is nearly all of the transcode's time.
  `encode_to` and `decode | to, encode | to` take about twice as long as
  the views. The fix for #108 is still a separate PR.

### instruction-counts

*Stage 6.*

Add instruction counts from callgrind as a second measure. They come from a
separate pass over the same binary.

**Why:** an instruction count is close to deterministic. It holds still on a
busy pwyll and on a GitHub VM, and it shows algorithmic problems (a linear
search per code point) directly as instructions per byte.

**Why it doesn't replace wall time:**
- SIMD code does fewer, wider instructions, so a count overstates simdutf's
  advantage.
- Branch mispredictions, cache misses and memory stalls cost nothing in the
  count.

So it is reported alongside wall time.

**No relinking.** Valgrind instruments the binary as it runs. The pass uses an
ordinary Release or RelWithDebInfo build, never a sanitizer build, and it
instruments glibc's iconv modules and the Rust `encoding_rs` library the same
way as everything else. The only change in the source is the client-request
header, below.

**Design:**
- **Count mode.** The fixture gains a mode selected by
  `BEMAN_TRANSCODE_BENCH_COUNT=N`. Catch2 picks its iteration counts from the
  clock, and Valgrind slows everything down, so in count mode those counts
  change from run to run. Count mode runs each benchmark body a fixed *N*
  times instead.
- **A wrapper for `meter.measure`.** Add `bench::measure(meter, fn)`.
  - In normal mode it forwards to `meter.measure(fn)` and does nothing else,
    so timing is unchanged.
  - In count mode, on the first call for a benchmark only:
    1. run `fn` once, uninstrumented, to warm up lazy setup (`iconv_open`,
       static tables);
    2. `CALLGRIND_TOGGLE_COLLECT`;
    3. run `fn` *N* times, passing each result to Catch2's
       `deoptimize_value`;
    4. `CALLGRIND_TOGGLE_COLLECT`;
    5. `CALLGRIND_DUMP_STATS_AT(<benchmark name>)`.
  - It then forwards to `meter.measure` so that Catch2 can finish. Later calls
    for the same benchmark skip the instrumented block.
  - See [count-mode-driver](#count-mode-driver).
- **Converting benchmarks.** All `BENCHMARK_ADVANCED` bodies move to
  `bench::measure`. The 12 plain `BENCHMARK` bodies in
  `pluggable_codec_benchmarks.bench.cpp` become `BENCHMARK_ADVANCED`.
- **Missing Valgrind.** `<valgrind/callgrind.h>` is included only when
  `__has_include` finds it. Otherwise the macros are defined as no-ops, so
  builds without Valgrind still work. Either way, the macros run only in count
  mode.
- **Running the pass.**

  ```
  valgrind --tool=callgrind --collect-atstart=no \
      --callgrind-out-file=RUN/callgrind.%p
  ```

  run with Catch2's minimal benchmark settings (`--benchmark-samples 1
  --benchmark-resamples 1 --benchmark-warmup-time 0`). Each
  `DUMP_STATS_AT` writes a separate file whose description line carries the
  benchmark name. Its `summary:` line holds the Ir total.
- **Report.** `process_benchmark_results.py` reads the dumps, divides by *N*
  and by input bytes, and shows instructions per byte:
  - a column in every table;
  - a separate chart family (lower is better, log axis);
  - ratio views for instruction counts, the same as Stage 3 for time.
- **Make target.** `make bench-instructions` runs the pass for the core
  suites into `$(BENCH_RESULTS_DIR)/instructions/`.
- **Drill-down.** The same dump files open in kcachegrind, which shows where
  the instructions go. For #108 that answers *which function* in Shift_JIS
  encode is expensive, not just that it is.
- **#107 check.** A benchmark that is optimized away shows close to 0
  instructions per byte, confirming the `implausible` flag independently.

### runner-stability

*Stage 7.*

Measure whether a GitHub-hosted runner is a usable baseline.

- A `workflow_dispatch`-only workflow builds Release once, then runs the core
  suites in *M* identical matrix jobs (default 10). Each job also runs the
  instruction-count pass.
- Each job uploads its XML, sidecars and callgrind dumps. A final job runs the
  report over all *M* runs and uploads it.
- The report shows, per benchmark:
  - the coefficient of variation across jobs, for wall time and for
    instructions;
  - each job's CPU model, so a hardware change between jobs is visible.
- The workflow is never part of `Required CI` and has no thresholds.
- Its result answers [runner-trust](#runner-trust).

## Open questions

### match-key

**Question:** What identifies "the same benchmark" across runs?
**Status:** OPEN
**Decision:** —
**Why:** The Catch2 name is unique within a suite and survives changes to
`describe()`. The tuple (conversion, input, library, api) survives renaming a
benchmark, but not editing its description. A rename on either side breaks
matching, which is acceptable if unmatched benchmarks are listed (Stage 1).
Leaning: (suite, name).
**Decided by:** —
**Log:**
- 2026-10-24: [multi-run](#multi-run) matches on (suite, name). This is
  provisional until a decision is recorded here.

### reference-library

**Question:** Which library is the 1× baseline in library-vs-library views?
**Status:** OPEN
**Decision:** —
**Why:** iconv is present for nearly every legacy conversion and is what
users would otherwise reach for. encoding_rs is the closest design to the
WHATWG backend. simdutf is the ceiling for UTF only. Possibly beman.transcode
itself as 1×, so every other library reads as "N× ours". A command-line option
with a per-conversion default may be the answer.
**Decided by:** —
**Log:**
- 2026-10-24: [ratio-views](#ratio-views) defaults to beman.transcode, so
  every other library reads as a multiple of ours, and `--reference-library`
  overrides it. There is no per-conversion default yet. Within one input the
  1× is the reference library's *fastest* result, so the column compares best
  against best, and beman's other APIs read against its best one. This is
  provisional until a decision is recorded here.

### change-significance

**Question:** When does the report call a run-vs-run difference real?
**Status:** OPEN
**Decision:** —
**Why:** Catch2's bootstrap CI covers one run's samples, not variation between
runs. With [repeated-runs](#repeated-runs) the spread across repeats is the
better measure. The candidates are: the ratio's CI excludes 1× (one run), the
repeats don't overlap, or a fixed ±5% band. Instruction counts may need only a
small fixed tolerance.
**Decided by:** —
**Log:**
- 2026-10-24: [ratio-views](#ratio-views) uses the first candidate. The
  ratio's interval is built conservatively from the two 95% CIs (lower bound
  over upper bound, and upper over lower), and a change is reported only when
  that interval excludes 1×. It inherits the weakness above: on a busy
  machine, two runs can each have a tight CI and still differ by load. This is
  provisional until [repeated-runs](#repeated-runs) gives a spread across
  repeats.
- 2026-10-24: [repeated-runs](#repeated-runs) gives that spread. A combined
  result's interval is the range across repeats, so between runs the rule
  becomes "the repeats don't overlap". Between libraries in one run it
  becomes "the per-repeat ratios all fall on one side of 1×". The single-run
  CI rule still applies to runs without repeats.

### aggregate-statistic

**Question:** How are repeated runs collapsed into one value?
**Status:** OPEN
**Decision:** —
**Why:** Interference only ever adds time. The minimum of the repeated means
estimates the uncontended cost, while the median is more conservative. On a
busy pwyll the minimum is attractive; on a dedicated machine either works.
Show both, and plot one.
**Decided by:** —
**Log:**
- 2026-10-24: [repeated-runs](#repeated-runs) plots the median and shows the
  fastest repeat beside it in the tables. Throughput and ratios come from the
  median, and the interval is the full range. There is no option to plot the
  minimum yet. This is provisional until a decision is recorded here.

### busy-threshold

**Question:** What load average marks a run as `busy`?
**Status:** OPEN
**Decision:** —
**Why:** The benchmarks are single-threaded, so one other busy core is enough
to perturb frequency and cache. A threshold well under the core count, for
example 1.0 above an idle baseline, may be right. Calibrate it against
quiet runs on pwyll.
**Decided by:** —
**Log:**
- 2026-10-24: [machine-load](#machine-load) uses an absolute threshold, an
  other load above 2.0, rather than a fraction of the CPU count. That is
  `BUSY_LOAD` in `tools/process_benchmark_results.py`. This is provisional
  until calibrated. pwyll at rest showed a 1-minute load of 1.4 on 20 CPUs,
  and 21.5 during the `clang++-23` builds that slowed the
  [multi-run](#multi-run) trial about 7×.

### count-mode-driver

**Question:** How does count mode run each benchmark body a fixed number of
times inside Catch2?
**Status:** OPEN
**Decision:** —
**Why:** Catch2 calls a `BENCHMARK_ADVANCED` body several times (estimation,
then samples), and gives no control over iteration counts. The
`bench::measure` wrapper above instruments only the first call for each name,
and relies on minimal Catch2 settings to keep the remaining calls cheap under
Valgrind. The alternative is a separate driver that bypasses Catch2. That
duplicates every benchmark body, so it's a fallback only.
**Decided by:** —
**Log:**

### baseline-storage

**Question:** Where do the baseline runs that comparisons point at live?
**Status:** OPEN
**Decision:** —
**Why:** `/data/benchmarks/` is git-ignored, so results are local by default.
The options are:
- workflow artifacts, which expire after 90 days;
- a curated "pwyll, quiet" baseline committed under a non-ignored path;
- a separate results branch.

A committed baseline gives the paper's figures a reproducible source.
**Decided by:** —
**Log:**

### runner-trust

**Question:** Are GitHub-hosted runner timings stable enough to be a baseline?
**Status:** OPEN
**Decision:** —
**Why:** Unknown. They are shared VMs, possibly on varying hardware.
[runner-stability](#runner-stability) measures it. Possible outcomes:
- wall time and instruction counts are both usable;
- only instruction counts are usable;
- neither is, and pwyll stays the only timing reference.
**Decided by:** —
**Log:**
