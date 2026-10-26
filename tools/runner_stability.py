#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Report how much benchmark results vary across identical CI jobs.

The runner-stability workflow (``.github/workflows/bench-runner-stability.yml``)
builds the benchmarks once, then runs the same binaries in several jobs, each
on whatever hosted runner it gets.  Each job leaves one directory: the XML and
sidecars of a timed pass, and those of a count pass with their callgrind dumps.
This tool reads one directory per job, matches each benchmark across the jobs
by suite and name, and reports, per benchmark:

* the coefficient of variation (standard deviation ÷ mean) across jobs, of the
  wall time and of the instructions per call;
* the slowest job's time ÷ the fastest's.

It also lists each job's CPU model, and repeats the wall-time summary within
each CPU model, so that a hardware change between jobs is told apart from a
noisy VM.  It applies no thresholds; the numbers answer runner-trust in
docs/plans/bench-comparison.md.

Usage:
    uv run python tools/runner_stability.py [--label TEXT] \\
        [--markdown PATH] [--json PATH] JOB_DIR [JOB_DIR ...]

Formulas:
    cv      = stdev(per-job values) / mean(per-job values)   (sample stdev)
    spread  = max(per-job time) / min(per-job time)
"""

import argparse
import json
import statistics
import sys
from collections.abc import Iterable
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).parent))

from process_benchmark_results import (  # noqa: E402
    Measurement,
    SuiteRun,
    axis_text,
    fmt_bytes,
    fmt_instructions,
    fmt_time,
    load_suite,
    suite_measurements,
)

FORMAT = "beman.transcode.runner-stability/1"

# A time is left out for these flags: the work was optimized away, or there
# was no input, so the time measures nothing worth comparing.  "noisy" stays
# in, since noise is what this report measures.
TIME_EXCLUDED_FLAGS = frozenset({"empty-input", "implausible"})
COUNT_EXCLUDED_FLAGS = frozenset({"empty-input", "few-instructions"})


# ── Model ───────────────────────────────────────────────────────────────────


@dataclass
class Job:
    """One CI job's results: every suite in its directory."""

    label: str
    suites: list[SuiteRun]

    def env(self, key: str) -> str:
        """One environment field across this job's suites; differing values joined."""
        values = sorted({axis_text(s.environment.get(key)) for s in self.suites})
        return "; ".join(v for v in values if v)

    @property
    def cpu(self) -> str:
        return self.env("cpu") or "unknown"

    @property
    def started(self) -> str:
        stamps = sorted(
            {str(s.environment.get("timestamp", "")) for s in self.suites} - {""}
        )
        return stamps[0] if stamps else ""

    @property
    def load(self) -> str:
        """The 1-minute load across the job's timed suites, low – high."""
        loads = [
            load[0]
            for s in self.suites
            if not s.counted
            for load in (s.environment.get("load_start"), s.environment.get("load_end"))
            if load
        ]
        if not loads:
            return ""
        low, high = min(loads), max(loads)
        return f"{low:.2f}" if low == high else f"{low:.2f} – {high:.2f}"


@dataclass
class Spread:
    """One measure of one benchmark across jobs."""

    values: dict[int, float]  # job index → value

    @property
    def n(self) -> int:
        return len(self.values)

    @property
    def median(self) -> float | None:
        return statistics.median(self.values.values()) if self.values else None

    @property
    def cv(self) -> float | None:
        """Sample standard deviation ÷ mean; None with fewer than two jobs."""
        if self.n < 2:
            return None
        mean = statistics.fmean(self.values.values())
        if mean <= 0:
            return None
        return statistics.stdev(self.values.values()) / mean

    @property
    def ratio(self) -> float | None:
        """Largest value ÷ smallest; None with fewer than two jobs."""
        if self.n < 2:
            return None
        low = min(self.values.values())
        return max(self.values.values()) / low if low > 0 else None

    def within(self, jobs: Iterable[int]) -> "Spread":
        keep = set(jobs)
        return Spread({j: v for j, v in self.values.items() if j in keep})


@dataclass
class Row:
    """One benchmark, matched by (suite, name), across the jobs."""

    suite: str
    name: str
    first: Measurement
    time: Spread
    instructions: Spread
    # The jobs whose results hold this benchmark, whether or not its values
    # were left out.
    present: set[int] = field(default_factory=set)

    @property
    def description(self) -> str:
        m = self.first
        parts = [m.conversion, m.library, m.api]
        what = " · ".join(p for p in parts if p)
        if m.input:
            what += f" — {m.input} ({fmt_bytes(m.input_bytes)})"
        return what or self.name


def load_job(path: Path) -> Job:
    """Every XML file under path, at any depth, as one job named for path."""
    files = sorted(path.rglob("*.xml"))
    if not files:
        raise ValueError(f"{path}: no XML files found")
    return Job(path.name, [load_suite(f) for f in files])


