# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ruff: noqa: E501
"""Tests for tools/dispatch_overhead.py."""

import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from dispatch_overhead import fmt_overhead, main, report
from process_benchmark_results import SIDECAR_FORMAT, sidecar_path

CONVERSION = "windows-1251 → UTF-8, Russian"

# name -> (api, input, input bytes, instructions per call)
BENCHMARKS = {
    "dispatch fixed codecs: short": ("fixed codecs", "short", 32, 1000.0),
    "dispatch by codec: short": ("by codec", "short", 32, 1010.0),
    "dispatch by label: short": ("by label", "short", 32, 2000.0),
    "dispatch fixed codecs: whole": ("fixed codecs", "whole", 435, 10000.0),
    "dispatch by codec: whole": ("by codec", "whole", 435, 10010.0),
}


def write_dispatch_run(
    run_dir: Path, count_runs: int | None = 2, dumps: bool = True
) -> Path:
    """A dispatch suite run as bench-dispatch-instructions leaves one."""
    run_dir.mkdir(parents=True, exist_ok=True)
    results = "".join(
        f'<BenchmarkResults name="{name}" samples="1" resamples="1" iterations="1" '
        'clockResolution="20.0" estimatedDuration="1">'
        '<mean value="1.0" lowerBound="1.0" upperBound="1.0" ci="0.95"/>'
        '<standardDeviation value="0.0" lowerBound="0.0" upperBound="0.0" ci="0.95"/>'
        '<outliers variance="0.0" lowMild="0" lowSevere="0" highMild="0" highSevere="0"/>'
        "</BenchmarkResults>"
        for name in BENCHMARKS
    )
    xml = run_dir / "dispatch.xml"
    xml.write_text(
        '<?xml version="1.0" encoding="UTF-8"?>'
        '<Catch2TestRun name="beman.transcode.benchmarks.dispatch">'
        f'<TestCase name="dispatch" tags="[benchmark][dispatch]">{results}'
        '<OverallResult success="true" skips="0"/></TestCase></Catch2TestRun>',
        encoding="utf-8",
    )
    environment: dict[str, object] = {"build_config": "Perf"}
    if count_runs is not None:
        environment["count_runs"] = count_runs
    sidecar = {
        "format": SIDECAR_FORMAT,
        "environment": environment,
        "benchmarks": [
            {
                "name": name,
                "conversion": CONVERSION,
                "library": "beman.transcode",
                "api": api,
                "input": input_,
                "input_source": "synthetic",
                "input_bytes": size,
            }
            for name, (api, input_, size, _) in BENCHMARKS.items()
        ],
    }
    sidecar_path(xml).write_text(json.dumps(sidecar), encoding="utf-8")
    if dumps and count_runs:
        for i, (name, (_, _, _, per_call)) in enumerate(BENCHMARKS.items(), 1):
            total = round(per_call * count_runs)
            (run_dir / f"dispatch.callgrind.4242.{i}").write_text(
                "# callgrind format\nversion: 1\n"
                f"desc: Trigger: Client Request: {name}\n"
                f"events: Ir\nsummary: {total}\n\nfn=(1) main\n0 {total}\n",
                encoding="utf-8",
            )
    return run_dir


def test_fmt_overhead_shows_instructions_and_share() -> None:
    assert fmt_overhead(1010.0, 1000.0) == "+10 (+1.00%)"
    assert fmt_overhead(990.0, 1000.0) == "-10 (-1.00%)"
    assert fmt_overhead(None, 1000.0) == "—"
    assert fmt_overhead(1000.0, None) == "—"


def test_report_tabulates_overhead_per_call(tmp_path: Path) -> None:
    text = report(write_dispatch_run(tmp_path) / "dispatch.xml")
    assert "mean of 2 counted runs" in text
    lines = text.splitlines()
    short = next(line for line in lines if "| short |" in line)
    assert short == (
        f"| {CONVERSION} | short | 32 | 1,000 | 1,010 | +10 (+1.00%) | 2,000 | +1,000 (+100.00%) |"
    )
    whole = next(line for line in lines if "| whole |" in line)
    assert whole.endswith("| 10,010 | +10 (+0.10%) | — | — |")


def test_report_refuses_a_timed_run(tmp_path: Path) -> None:
    with pytest.raises(SystemExit, match="not run in count mode"):
        report(write_dispatch_run(tmp_path, count_runs=None) / "dispatch.xml")


def test_report_refuses_a_run_without_dumps(tmp_path: Path) -> None:
    with pytest.raises(SystemExit, match="no callgrind counts"):
        report(write_dispatch_run(tmp_path, dumps=False) / "dispatch.xml")


def test_main_accepts_the_directory_and_writes_markdown(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    run_dir = write_dispatch_run(tmp_path / "run")
    out = tmp_path / "overhead.md"
    assert main([str(run_dir), "--markdown", str(out)]) == 0
    assert out.read_text(encoding="utf-8") == capsys.readouterr().out
