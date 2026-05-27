#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Parse Catch2 XML benchmark output and render throughput reports.

Reads one or more Catch2 XML files produced with --reporter xml, cross-references
corpus byte counts from a manifest or the checked-in small corpus files, computes
throughput in MiB/s, and writes a Markdown table and a Vega-Lite JSON chart spec.

Usage:
    uv run python tools/process_benchmark_results.py [--manifest PATH] \\
        [--corpus-dir PATH] [--vegalite PATH] result.xml [result2.xml ...]

Formulas:
    throughput_MiBs  = corpus_bytes / (mean_ns * 1e-9) / 2**20
    throughput_GiBs  = corpus_bytes / (mean_ns * 1e-9) / 2**30
"""

import argparse
import json
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path
from typing import Any

MIB = 2**20
GIB = 2**30

# Default paths (relative to repo root where the tool is typically run from)
DEFAULT_MANIFEST = Path("data/benchmarks/corpus_manifest.json")
DEFAULT_CORPUS_DIR = Path("benchmark/corpus")

# Heuristic table: substring → corpus filename.  More-specific patterns come
# first so they win over shorter keyword matches.
CORPUS_NAME_HINTS: list[tuple[str, str]] = [
    ("windows-1251", "ru_mars_windows1251.bin"),
    ("windows_1251", "ru_mars_windows1251.bin"),
    ("Shift-JIS", "ja_mars_shiftjis.bin"),
    ("shift-jis", "ja_mars_shiftjis.bin"),
    ("shift_jis", "ja_mars_shiftjis.bin"),
    ("shiftjis", "ja_mars_shiftjis.bin"),
    ("en_mars", "en_mars_utf8.txt"),
    ("ar_mars", "ar_mars_utf8.txt"),
    ("ru_mars", "ru_mars_utf8.txt"),
    ("ja_mars", "ja_mars_utf8.txt"),
    ("English", "en_mars_utf8.txt"),
    ("Arabic", "ar_mars_utf8.txt"),
    ("Russian", "ru_mars_utf8.txt"),
    # "Japanese" left out — ambiguous between UTF-8 and Shift-JIS variants
]


@dataclass
class BenchmarkResult:
    binary: str
    test_case: str
    name: str
    mean_ns: float
    low_ns: float
    high_ns: float
    samples: int
    iterations: int


def parse_catch2_xml(path: Path) -> list[BenchmarkResult]:
    """Parse a Catch2 XML file; return all BenchmarkResults found."""
    tree = ET.parse(path)
    root = tree.getroot()
    binary = root.attrib.get("name", path.stem)

    results: list[BenchmarkResult] = []
    for test_case in root.findall(".//TestCase"):
        tc_name = test_case.attrib.get("name", "")
        for br in test_case.findall("BenchmarkResults"):
            mean_el = br.find("mean")
            if mean_el is None:
                continue
            results.append(
                BenchmarkResult(
                    binary=binary,
                    test_case=tc_name,
                    name=br.attrib.get("name", ""),
                    mean_ns=float(mean_el.attrib["value"]),
                    low_ns=float(mean_el.attrib["lowerBound"]),
                    high_ns=float(mean_el.attrib["upperBound"]),
                    samples=int(br.attrib.get("samples", 0)),
                    iterations=int(br.attrib.get("iterations", 1)),
                )
            )
    return results


def load_corpus_sizes(
    manifest_path: Path | None = None,
    corpus_dir: Path | None = None,
) -> dict[str, int]:
    """Return {filename → byte_count} from manifest and/or corpus directory.

    Manifest values override corpus-dir values when both are present.
    """
    sizes: dict[str, int] = {}

    # Scan corpus directory for the small checked-in files
    if corpus_dir is not None and corpus_dir.is_dir():
        for f in corpus_dir.iterdir():
            if f.is_file():
                sizes[f.name] = f.stat().st_size

    # Overlay with manifest entries (larger downloaded files)
    if manifest_path is not None and manifest_path.is_file():
        data = json.loads(manifest_path.read_text(encoding="utf-8"))
        for entry in data:
            filename: str = entry["file"]
            byte_count: int = entry["bytes"]
            sizes[filename] = byte_count

    return sizes


def infer_corpus_file(bench_name: str) -> str | None:
    """Return the most likely corpus filename for a benchmark name, or None."""
    for hint, filename in CORPUS_NAME_HINTS:
        if hint in bench_name:
            return filename
    return None


def throughput_MiBs(corpus_bytes: int, mean_ns: float) -> float:
    """Compute throughput in MiB/s given corpus size and mean latency."""
    if mean_ns <= 0:
        return 0.0
    return corpus_bytes / (mean_ns * 1e-9) / MIB


def _fmt_ns(ns: float) -> str:
    """Format nanoseconds as µs (4 significant figures)."""
    return f"{ns / 1000:.4g} µs"


def render_markdown_table(
    results: list[BenchmarkResult],
    corpus_sizes: dict[str, int],
    label: str = "",
) -> str:
    """Render a Markdown table from benchmark results.

    Columns: Benchmark | Corpus (bytes) | Mean | ±CI | Throughput (MiB/s)
    """
    header_label = f" — {label}" if label else ""
    lines: list[str] = [
        f"## Benchmark Results{header_label}",
        "",
        "| Benchmark | Corpus (B) | Mean | ±CI | MiB/s |",
        "|-----------|-----------|------|-----|-------|",
    ]

    for r in results:
        corpus_file = infer_corpus_file(r.name)
        corpus_bytes = corpus_sizes.get(corpus_file, 0) if corpus_file else 0
        corpus_label = f"{corpus_bytes:,}" if corpus_bytes else "?"
        ci_ns = (r.high_ns - r.low_ns) / 2
        mean_str = _fmt_ns(r.mean_ns)
        ci_str = _fmt_ns(ci_ns)
        if corpus_bytes:
            tput = throughput_MiBs(corpus_bytes, r.mean_ns)
            tput_str = f"{tput:.1f}"
        else:
            tput_str = "?"
        lines.append(
            f"| {r.name} | {corpus_label} | {mean_str} | ±{ci_str} | {tput_str} |"
        )

    lines.append("")
    lines.append(
        "> Throughput = corpus_bytes / mean_time; 1 MiB = 2²⁰ bytes; "
        "CI = 95% confidence interval."
    )
    return "\n".join(lines) + "\n"


def render_vegalite_spec(
    results: list[BenchmarkResult],
    corpus_sizes: dict[str, int],
    title: str = "Benchmark Throughput",
) -> dict[str, Any]:
    """Return a Vega-Lite bar-chart spec showing MiB/s per benchmark.

    Benchmarks with unknown corpus size are omitted from the chart.
    """
    data_values: list[dict[str, object]] = []
    for r in results:
        corpus_file = infer_corpus_file(r.name)
        corpus_bytes = corpus_sizes.get(corpus_file, 0) if corpus_file else 0
        if not corpus_bytes:
            continue
        tput = throughput_MiBs(corpus_bytes, r.mean_ns)
        data_values.append({"benchmark": r.name, "throughput_MiBs": round(tput, 2)})

    return {
        "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
        "title": title,
        "data": {"values": data_values},
        "mark": "bar",
        "encoding": {
            "x": {
                "field": "benchmark",
                "type": "nominal",
                "sort": "-y",
                "title": "Benchmark",
                "axis": {"labelAngle": -30},
            },
            "y": {
                "field": "throughput_MiBs",
                "type": "quantitative",
                "title": "Throughput (MiB/s)",
            },
            "tooltip": [
                {"field": "benchmark", "type": "nominal"},
                {"field": "throughput_MiBs", "type": "quantitative", "title": "MiB/s"},
            ],
        },
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Parse Catch2 XML benchmark results and render throughput tables."
    )
    parser.add_argument(
        "xml_files",
        metavar="XML",
        nargs="+",
        type=Path,
        help="Catch2 XML result files (produced with --reporter xml --out FILE)",
    )
    parser.add_argument(
        "--manifest",
        metavar="PATH",
        type=Path,
        default=DEFAULT_MANIFEST,
        help=f"corpus_manifest.json path (default: {DEFAULT_MANIFEST})",
    )
    parser.add_argument(
        "--corpus-dir",
        metavar="DIR",
        type=Path,
        default=DEFAULT_CORPUS_DIR,
        help=f"Directory of corpus files for size lookup (default: {DEFAULT_CORPUS_DIR})",
    )
    parser.add_argument(
        "--vegalite",
        metavar="PATH",
        type=Path,
        default=None,
        help="Write a Vega-Lite JSON chart spec to this file",
    )
    parser.add_argument(
        "--label",
        metavar="TEXT",
        default="",
        help="Optional label appended to the table heading",
    )
    args = parser.parse_args(argv)

    corpus_sizes = load_corpus_sizes(args.manifest, args.corpus_dir)

    all_results: list[BenchmarkResult] = []
    for xml_path in args.xml_files:
        if not xml_path.is_file():
            print(f"error: {xml_path}: file not found", file=sys.stderr)
            return 1
        all_results.extend(parse_catch2_xml(xml_path))

    if not all_results:
        print("No benchmark results found.", file=sys.stderr)
        return 1

    label = args.label or (args.xml_files[0].stem if len(args.xml_files) == 1 else "")
    print(render_markdown_table(all_results, corpus_sizes, label=label))

    if args.vegalite is not None:
        spec = render_vegalite_spec(all_results, corpus_sizes, title=f"Throughput — {label}")
        args.vegalite.parent.mkdir(parents=True, exist_ok=True)
        args.vegalite.write_text(
            json.dumps(spec, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
        )
        print(f"Vega-Lite spec written to {args.vegalite}", file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
