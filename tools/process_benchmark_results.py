#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Turn Catch2 benchmark XML into throughput reports and charts.

Each XML file written with ``--reporter xml --out FILE.xml`` has a sidecar,
``FILE.meta.json``, written by the benchmark fixture's Catch2 listener.  The
sidecar says what every benchmark converts, which library and API do the
converting, which input it read and how many bytes that input held, and how
the binary was built.  This tool joins the two, computes throughput, flags
results that cannot be trusted, and renders:

* Markdown tables (stdout, or ``--markdown PATH``) for PRs and papers;
* a self-contained HTML report (``--html PATH``) with one chart per conversion;
* standalone SVG charts (``--svg-dir DIR``), one file per conversion, for
  figures in papers;
* a Vega-Lite spec (``--vegalite PATH``) for interactive exploration.

Usage:
    uv run python tools/process_benchmark_results.py [--label TEXT] \\
        [--markdown PATH] [--html PATH] [--svg-dir DIR] [--vegalite PATH] \\
        result.xml [result2.xml ...]

Formulas:
    throughput_MiBs        = input_bytes / (mean_ns * 1e-9) / 2**20
    throughput CI (95%)    = input_bytes / upper_ns .. input_bytes / lower_ns
"""

import argparse
import html
import json
import math
import re
import sys
import xml.etree.ElementTree as ET
from collections.abc import Iterable
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

MIB = 2**20

SIDECAR_FORMAT = "beman.transcode.bench-meta/1"

# A result moving more than this many input bytes per nanosecond (~30 GiB/s)
# is beyond any scalar transcoder and beyond the SIMD baselines; the work was
# almost certainly optimized away.  So is a whole call finishing in under a
# nanosecond, whatever its input size.
IMPLAUSIBLE_BYTES_PER_NS = 32.0
IMPLAUSIBLE_MEAN_NS = 1.0

# Catch2 calls outlier variance above 0.5 "severely inflated": the mean is not
# a reliable summary of the samples.
NOISY_OUTLIER_VARIANCE = 0.5

# Flags, by slug.  "Excluded" flags keep a result off the charts and out of
# the throughput column, because the number would be wrong, not just noisy.
FLAG_TEXT: dict[str, str] = {
    "no-metadata": "no sidecar entry, so the input size is unknown",
    "empty-input": "input was empty (corpus file missing?)",
    "implausible": "too fast to be real work; likely optimized away",
    "noisy": "outliers severely inflate the variance",
}
EXCLUDED_FLAGS = frozenset({"no-metadata", "empty-input", "implausible"})

# Library identity → categorical slot and marker shape.  Colour follows the
# library across every chart; the shape repeats the identity for print and
# colour-vision deficiency.  Anything else is drawn as a hollow grey ring.
LIBRARY_ORDER = [
    "beman.transcode",
    "iconv",
    "encoding_rs",
    "simdutf",
    "C/C++ standard library",
]
LIBRARY_SHAPES = ["circle", "square", "diamond", "triangle", "triangle-down"]

INPUT_SOURCE_TEXT = {
    "data": "downloaded",
    "corpus": "checked-in",
    "synthetic": "synthetic",
    "missing": "missing",
}


# ── Data model ──────────────────────────────────────────────────────────────


@dataclass
class BenchmarkResult:
    """One BenchmarkResults element from a Catch2 XML file."""

    binary: str
    test_case: str
    name: str
    mean_ns: float
    low_ns: float
    high_ns: float
    samples: int
    iterations: int
    std_dev_ns: float = 0.0
    outlier_variance: float = 0.0


@dataclass
class Measurement:
    """A benchmark result joined with what the benchmark fixture recorded."""

    result: BenchmarkResult
    suite: str
    conversion: str = ""
    library: str = ""
    api: str = ""
    input: str = ""
    input_source: str = ""
    input_bytes: int | None = None
    flags: list[str] = field(default_factory=list)

    @property
    def name(self) -> str:
        return self.result.name

    @property
    def plotted(self) -> bool:
        return not EXCLUDED_FLAGS.intersection(self.flags)

    @property
    def throughput(self) -> float | None:
        if self.input_bytes is None or not self.plotted:
            return None
        return throughput_MiBs(self.input_bytes, self.result.mean_ns)

    @property
    def throughput_ci(self) -> tuple[float, float] | None:
        if self.input_bytes is None or not self.plotted:
            return None
        return (
            throughput_MiBs(self.input_bytes, self.result.high_ns),
            throughput_MiBs(self.input_bytes, self.result.low_ns),
        )


@dataclass
class Run:
    """One XML file and its sidecar."""

    path: Path
    suite: str
    environment: dict[str, Any]
    measurements: list[Measurement]
    has_sidecar: bool


@dataclass
class InputGroup:
    input: str
    input_bytes: int | None
    input_source: str
    rows: list[Measurement]


@dataclass
class ConversionGroup:
    conversion: str
    inputs: list[InputGroup]

    @property
    def measurements(self) -> list[Measurement]:
        return [m for g in self.inputs for m in g.rows]

    @property
    def libraries(self) -> list[str]:
        return ordered_libraries(m.library for m in self.measurements)


# ── Parsing ─────────────────────────────────────────────────────────────────


def parse_catch2_xml(path: Path) -> list[BenchmarkResult]:
    """Parse a Catch2 XML file; return all BenchmarkResults found."""
    tree = ET.parse(path)
    root = tree.getroot()
    binary = root.attrib.get("name", path.stem)

    results: list[BenchmarkResult] = []
    for test_case in root.findall(".//TestCase"):
        tc_name = test_case.attrib.get("name", "")
        for br in test_case.iter("BenchmarkResults"):
            mean_el = br.find("mean")
            if mean_el is None:
                continue
            std_el = br.find("standardDeviation")
            outliers_el = br.find("outliers")
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
                    std_dev_ns=(
                        float(std_el.attrib["value"]) if std_el is not None else 0.0
                    ),
                    outlier_variance=(
                        float(outliers_el.attrib.get("variance", 0.0))
                        if outliers_el is not None
                        else 0.0
                    ),
                )
            )
    return results


def sidecar_path(xml_path: Path) -> Path:
    """The metadata sidecar the benchmark fixture writes beside xml_path."""
    return xml_path.with_suffix(".meta.json")


def load_sidecar(path: Path) -> dict[str, Any] | None:
    """Load a sidecar, or None if absent.  Rejects an unknown format."""
    if not path.is_file():
        return None
    data: dict[str, Any] = json.loads(path.read_text(encoding="utf-8"))
    if data.get("format") != SIDECAR_FORMAT:
        raise ValueError(
            f"{path}: unknown sidecar format {data.get('format')!r}; "
            f"expected {SIDECAR_FORMAT!r}"
        )
    return data


def suite_name(binary: str) -> str:
    """'beman.transcode.benchmarks.utf' → 'utf'."""
    return binary.rsplit(".", 1)[-1] if binary else ""


def throughput_MiBs(input_bytes: int, mean_ns: float) -> float:
    """Compute throughput in MiB/s given input size and mean latency."""
    if mean_ns <= 0:
        return 0.0
    return input_bytes / (mean_ns * 1e-9) / MIB


def flags_for(m: Measurement) -> list[str]:
    """The flags that apply to a measurement, in FLAG_TEXT order."""
    flags: list[str] = []
    if m.input_bytes is None:
        flags.append("no-metadata")
    elif m.input_bytes == 0 or m.input_source == "missing":
        flags.append("empty-input")
    elif (
        m.result.mean_ns < IMPLAUSIBLE_MEAN_NS
        or m.input_bytes / m.result.mean_ns > IMPLAUSIBLE_BYTES_PER_NS
    ):
        flags.append("implausible")
    if m.result.outlier_variance > NOISY_OUTLIER_VARIANCE:
        flags.append("noisy")
    return flags


def load_run(xml_path: Path) -> Run:
    """Parse one XML file, join it with its sidecar, and flag each result."""
    results = parse_catch2_xml(xml_path)
    sidecar = load_sidecar(sidecar_path(xml_path))
    meta: dict[str, dict[str, Any]] = {}
    environment: dict[str, Any] = {}
    if sidecar is not None:
        environment = dict(sidecar.get("environment", {}))
        meta = {b["name"]: b for b in sidecar.get("benchmarks", [])}

    binary = results[0].binary if results else xml_path.stem
    suite = suite_name(binary)
    measurements: list[Measurement] = []
    for r in results:
        entry = meta.get(r.name, {})
        m = Measurement(
            result=r,
            suite=suite,
            conversion=entry.get("conversion", ""),
            library=entry.get("library", ""),
            api=entry.get("api", ""),
            input=entry.get("input", ""),
            input_source=entry.get("input_source", ""),
            input_bytes=entry.get("input_bytes"),
        )
        m.flags = flags_for(m)
        measurements.append(m)
    return Run(xml_path, suite, environment, measurements, sidecar is not None)


# ── Grouping and formatting ─────────────────────────────────────────────────


def ordered_libraries(libraries: Iterable[str]) -> list[str]:
    """Distinct libraries, known ones in LIBRARY_ORDER, then the rest sorted."""
    present = set(libraries)
    known = [lib for lib in LIBRARY_ORDER if lib in present]
    return known + sorted(present - set(LIBRARY_ORDER))


def library_slot(library: str) -> int | None:
    """1-based categorical slot for a library, or None for 'other'."""
    try:
        return LIBRARY_ORDER.index(library) + 1
    except ValueError:
        return None


def group_by_conversion(measurements: list[Measurement]) -> list[ConversionGroup]:
    """Group by conversion, then input.

    Conversions compared across libraries come first, then those with more
    rows; within a conversion, inputs run small to large and rows follow
    library order, fastest first.
    """
    by_conv: dict[str, dict[str, list[Measurement]]] = {}
    for m in measurements:
        conv = m.conversion or "(not described)"
        key = m.input or m.name
        by_conv.setdefault(conv, {}).setdefault(key, []).append(m)

    def row_key(m: Measurement) -> tuple[int, float]:
        slot = library_slot(m.library)
        return (slot if slot is not None else 99, -(m.throughput or 0.0))

    groups: list[ConversionGroup] = []
    for conv, inputs in by_conv.items():
        input_groups = [
            InputGroup(
                input=key,
                input_bytes=rows[0].input_bytes,
                input_source=rows[0].input_source,
                rows=sorted(rows, key=row_key),
            )
            for key, rows in inputs.items()
        ]
        input_groups.sort(key=lambda g: (g.input_bytes or 0, g.input))
        groups.append(ConversionGroup(conv, input_groups))

    groups.sort(key=lambda g: (-len(g.libraries), -len(g.measurements), g.conversion))
    return groups


def row_label(m: Measurement, group: InputGroup) -> str:
    """The API, plus the suite when two rows in one input group share it."""
    label = m.api or m.name
    twins = [r for r in group.rows if (r.library, r.api) == (m.library, m.api)]
    if len(twins) > 1:
        label += f" ({m.suite})"
    return label


def fmt_bytes(n: int | None) -> str:
    if n is None:
        return "?"
    if n >= MIB:
        return f"{n / MIB:.1f} MiB"
    if n >= 10 * 1024:
        return f"{n / 1024:.0f} KiB"
    return f"{n:,} B"


def fmt_time(ns: float) -> str:
    """Format a duration with a unit that keeps 3–4 significant figures."""
    for unit, scale in (("s", 1e9), ("ms", 1e6), ("µs", 1e3)):
        if ns >= scale:
            return f"{ns / scale:.4g} {unit}"
    return f"{ns:.4g} ns"


def fmt_throughput(t: float) -> str:
    if t >= 100:
        return f"{t:,.0f}"
    if t >= 10:
        return f"{t:.1f}"
    return f"{t:.2f}"


def fmt_tick(t: float) -> str:
    if t >= 1000:
        return f"{t / 1000:g}K"
    return f"{t:g}"


def input_label(group: InputGroup) -> str:
    source = INPUT_SOURCE_TEXT.get(group.input_source, group.input_source)
    size = fmt_bytes(group.input_bytes)
    return f"{group.input} · {size}" + (f" · {source}" if source else "")


def slugify(text: str) -> str:
    """'Shift_JIS → UTF-8' → 'shift-jis-to-utf-8'."""
    text = text.replace("→", " to ").lower()
    return re.sub(r"[^a-z0-9]+", "-", text).strip("-") or "unnamed"


def environment_summary(runs: list[Run]) -> list[tuple[str, str]]:
    """Key/value rows describing the runs; differing values are joined."""
    keys = [
        ("compiler", "Compiler"),
        ("build_config", "Build config"),
        ("cpu", "CPU"),
        ("host", "Host"),
        ("catch2", "Catch2"),
    ]
    rows: list[tuple[str, str]] = []
    for key, title in keys:
        values = sorted({str(r.environment.get(key, "")) for r in runs} - {""})
        if values:
            rows.append((title, "; ".join(values)))
    stamps = sorted({str(r.environment.get("timestamp", "")) for r in runs} - {""})
    if stamps:
        rows.append(
            ("Run at", stamps[0] if len(stamps) == 1 else f"{stamps[0]} – {stamps[-1]}")
        )
    rows.append(("Suites", ", ".join(r.suite for r in runs)))
    return rows


def run_warnings(runs: list[Run]) -> list[str]:
    """Whole-report problems: builds unfit for timing, missing sidecars."""
    warnings: list[str] = []
    for r in runs:
        env = r.environment
        if not r.has_sidecar:
            warnings.append(
                f"{r.path.name}: no {sidecar_path(r.path).name} beside it, so "
                "input sizes are unknown; rerun the benchmark with --out to "
                "write one."
            )
            continue
        if env.get("sanitizers"):
            warnings.append(
                f"{r.suite}: built with {', '.join(env['sanitizers'])} "
                "sanitizer; timings are not meaningful."
            )
        elif env.get("optimized") is False:
            warnings.append(f"{r.suite}: built without optimization.")
    compilers = {r.environment.get("compiler") for r in runs if r.has_sidecar}
    if len(compilers) > 1:
        warnings.append(
            "Suites were built with different compilers; compare across them with care."
        )
    return warnings


def flag_counts(measurements: list[Measurement]) -> dict[str, int]:
    counts = {f: 0 for f in FLAG_TEXT}
    for m in measurements:
        for f in m.flags:
            counts[f] += 1
    return {f: n for f, n in counts.items() if n}


# ── Markdown ────────────────────────────────────────────────────────────────


def _md_cell(text: str) -> str:
    return text.replace("|", "\\|")


def render_markdown(runs: list[Run], label: str = "") -> str:
    """Markdown report: environment, warnings, one table per conversion."""
    measurements = [m for r in runs for m in r.measurements]
    heading = "Benchmark Results" + (f" — {label}" if label else "")
    lines = [f"## {heading}", ""]
    for key, value in environment_summary(runs):
        lines.append(f"- **{key}:** {_md_cell(value)}")
    lines.append("")
    for w in run_warnings(runs):
        lines.append(f"> [!WARNING]\n> {w}\n")

    for group in group_by_conversion(measurements):
        lines += [
            f"### {_md_cell(group.conversion)}",
            "",
            "| Input | Library | API | Mean | 95% CI | MiB/s | Notes |",
            "|-------|---------|-----|-----:|-------:|------:|-------|",
        ]
        for ig in group.inputs:
            for m in ig.rows:
                r = m.result
                ci = f"{fmt_time(r.low_ns)} – {fmt_time(r.high_ns)}"
                tput = m.throughput
                cells = [
                    f"{ig.input} ({fmt_bytes(ig.input_bytes)})",
                    m.library or "?",
                    row_label(m, ig),
                    fmt_time(r.mean_ns),
                    ci,
                    fmt_throughput(tput) if tput is not None else "—",
                    ", ".join(m.flags),
                ]
                lines.append("| " + " | ".join(_md_cell(c) for c in cells) + " |")
        lines.append("")

    counts = flag_counts(measurements)
    if counts:
        lines.append("Notes:")
        lines.append("")
        for f, n in counts.items():
            lines.append(f"- `{f}` ({n}): {FLAG_TEXT[f]}")
        lines.append("")
    lines.append(
        "> Throughput = input bytes / mean time; 1 MiB = 2²⁰ bytes; the CI is "
        "Catch2's 95% bootstrap interval on the mean."
    )
    return "\n".join(lines) + "\n"


# ── SVG charts ──────────────────────────────────────────────────────────────

# Light-mode values, used by standalone SVGs (paper figures print on white).
# The HTML report defines the same custom properties per theme.
LIGHT_TOKENS = {
    "--surface": "#fcfcfb",
    "--ink": "#0b0b0b",
    "--ink-2": "#52514e",
    "--muted": "#898781",
    "--grid": "#e1e0d9",
    "--axis": "#c3c2b7",
    "--lib-1": "#2a78d6",
    "--lib-2": "#eb6834",
    "--lib-3": "#1baf7a",
    "--lib-4": "#eda100",
    "--lib-5": "#e87ba4",
    "--lib-other": "#898781",
}
DARK_TOKENS = {
    "--surface": "#1a1a19",
    "--ink": "#f0efec",
    "--ink-2": "#c3c2b7",
    "--muted": "#898781",
    "--grid": "#2c2c2a",
    "--axis": "#383835",
    "--lib-1": "#3987e5",
    "--lib-2": "#d95926",
    "--lib-3": "#199e70",
    "--lib-4": "#c98500",
    "--lib-5": "#d55181",
    "--lib-other": "#898781",
}

CHART_CSS = (
    """
