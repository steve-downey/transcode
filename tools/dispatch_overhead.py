# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tabulate the overhead of runtime codec dispatch from counted benchmarks.

Reads the dispatch suite run in count mode (``make bench-dispatch-instructions``:
``dispatch.xml``, its sidecar, and the callgrind dumps beside it) and prints,
for each conversion and input, the instructions per call of each way of
choosing the codecs, and each runtime path's difference from the fixed codecs.

The general report divides counts by input size.  That suits a cost per byte,
but dispatch costs the same per call whatever the input, so this table keeps
counts per call and shows the difference in instructions and as a share of
the floor.
"""

import argparse
import sys
from dataclasses import dataclass
from pathlib import Path

from process_benchmark_results import Measurement, load_suite

BASELINE_API = "fixed codecs"
RUNTIME_APIS = ("by codec", "by label")


@dataclass
class Row:
    """One conversion on one input: instructions per call for each API."""

    conversion: str
    input: str
    input_bytes: int | None
    counts: dict[str, float]


def collect_rows(measurements: list[Measurement]) -> list[Row]:
    """Group counted measurements by conversion and input, in first-seen order."""
    rows: dict[tuple[str, str], Row] = {}
    for m in measurements:
        if m.instructions is None:
            continue
        key = (m.conversion, m.input)
        row = rows.setdefault(key, Row(m.conversion, m.input, m.input_bytes, {}))
        row.counts[m.api] = m.instructions
    return list(rows.values())


def fmt_count(x: float) -> str:
    return f"{x:,.0f}"


def fmt_overhead(value: float | None, base: float | None) -> str:
    """The difference from the baseline, in instructions and as a percentage."""
    if value is None or base is None or base <= 0:
        return "—"
    delta = value - base
    return f"{delta:+,.0f} ({delta / base:+.2%})"


def markdown_table(rows: list[Row]) -> str:
    header = ["Conversion", "Input", "Bytes", BASELINE_API]
    for api in RUNTIME_APIS:
        header += [api, f"{api} overhead"]
    lines = [
        "| " + " | ".join(header) + " |",
        "|" + "|".join(["---"] * 2 + ["---:"] * (len(header) - 2)) + "|",
    ]
    for row in rows:
        base = row.counts.get(BASELINE_API)
        cells = [
            row.conversion,
            row.input,
            "—" if row.input_bytes is None else f"{row.input_bytes:,}",
            "—" if base is None else fmt_count(base),
        ]
        for api in RUNTIME_APIS:
            value = row.counts.get(api)
            cells += [
                "—" if value is None else fmt_count(value),
                fmt_overhead(value, base),
            ]
        lines.append("| " + " | ".join(cells) + " |")
    return "\n".join(lines) + "\n"


def report(xml_path: Path) -> str:
    suite = load_suite(xml_path)
    if not suite.counted:
        raise SystemExit(
            f"{xml_path}: not run in count mode; use make bench-dispatch-instructions"
        )
    rows = collect_rows(suite.measurements)
    if not rows:
        raise SystemExit(f"{xml_path}: no callgrind counts found beside it")
    return (
        f"Instructions per call, mean of {suite.count_runs} counted runs.\n"
        f"Overhead is the difference from `{BASELINE_API}`.\n\n" + markdown_table(rows)
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "path",
        type=Path,
        help="dispatch.xml, or the directory holding it",
    )
    parser.add_argument("--markdown", type=Path, help="also write the table here")
    args = parser.parse_args(argv)

    xml_path = args.path / "dispatch.xml" if args.path.is_dir() else args.path
    text = report(xml_path)
    sys.stdout.write(text)
    if args.markdown:
        args.markdown.write_text(text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