def match_jobs(jobs: list[Job]) -> list[Row]:
    """Match benchmarks across jobs by (suite, name), in first-seen order."""
    rows: dict[tuple[str, str], Row] = {}
    for j, job in enumerate(jobs):
        for m in suite_measurements(job.suites):
            row = rows.setdefault(
                (m.suite, m.name), Row(m.suite, m.name, m, Spread({}), Spread({}))
            )
            row.present.add(j)
            if m.timed and not TIME_EXCLUDED_FLAGS.intersection(m.flags):
                row.time.values[j] = m.result.mean_ns
            if m.instructions is not None and not COUNT_EXCLUDED_FLAGS.intersection(
                m.flags
            ):
                row.instructions.values[j] = m.instructions
    return list(rows.values())


def cpu_groups(jobs: list[Job]) -> dict[str, list[int]]:
    """Job indexes by CPU model, the most common model first."""
    groups: dict[str, list[int]] = {}
    for j, job in enumerate(jobs):
        groups.setdefault(job.cpu, []).append(j)
    return dict(sorted(groups.items(), key=lambda kv: -len(kv[1])))


# ── Summaries ───────────────────────────────────────────────────────────────


@dataclass
class Summary:
    """The distribution of one measure's CV over the benchmarks."""

    benchmarks: int
    median: float | None
    p90: float | None
    worst: float | None
    under_1pct: int
    under_5pct: int


def summarize(cvs: Iterable[float | None]) -> Summary:
    values = sorted(c for c in cvs if c is not None)
    if not values:
        return Summary(0, None, None, None, 0, 0)
    p90 = values[min(len(values) - 1, int(0.9 * len(values)))]
    return Summary(
        benchmarks=len(values),
        median=statistics.median(values),
        p90=p90,
        worst=values[-1],
        under_1pct=sum(v < 0.01 for v in values),
        under_5pct=sum(v < 0.05 for v in values),
    )


def fmt_percent(x: float | None) -> str:
    if x is None:
        return "—"
    p = 100 * x
    if p == 0:
        return "0%"
    if p < 0.01:
        return "<0.01%"
    if p < 10:
        return f"{p:.2f}%"
    return f"{p:.0f}%"


def fmt_factor(x: float | None) -> str:
    return "—" if x is None else f"×{x:.2f}"


def fmt_count(n: int, of: int) -> str:
    return str(n) if n == of else f"{n} of {of}"


# ── Markdown ────────────────────────────────────────────────────────────────


def _md(text: str) -> str:
    return text.replace("|", "\\|")


def _summary_line(title: str, s: Summary) -> str:
    if not s.benchmarks:
        return f"| {title} | 0 | — | — | — | — | — |"
    return (
        f"| {title} | {s.benchmarks} | {fmt_percent(s.median)} | "
        f"{fmt_percent(s.p90)} | {fmt_percent(s.worst)} | "
        f"{s.under_1pct} | {s.under_5pct} |"
    )


SUMMARY_HEAD = [
    "| Measure | Benchmarks | Median CV | 90th pct CV | Worst CV | CV < 1% | CV < 5% |",
    "|---------|-----------:|----------:|------------:|---------:|--------:|--------:|",
]

NOTE = (
    "CV is the sample standard deviation of the per-job values ÷ their mean, "
    "over the jobs that ran the benchmark; a benchmark in fewer than two jobs "
    "has none.  A job's wall time is Catch2's mean for the benchmark in that "
    "job; spread is the slowest job's ÷ the fastest's.  Instructions are "
    "callgrind's Ir per call.  Times flagged implausible or empty-input, and "
    "counts flagged few-instructions or empty-input, are left out.  No "
    "threshold is applied."
)