.bt-chart { font-family: system-ui, -apple-system, "Segoe UI", sans-serif; }
.bt-chart text { fill: var(--ink-2); font-size: 12px; }
.bt-chart .bt-title { fill: var(--ink); font-size: 14px; font-weight: 600; }
.bt-chart .bt-input { fill: var(--ink); font-size: 12px; font-weight: 600; }
.bt-chart .bt-value { fill: var(--ink); font-variant-numeric: tabular-nums; }
.bt-chart .bt-flag { fill: var(--muted); font-style: italic; }
.bt-chart .bt-tick { fill: var(--muted); font-size: 11px;
  font-variant-numeric: tabular-nums; }
.bt-chart .bt-grid { stroke: var(--grid); stroke-width: 1; }
.bt-chart .bt-axis { stroke: var(--axis); stroke-width: 1; }
.bt-chart .bt-ci { stroke-width: 2; stroke-linecap: round; }
.bt-chart .bt-mark { stroke: var(--surface); stroke-width: 2; }
.bt-chart .bt-hit { fill: transparent; }
.bt-chart .bt-row:hover .bt-hit { fill: var(--grid); fill-opacity: 0.5; }
"""
    + "".join(
        f".bt-chart .bt-mark.lib-{i} {{ fill: var(--lib-{i}); }}\n"
        f".bt-chart .bt-ci.lib-{i} {{ stroke: var(--lib-{i}); }}\n"
        for i in range(1, 6)
    )
    + (
        ".bt-chart .bt-mark.lib-other { fill: none; stroke: var(--lib-other); }\n"
        ".bt-chart .bt-ci.lib-other { stroke: var(--lib-other); }\n"
    )
)


def _resolve_tokens(css: str, tokens: dict[str, str]) -> str:
    """Replace var(--x) with its value: SVG renderers outside browsers (rsvg,
    Inkscape, the LaTeX toolchain) do not implement custom properties."""
    return re.sub(r"var\((--[a-z0-9-]+)\)", lambda m: tokens[m.group(1)], css)


def _lib_class(library: str) -> str:
    slot = library_slot(library)
    return f"lib-{slot}" if slot is not None else "lib-other"


def _marker(shape: str, x: float, y: float, cls: str) -> str:
    r = 5.0
    if shape == "square":
        return (
            f'<rect class="bt-mark {cls}" x="{x - r:.1f}" y="{y - r:.1f}" '
            f'width="{2 * r:.1f}" height="{2 * r:.1f}" rx="1"/>'
        )
    if shape in ("diamond", "triangle", "triangle-down"):
        if shape == "diamond":
            d = r * 1.3
            pts = [(x, y - d), (x + d, y), (x, y + d), (x - d, y)]
        elif shape == "triangle":
            pts = [(x, y - r * 1.2), (x + r * 1.2, y + r), (x - r * 1.2, y + r)]
        else:
            pts = [(x, y + r * 1.2), (x + r * 1.2, y - r), (x - r * 1.2, y - r)]
        p = " ".join(f"{px:.1f},{py:.1f}" for px, py in pts)
        return f'<polygon class="bt-mark {cls}" points="{p}"/>'
    return f'<circle class="bt-mark {cls}" cx="{x:.1f}" cy="{y:.1f}" r="{r}"/>'


def _library_shape(library: str) -> str:
    slot = library_slot(library)
    return LIBRARY_SHAPES[slot - 1] if slot is not None else "circle"


def log_domain(measurements: Iterable[Measurement]) -> tuple[float, float]:
    """Decade-aligned log10 domain covering every plotted CI, at least one
    decade wide."""
    lo, hi = math.inf, -math.inf
    for m in measurements:
        ci = m.throughput_ci
        if ci is None:
            continue
        lo, hi = min(lo, ci[0]), max(hi, ci[1])
    if lo == math.inf or lo <= 0:
        return (1.0, 4.0)
    d0, d1 = math.floor(math.log10(lo)), math.ceil(math.log10(hi))
    return (d0, max(d1, d0 + 1))


def _truncate(text: str, max_chars: int) -> str:
    return text if len(text) <= max_chars else text[: max_chars - 1] + "…"


WIDTH = 760
LABEL_X = 16
PLOT_X0 = 300
PLOT_X1 = 640
VALUE_X = 652
ROW_H = 22
INPUT_H = 26


def render_svg_chart(group: ConversionGroup, standalone: bool = False) -> str:
    """One dot-and-whisker chart for a conversion, throughput on a log axis.

    Each row is one benchmark: the mark is the mean, the whisker the 95% CI.
    Rows are grouped under their input.  Excluded results keep their row with
    the reason in place of a value, so nothing silently disappears.
    """
    d0, d1 = log_domain(group.measurements)
    span = max(d1 - d0, 1.0)

    def x_of(t: float) -> float:
        return PLOT_X0 + (math.log10(t) - d0) / span * (PLOT_X1 - PLOT_X0)

    libraries = group.libraries
    body: list[str] = []
    title = html.escape(group.conversion)
    # A standalone figure carries its own title; in the HTML report the
    # section heading above the chart already names it.
    top = 30.0 if standalone else 0.0
    if standalone:
        body.append(f'<text class="bt-title" x="{LABEL_X}" y="20">{title}</text>')

    if len(libraries) > 1:
        lx = float(LABEL_X)
        ly = top + 12
        for lib in libraries:
            cls = _lib_class(lib)
            body.append(_marker(_library_shape(lib), lx + 5, ly - 4, cls))
            body.append(
                f'<text x="{lx + 15:.0f}" y="{ly:.0f}">{html.escape(lib)}</text>'
            )
            lx += 30 + 7 * len(lib)
        top += 24

    y = top + 6
    rows_svg: list[str] = []
    for ig in group.inputs:
        y += INPUT_H
        rows_svg.append(
            f'<text class="bt-input" x="{LABEL_X}" y="{y - 8:.0f}">'
            f"{html.escape(input_label(ig))}</text>"
        )
        for m in ig.rows:
            cy = y + ROW_H / 2
            label = row_label(m, ig)
            tip = _tooltip(m)
            cls = _lib_class(m.library)
            parts = [
                f'<g class="bt-row" data-tip="{html.escape(tip, quote=True)}">',
                f"<title>{html.escape(tip)}</title>",
                f'<rect class="bt-hit" x="0" y="{y:.0f}" width="{WIDTH}" '
                f'height="{ROW_H}"/>',
                f'<text x="{LABEL_X + 12}" y="{cy + 4:.1f}">'
                f"{html.escape(_truncate(label, 40))}</text>",
            ]
            ci = m.throughput_ci
            tput = m.throughput
            if ci is not None and tput is not None and tput > 0:
                parts.append(
                    f'<line class="bt-ci {cls}" x1="{x_of(ci[0]):.1f}" '
                    f'y1="{cy:.1f}" x2="{x_of(ci[1]):.1f}" y2="{cy:.1f}"/>'
                )
                parts.append(_marker(_library_shape(m.library), x_of(tput), cy, cls))
                parts.append(
                    f'<text class="bt-value" x="{VALUE_X}" y="{cy + 4:.1f}">'
                    f"{fmt_throughput(tput)}</text>"
                )
            else:
                reason = next(f for f in m.flags if f in EXCLUDED_FLAGS)
                parts.append(
                    f'<text class="bt-flag" x="{PLOT_X0}" y="{cy + 4:.1f}">'
                    f"not plotted: {html.escape(FLAG_TEXT[reason])}</text>"
                )
            parts.append("</g>")
            rows_svg.append("".join(parts))
            y += ROW_H

    plot_top, plot_bottom = top + 6, y + 4
    grid: list[str] = []
    # Decades always; 2× and 5× too when the axis is narrow enough to need them.
    steps = (1, 2, 5) if span <= 2 else (1,)
    for decade in range(int(d0), int(d1) + 1):
        for step in steps:
            t = step * 10.0**decade
            if math.log10(t) > d1 + 1e-9:
                break
            gx = x_of(t)
            grid.append(
                f'<line class="bt-grid" x1="{gx:.1f}" y1="{plot_top:.0f}" '
                f'x2="{gx:.1f}" y2="{plot_bottom:.0f}"/>'
            )
            grid.append(
                f'<text class="bt-tick" x="{gx:.1f}" y="{plot_bottom + 16:.0f}" '
                f'text-anchor="middle">{fmt_tick(t)}</text>'
            )
    grid.append(
        f'<line class="bt-axis" x1="{PLOT_X0}" y1="{plot_bottom:.0f}" '
        f'x2="{PLOT_X1}" y2="{plot_bottom:.0f}"/>'
    )
    grid.append(
        f'<text class="bt-tick" x="{(PLOT_X0 + PLOT_X1) / 2:.0f}" '
        f'y="{plot_bottom + 34:.0f}" text-anchor="middle">'
        "throughput, MiB/s (log scale) · whisker = 95% CI</text>"
    )
    grid.append(
        f'<text class="bt-tick" x="{VALUE_X}" y="{plot_top + 2:.0f}">MiB/s</text>'
    )
    height = plot_bottom + 46

    style = ""
    if standalone:
        style = (
            "<style>"
            + _resolve_tokens(CHART_CSS, LIGHT_TOKENS)
            + ".bt-chart .bt-row:hover .bt-hit { fill: none; }"
            + "</style>"
            + f'<rect width="{WIDTH}" height="{height:.0f}" '
            + f'fill="{LIGHT_TOKENS["--surface"]}"/>'
        )
    head = (
        f'<svg xmlns="http://www.w3.org/2000/svg" class="bt-chart" '
        f'viewBox="0 0 {WIDTH} {height:.0f}" width="{WIDTH}" '
        f'height="{height:.0f}" role="img" aria-label="{title} throughput">'
    )
    return head + style + "".join(grid + body + rows_svg) + "</svg>"


def _tooltip(m: Measurement) -> str:
    r = m.result
    lines = [
        m.name,
        f"{m.library or '?'} · {m.api or '?'} · suite {m.suite}",
        f"mean {fmt_time(r.mean_ns)} (95% CI {fmt_time(r.low_ns)} – "
        f"{fmt_time(r.high_ns)}), σ {fmt_time(r.std_dev_ns)}",
    ]
    ci = m.throughput_ci
    if ci is not None and m.throughput is not None:
        lines.append(
            f"{fmt_throughput(m.throughput)} MiB/s "
            f"({fmt_throughput(ci[0])} – {fmt_throughput(ci[1])})"
        )
    lines.append(f"{r.samples} samples × {r.iterations} iterations")
    for f in m.flags:
        lines.append(f"⚠ {f}: {FLAG_TEXT[f]}")
    return "\n".join(lines)


def write_svg_dir(runs: list[Run], out_dir: Path) -> list[Path]:
    """Write one standalone SVG per conversion; return the paths written."""
    measurements = [m for r in runs for m in r.measurements]
    groups = group_by_conversion(measurements)
    out_dir.mkdir(parents=True, exist_ok=True)
    written: list[Path] = []
    for g in groups:
        path = out_dir / f"{slugify(g.conversion)}.svg"
        path.write_text(render_svg_chart(g, standalone=True) + "\n", encoding="utf-8")
        written.append(path)
    return written


# ── HTML ────────────────────────────────────────────────────────────────────

PAGE_CSS = """
:root {
  color-scheme: light;
  --page: #f9f9f7;
  --border: rgba(11,11,11,0.10);
  --warn: #fab219; --critical: #d03b3b;
  %(light)s
}
@media (prefers-color-scheme: dark) {
  :root:not([data-theme="light"]) { color-scheme: dark; --page: #0d0d0d;
    --border: rgba(255,255,255,0.10); %(dark)s }
}
:root[data-theme="dark"] { color-scheme: dark; --page: #0d0d0d;
  --border: rgba(255,255,255,0.10); %(dark)s }
