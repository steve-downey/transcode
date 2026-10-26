# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ruff: noqa: E501
"""Tests for tools/runner_stability.py."""

import json
import statistics
import sys
from collections.abc import Mapping
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from process_benchmark_results import SIDECAR_FORMAT
from runner_stability import (
    FORMAT,
    Spread,
    cpu_groups,
    fmt_percent,
    load_job,
    main,
    match_jobs,
    render_json,
    render_markdown,
    summarize,
)

EN = "UTF-8 decode: English"
AR = "UTF-8 decode: Arabic"
TINY = "optimized away"


def catch2_xml(means: Mapping[str, float]) -> str:
    results = "".join(
        f'    <BenchmarkResults name="{name}" samples="10" resamples="100" '
        'iterations="1" clockResolution="20.0" estimatedDuration="1000">\n'
        f'      <mean value="{mean}" lowerBound="{mean}" upperBound="{mean}" ci="0.95"/>\n'
        '      <standardDeviation value="1.0" lowerBound="1.0" upperBound="1.0" ci="0.95"/>\n'
        '      <outliers variance="0.0" lowMild="0" lowSevere="0" highMild="0" highSevere="0"/>\n'
        "    </BenchmarkResults>\n"
        for name, mean in means.items()
    )
    return (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<Catch2TestRun name="beman.transcode.benchmarks.smoke" rng-seed="1" '
        'xml-format-version="3" catch2-version="3.14.0">\n'
        '  <TestCase name="tc" tags="[smoke]" filename="x.cpp" line="1">\n'
        + results
        + '    <OverallResult success="true" skips="0"/>\n  </TestCase>\n'
        "</Catch2TestRun>\n"
    )


def sidecar(names: list[str], cpu: str, count_runs: int = 0) -> dict[str, object]:
    environment: dict[str, object] = {
        "timestamp": "2026-10-25T00:00:00Z",
        "cpu": cpu,
        "cpus": 4,
        "kernel": "6.8.0",
        "compiler": "gcc 16.1.0",
        "build_config": "RelWithDebInfo",
        "optimized": True,
        "sanitizers": [],
        "load_start": [0.5, 0.4, 0.3],
        "load_end": [1.5, 0.6, 0.4],
    }
    if count_runs:
        environment["count_runs"] = count_runs
    return {
        "format": SIDECAR_FORMAT,
        "environment": environment,
        "benchmarks": [
            {
                "name": n,
                "conversion": "UTF-8 → UTF-32",
                "library": "beman.transcode",
                "api": "whatwg_decode view",
                "input": "en_mars_utf8.txt",
                "input_source": "corpus",
                "input_bytes": 4096,
            }
            for n in names
        ],
    }


def callgrind_dump(name: str, total: int) -> str:
    return (
        "# callgrind format\nversion: 1\ncreator: callgrind-3.26.0\npid: 1\n"
        f"cmd: smoke\npart: 1\n\ndesc: Trigger: Client Request: {name}\n"
        f"positions: line\nevents: Ir\nsummary: {total}\n"
        f"\nfn=(1) main\n0 {total}\ntotals: {total}\n"
    )


def write_job(
    job_dir: Path,
    means: Mapping[str, float],
    counts: Mapping[str, int] | None = None,
    cpu: str = "Test CPU",
    count_runs: int = 2,
) -> Path:
    """A job directory as bench-stability-job leaves one: timing/ and instructions/."""
    timing = job_dir / "timing"
    timing.mkdir(parents=True)
    (timing / "smoke.xml").write_text(catch2_xml(means), encoding="utf-8")
    (timing / "smoke.meta.json").write_text(
        json.dumps(sidecar(list(means), cpu)), encoding="utf-8"
    )
    if counts is not None:
        counted = job_dir / "instructions"
        counted.mkdir()
        (counted / "smoke.xml").write_text(
            catch2_xml({n: 1e6 for n in counts}), encoding="utf-8"
        )
        (counted / "smoke.meta.json").write_text(
            json.dumps(sidecar(list(counts), cpu, count_runs)), encoding="utf-8"
        )
        for i, (name, per_call) in enumerate(counts.items(), 1):
            (counted / f"smoke.callgrind.77.{i}").write_text(
                callgrind_dump(name, per_call * count_runs), encoding="utf-8"
            )
    return job_dir


TIMES = [1000.0, 1100.0, 900.0]


@pytest.fixture
def three_jobs(tmp_path: Path) -> list[Path]:
    return [
        write_job(
            tmp_path / f"job-{i}",
            {EN: t, AR: 2 * t, TINY: 0.2},
            {EN: 40000, AR: 80000 + i},
            cpu="CPU A" if i < 3 else "CPU B",
        )
        for i, t in enumerate(TIMES, 1)
    ]


def test_bootstrap() -> None:
    assert FORMAT.startswith("beman.transcode.")


def test_spread_cv_and_ratio() -> None:
    s = Spread({0: 1000.0, 1: 1100.0, 2: 900.0})
    assert s.cv == pytest.approx(statistics.stdev(TIMES) / 1000.0)
    assert s.ratio == pytest.approx(1100.0 / 900.0)
    assert s.median == 1000.0


def test_spread_needs_two_jobs() -> None:
    one = Spread({0: 5.0})
    assert one.cv is None and one.ratio is None
    assert one.median == 5.0
    assert Spread({}).median is None