def render_markdown(jobs: list[Job], rows: list[Row], label: str = "") -> str:
    heading = "Runner Stability" + (f" — {label}" if label else "")
    lines = [f"## {heading}", ""]
    for key, title in (
        ("compiler", "Compiler"),
        ("build_config", "Build config"),
        ("commit", "Commit"),
    ):
        values = sorted({j.env(key) for j in jobs} - {""})
        if values:
            lines.append(f"- **{title}:** {_md('; '.join(values))}")
    lines += [f"- **Jobs:** {len(jobs)}", ""]

    lines += [
        "### Jobs",
        "",
        "| Job | CPU | CPUs | Kernel | Load (1 min) | Started |",
        "|-----|-----|-----:|--------|-------------:|---------|",
    ]
    for job in jobs:
        cells = [
            job.label,
            job.cpu,
            job.env("cpus"),
            job.env("kernel"),
            job.load,
            job.started,
        ]
        lines.append("| " + " | ".join(_md(c) for c in cells) + " |")
    lines.append("")

    lines += ["### Summary", "", *SUMMARY_HEAD]
    lines.append(_summary_line("Wall time", summarize(r.time.cv for r in rows)))
    lines.append(
        _summary_line("Instructions", summarize(r.instructions.cv for r in rows))
    )
    lines.append("")

    groups = cpu_groups(jobs)
    if len(groups) > 1:
        lines += [
            "### Wall time within each CPU model",
            "",
            "The same summary, over only the jobs that ran on one CPU model.",
            "",
            *[h.replace("Measure", "CPU model") for h in SUMMARY_HEAD[:1]],
            SUMMARY_HEAD[1].replace("|---------|", "|-----------|"),
        ]
        for cpu, members in groups.items():
            s = summarize(r.time.within(members).cv for r in rows)
            title = f"{cpu} ({len(members)} job{'s' if len(members) > 1 else ''})"
            lines.append(_summary_line(_md(title), s))
        lines.append("")

    n = len(jobs)
    suites = list(dict.fromkeys(r.suite for r in rows))
    for suite in suites:
        lines += [
            f"### {suite}",
            "",
            "| Benchmark | Jobs | Median time | Time CV | Spread "
            "| Instructions | Instr CV |",
            "|-----------|-----:|------------:|--------:|-------:"
            "|-------------:|---------:|",
        ]
        for r in rows:
            if r.suite != suite:
                continue
            t, i = r.time, r.instructions
            cells = [
                r.description,
                fmt_count(len(r.present), n),
                fmt_time(t.median) if t.median is not None else "—",
                fmt_percent(t.cv),
                fmt_factor(t.ratio),
                fmt_instructions(i.median) if i.median is not None else "—",
                fmt_percent(i.cv),
            ]
            lines.append("| " + " | ".join(_md(c) for c in cells) + " |")
        lines.append("")

    lines.append("> " + NOTE)
    return "\n".join(lines) + "\n"


# ── JSON ────────────────────────────────────────────────────────────────────


def _summary_json(s: Summary) -> dict[str, Any]:
    return {
        "benchmarks": s.benchmarks,
        "median_cv": s.median,
        "p90_cv": s.p90,
        "worst_cv": s.worst,
        "under_1pct": s.under_1pct,
        "under_5pct": s.under_5pct,
    }


def render_json(jobs: list[Job], rows: list[Row]) -> dict[str, Any]:
    """Every per-job value, for analysis beyond the Markdown summary."""
    labels = [j.label for j in jobs]

    def by_job(s: Spread) -> dict[str, float]:
        return {labels[j]: v for j, v in sorted(s.values.items())}

    return {
        "format": FORMAT,
        "jobs": [
            {
                "label": j.label,
                "cpu": j.cpu,
                "cpus": j.env("cpus"),
                "kernel": j.env("kernel"),
                "started": j.started,
            }
            for j in jobs
        ],
        "summary": {
            "time": _summary_json(summarize(r.time.cv for r in rows)),
            "instructions": _summary_json(summarize(r.instructions.cv for r in rows)),
            "time_by_cpu": {
                cpu: _summary_json(summarize(r.time.within(m).cv for r in rows))
                for cpu, m in cpu_groups(jobs).items()
            },
        },
        "benchmarks": [
            {
                "suite": r.suite,
                "name": r.name,
                "conversion": r.first.conversion,
                "library": r.first.library,
                "api": r.first.api,
                "input": r.first.input,
                "input_bytes": r.first.input_bytes,
                "time_ns": by_job(r.time),
                "time_cv": r.time.cv,
                "instructions": by_job(r.instructions),
                "instructions_cv": r.instructions.cv,
            }
            for r in rows
        ],
    }


# ── Command line ────────────────────────────────────────────────────────────


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Report how much benchmark results vary across identical jobs.",
    )
    parser.add_argument(
        "job_dirs",
        metavar="JOB_DIR",
        nargs="+",
        type=Path,
        help="One directory per job, holding its XML files, sidecars and "
        "callgrind dumps at any depth; the job is named for the directory",
    )
    parser.add_argument(
        "--label", metavar="TEXT", default="", help="Label for the report heading"
    )
    parser.add_argument(
        "--markdown",
        metavar="PATH",
        type=Path,
        default=None,
        help="Write the Markdown report here instead of stdout",
    )
    parser.add_argument(
        "--json", metavar="PATH", type=Path, default=None, help="Write every value"
    )
    args = parser.parse_args(argv)

    jobs: list[Job] = []
    for path in args.job_dirs:
        if not path.is_dir():
            print(f"error: {path}: not a directory", file=sys.stderr)
            return 1
        try:
            jobs.append(load_job(path))
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 1
    rows = match_jobs(jobs)
    if not rows:
        print("No benchmark results found.", file=sys.stderr)
        return 1

    markdown = render_markdown(jobs, rows, args.label)
    if args.markdown is None:
        print(markdown)
    else:
        _write(args.markdown, markdown)
    if args.json is not None:
        text = json.dumps(render_json(jobs, rows), indent=2, ensure_ascii=False)
        _write(args.json, text + "\n")
    return 0


def _write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    print(f"wrote {path}", file=sys.stderr)


if __name__ == "__main__":
    sys.exit(main())