body { background: var(--page); color: var(--ink);
  font: 14px/1.45 system-ui, -apple-system, "Segoe UI", sans-serif; }
main { max-width: 820px; margin: 0 auto; padding: 24px 16px 48px; }
h1 { font-size: 22px; margin: 0 0 4px; }
h2 { font-size: 16px; margin: 32px 0 8px; }
.sub { color: var(--ink-2); margin: 0 0 16px; }
dl.env { display: grid; grid-template-columns: max-content 1fr; gap: 2px 16px;
  margin: 0 0 16px; color: var(--ink-2); }
dl.env dt { color: var(--muted); }
dl.env dd { margin: 0; overflow-wrap: anywhere; }
.callout { border: 1px solid var(--border); border-left: 4px solid var(--warn);
  background: var(--surface); padding: 8px 12px; margin: 8px 0;
  border-radius: 4px; }
.callout.critical { border-left-color: var(--critical); }
.callout strong { color: var(--ink); }
.card { background: var(--surface); border: 1px solid var(--border);
  border-radius: 8px; padding: 8px 0; margin: 12px 0; }
.scroll { overflow-x: auto; }
.card svg { display: block; max-width: 100%%; min-width: 600px; height: auto; }
.legend { display: flex; flex-wrap: wrap; gap: 4px 16px; color: var(--ink-2); }
.legend svg { vertical-align: middle; }
table { border-collapse: collapse; width: 100%%; font-size: 13px; }
th, td { text-align: left; padding: 4px 8px; border-bottom: 1px solid var(--grid);
  vertical-align: top; }