def test_spread_within_keeps_only_those_jobs() -> None:
    s = Spread({0: 1.0, 1: 2.0, 2: 3.0}).within([0, 2])
    assert s.values == {0: 1.0, 2: 3.0}


def test_load_job_reads_timing_and_counts_at_any_depth(three_jobs: list[Path]) -> None:
    job = load_job(three_jobs[0])
    assert job.label == "job-1"
    assert len(job.suites) == 2
    assert job.cpu == "CPU A"
    assert job.load == "0.50 – 1.50"


def test_load_job_without_xml_is_an_error(tmp_path: Path) -> None:
    with pytest.raises(ValueError, match="no XML files"):
        load_job(tmp_path)


def test_match_jobs_joins_time_and_count_per_benchmark(
    three_jobs: list[Path],
) -> None:
    rows = {r.name: r for r in match_jobs([load_job(p) for p in three_jobs])}
    en = rows[EN]
    assert en.time.values == {0: 1000.0, 1: 1100.0, 2: 900.0}
    assert en.instructions.values == {0: 40000, 1: 40000, 2: 40000}
    assert en.instructions.cv == 0
    assert rows[AR].instructions.cv == pytest.approx(
        statistics.stdev([80001, 80002, 80003]) / 80002
    )


def test_implausible_time_is_left_out(three_jobs: list[Path]) -> None:
    rows = {r.name: r for r in match_jobs([load_job(p) for p in three_jobs])}
    assert rows[TINY].time.n == 0
    assert rows[TINY].time.cv is None


def test_benchmark_missing_from_a_job_has_fewer_values(tmp_path: Path) -> None:
    jobs = [
        load_job(write_job(tmp_path / "a", {EN: 1000.0, AR: 1.0e4})),
        load_job(write_job(tmp_path / "b", {EN: 1010.0})),
    ]
    rows = {r.name: r for r in match_jobs(jobs)}
    assert rows[EN].time.n == 2
    assert rows[AR].time.n == 1
    assert "| 1 of 2 |" in render_markdown(jobs, list(rows.values()))


def test_cpu_groups_most_common_first(three_jobs: list[Path]) -> None:
    jobs = [load_job(p) for p in reversed(three_jobs)]
    assert cpu_groups(jobs) == {"CPU A": [1, 2], "CPU B": [0]}


def test_summarize_ignores_missing_cvs() -> None:
    s = summarize([0.001, None, 0.02, 0.1])
    assert s.benchmarks == 3
    assert s.median == 0.02
    assert s.worst == 0.1
    assert s.under_1pct == 1
    assert s.under_5pct == 2
    assert summarize([None]).benchmarks == 0


def test_fmt_percent() -> None:
    assert fmt_percent(None) == "—"
    assert fmt_percent(0.0) == "0%"
    assert fmt_percent(0.0000001) == "<0.01%"
    assert fmt_percent(0.0123) == "1.23%"
    assert fmt_percent(0.5) == "50%"


def test_markdown_lists_jobs_summary_and_cpu_groups(three_jobs: list[Path]) -> None:
    jobs = [load_job(p) for p in three_jobs]
    text = render_markdown(jobs, match_jobs(jobs), label="abc123")
    assert text.startswith("## Runner Stability — abc123\n")
    assert "| job-3 | CPU B | 4 | 6.8.0 | 0.50 – 1.50 |" in text
    assert "| Wall time | 2 |" in text
    assert "| Instructions | 2 |" in text
    assert "### Wall time within each CPU model" in text
    assert "| CPU A (2 jobs) | 2 |" in text
    assert "| CPU B (1 job) | 0 |" in text
    assert "### smoke" in text
    assert "No threshold is applied." in text


def test_one_cpu_model_has_no_cpu_section(tmp_path: Path) -> None:
    jobs = [load_job(write_job(tmp_path / f"j{i}", {EN: 1000.0 + i})) for i in (1, 2)]
    text = render_markdown(jobs, match_jobs(jobs))
    assert "within each CPU model" not in text


def test_json_has_every_value_by_job(three_jobs: list[Path]) -> None:
    jobs = [load_job(p) for p in three_jobs]
    data = render_json(jobs, match_jobs(jobs))
    assert data["format"] == FORMAT
    assert [j["cpu"] for j in data["jobs"]] == ["CPU A", "CPU A", "CPU B"]
    en = next(b for b in data["benchmarks"] if b["name"] == EN)
    assert en["time_ns"] == {"job-1": 1000.0, "job-2": 1100.0, "job-3": 900.0}
    assert en["instructions_cv"] == 0
    assert data["summary"]["time"]["benchmarks"] == 2
    assert set(data["summary"]["time_by_cpu"]) == {"CPU A", "CPU B"}


def test_main_writes_markdown_and_json(three_jobs: list[Path], tmp_path: Path) -> None:
    md, js = tmp_path / "out" / "report.md", tmp_path / "out" / "report.json"
    argv = ["--markdown", str(md), "--json", str(js), *map(str, three_jobs)]
    assert main(argv) == 0
    assert "## Runner Stability" in md.read_text(encoding="utf-8")
    assert json.loads(js.read_text(encoding="utf-8"))["format"] == FORMAT


def test_main_rejects_a_missing_directory(tmp_path: Path) -> None:
    assert main([str(tmp_path / "nope")]) == 1


def test_main_rejects_a_directory_without_results(tmp_path: Path) -> None:
    assert main([str(tmp_path)]) == 1