th { color: var(--muted); font-weight: 600; position: sticky; top: 0;
  background: var(--surface); }
td.num { text-align: right; font-variant-numeric: tabular-nums;
  white-space: nowrap; }
td.flags { color: var(--ink-2); }
nav.toc { columns: 2 240px; color: var(--ink-2); }
nav.toc a { color: inherit; }
#bt-tip { position: fixed; pointer-events: none; white-space: pre-line;
  background: var(--surface); color: var(--ink); border: 1px solid var(--border);
  border-radius: 6px; padding: 6px 8px; font-size: 12px; max-width: 360px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.15); }
"""

PAGE_JS = """
(() => {
  const tip = document.getElementById('bt-tip');
  document.addEventListener('mousemove', (e) => {
    const row = e.target.closest && e.target.closest('.bt-row');
    if (!row) { tip.hidden = true; return; }
    tip.textContent = row.dataset.tip;
    tip.hidden = false;
    const pad = 14, w = tip.offsetWidth, h = tip.offsetHeight;
    let x = e.clientX + pad, y = e.clientY + pad;
    if (x + w > innerWidth - 8) x = e.clientX - w - pad;
    if (y + h > innerHeight - 8) y = e.clientY - h - pad;
    tip.style.left = x + 'px'; tip.style.top = y + 'px';
  });
  // The native <title> tooltip would double up with this one.
  document.querySelectorAll('.bt-row > title').forEach((t) => t.remove());
})();
"""


def _legend_html(libraries: list[str]) -> str:
    items = []
    for lib in libraries:
        mark = _marker(_library_shape(lib), 8, 8, _lib_class(lib))
        items.append(
            '<span><svg class="bt-chart" width="16" height="16" '
            f'aria-hidden="true">{mark}</svg> {html.escape(lib)}</span>'
        )
    return '<div class="legend">' + "".join(items) + "</div>"


def render_html(runs: list[Run], label: str = "") -> str:
    """A self-contained HTML report: no external scripts, styles or fonts."""
    measurements = [m for r in runs for m in r.measurements]
    groups = group_by_conversion(measurements)
    e = html.escape

    def tokens(t: dict[str, str]) -> str:
        return " ".join(f"{k}: {v};" for k, v in t.items())

    css = PAGE_CSS % {"light": tokens(LIGHT_TOKENS), "dark": tokens(DARK_TOKENS)}
    out = [
        "<title>transcode benchmarks</title>",
        f"<style>{css}{CHART_CSS}</style>",
        "<main>",
        "<h1>beman.transcode benchmarks</h1>",
        f'<p class="sub">{e(label)}</p>' if label else "",
        '<dl class="env">',
    ]
    for key, value in environment_summary(runs):
        out.append(f"<dt>{e(key)}</dt><dd>{e(value)}</dd>")
    out.append("</dl>")

    for w in run_warnings(runs):
        out.append(
            f'<div class="callout critical"><strong>⚠ Warning</strong> {e(w)}</div>'
        )
    counts = flag_counts(measurements)
    if counts:
        items = "; ".join(f"{n} {e(f)} ({e(FLAG_TEXT[f])})" for f, n in counts.items())
        out.append(
            f'<div class="callout"><strong>⚠ Flagged results</strong> {items}. '
            "Excluded results keep their row in the charts with the reason in "
            "place of a value.</div>"
        )

    out.append(_legend_html(ordered_libraries(m.library for m in measurements)))
    out.append('<nav class="toc"><ul>')
    for g in groups:
        out.append(
            f'<li><a href="#{slugify(g.conversion)}">{e(g.conversion)}</a> '
            f"({len(g.measurements)})</li>"
        )
    out.append('<li><a href="#all-results">All results</a></li></ul></nav>')

    for g in groups:
        out.append(f'<h2 id="{slugify(g.conversion)}">{e(g.conversion)}</h2>')
        out.append(f'<div class="card scroll">{render_svg_chart(g)}</div>')

    out.append('<h2 id="all-results">All results</h2><div class="card scroll"><table>')
    out.append(
        "<thead><tr><th>Conversion</th><th>Input</th><th>Library</th><th>API</th>"
        "<th>Mean</th><th>95% CI</th><th>MiB/s</th><th>Notes</th>"
        "<th>Benchmark</th><th>Suite</th></tr></thead><tbody>"
    )
    for g in groups:
        for ig in g.inputs:
            for m in ig.rows:
                r = m.result
                tput = m.throughput
                out.append(
                    "<tr>"
                    f"<td>{e(g.conversion)}</td>"
                    f"<td>{e(ig.input)} ({e(fmt_bytes(ig.input_bytes))})</td>"
                    f"<td>{e(m.library or '?')}</td><td>{e(m.api or '?')}</td>"
                    f'<td class="num">{e(fmt_time(r.mean_ns))}</td>'
                    f'<td class="num">{e(fmt_time(r.low_ns))} – '
                    f"{e(fmt_time(r.high_ns))}</td>"
                    f'<td class="num">'
                    f"{e(fmt_throughput(tput)) if tput is not None else '—'}</td>"
                    f'<td class="flags">{e(", ".join(m.flags))}</td>'
                    f"<td>{e(m.name)}</td><td>{e(m.suite)}</td>"
                    "</tr>"
                )
    out.append("</tbody></table></div>")
    out.append(
        '<p class="sub">Throughput = input bytes / mean time; 1 MiB = 2²⁰ '
        "bytes. Whiskers and the CI column are Catch2's 95% bootstrap interval "
        "on the mean.</p>"
    )
    out.append("</main>")
    out.append('<div id="bt-tip" hidden></div>')
    out.append(f"<script>{PAGE_JS}</script>")
    return "\n".join(s for s in out if s) + "\n"


# ── Vega-Lite ───────────────────────────────────────────────────────────────


def render_vegalite_spec(
    runs: list[Run], title: str = "Benchmark Throughput"
) -> dict[str, Any]:
    """Vega-Lite spec: mean and CI per benchmark, faceted by conversion.

    Excluded results are omitted.
    """
    values: list[dict[str, object]] = []
    for r in runs:
        for m in r.measurements:
            tput, ci = m.throughput, m.throughput_ci
            if tput is None or ci is None:
                continue
            values.append(
                {
                    "benchmark": m.name,
                    "conversion": m.conversion,
                    "input": m.input,
                    "library": m.library,
                    "api": m.api,
                    "throughput_MiBs": round(tput, 2),
                    "ci_low": round(ci[0], 2),
                    "ci_high": round(ci[1], 2),
                }
            )
    x = {
        "field": "throughput_MiBs",
        "type": "quantitative",
        "scale": {"type": "log"},
        "title": "Throughput (MiB/s)",
    }
    y = {"field": "benchmark", "type": "nominal", "title": None, "sort": "-x"}
    color = {"field": "library", "type": "nominal", "scale": {"domain": LIBRARY_ORDER}}
    return {
        "$schema": "https://vega.github.io/schema/vega-lite/v5.json",
        "title": title,
        "data": {"values": values},
        "facet": {"row": {"field": "conversion", "type": "nominal"}},
        "resolve": {"scale": {"y": "independent"}},
        "spec": {
            "layer": [
                {
                    "mark": "rule",
                    "encoding": {
                        "x": {**x, "field": "ci_low"},
                        "x2": {"field": "ci_high"},
                        "y": y,
                        "color": color,
                    },
                },
                {
                    "mark": {"type": "point", "filled": True},
                    "encoding": {
                        "x": x,
                        "y": y,
                        "color": color,
                        "shape": {"field": "library", "type": "nominal"},
                        "tooltip": [
                            {"field": "benchmark"},
                            {"field": "api"},
                            {"field": "input"},
                            {"field": "throughput_MiBs", "title": "MiB/s"},
                            {"field": "ci_low", "title": "CI low"},
                            {"field": "ci_high", "title": "CI high"},
                        ],
                    },
                },
            ]
        },
    }


# ── CLI ─────────────────────────────────────────────────────────────────────


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Turn Catch2 benchmark XML into throughput reports and charts."
    )
    parser.add_argument(
        "xml_files",
        metavar="XML",
        nargs="+",
        type=Path,
        help="Catch2 XML result files; each FILE.xml is read with FILE.meta.json",
    )
    parser.add_argument(
        "--label", metavar="TEXT", default="", help="Label for report headings"
    )
    parser.add_argument(
        "--markdown",
        metavar="PATH",
        type=Path,
        default=None,
        help="Write the Markdown report here instead of stdout",
    )
    parser.add_argument(
        "--html", metavar="PATH", type=Path, default=None, help="Write an HTML report"
    )
    parser.add_argument(
        "--svg-dir",
        metavar="DIR",
        type=Path,
        default=None,
        help="Write one standalone SVG chart per conversion into DIR",
    )
    parser.add_argument(
        "--vegalite",
        metavar="PATH",
        type=Path,
        default=None,
        help="Write a Vega-Lite JSON chart spec to this file",
    )
    args = parser.parse_args(argv)

    runs: list[Run] = []
    for xml_path in args.xml_files:
        if not xml_path.is_file():
            print(f"error: {xml_path}: file not found", file=sys.stderr)
            return 1
        try:
            runs.append(load_run(xml_path))
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 1

    if not any(r.measurements for r in runs):
        print("No benchmark results found.", file=sys.stderr)
        return 1

    label = args.label or (args.xml_files[0].stem if len(args.xml_files) == 1 else "")

    markdown = render_markdown(runs, label=label)
    if args.markdown is None:
        print(markdown)
    else:
        _write(args.markdown, markdown)

    if args.html is not None:
        _write(args.html, render_html(runs, label=label))
    if args.svg_dir is not None:
        paths = write_svg_dir(runs, args.svg_dir)
        print(f"{len(paths)} SVG charts written to {args.svg_dir}", file=sys.stderr)
    if args.vegalite is not None:
        spec = render_vegalite_spec(runs, title=f"Throughput — {label}")
        _write(args.vegalite, json.dumps(spec, indent=2, ensure_ascii=False) + "\n")
    return 0


def _write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    print(f"wrote {path}", file=sys.stderr)


if __name__ == "__main__":
    sys.exit(main())
