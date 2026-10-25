# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ruff: noqa: E501
"""Tests for tools/process_benchmark_results.py."""

import json
import re
import sys
from collections.abc import Mapping
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from process_benchmark_results import (
    BUSY_LOAD,
    COUNT_NOTE,
    IMPLAUSIBLE_BYTES_PER_NS,
    REFERENCE_LIBRARY,
    SIDECAR_FORMAT,
    Measurement,
    Ratio,
    Run,
    assign_labels,
    comparison_groups,
    comparison_warnings,
    count_ratio,
    duplicate_warnings,
    environment_summary,
    fmt_count_ratio,
    fmt_ratio,
    group_by_conversion,
    library_ratio,
    load_suite,
    log_domain,
    main,
    match_runs,
    other_load,
    parse_callgrind_dump,
    parse_catch2_xml,
    ratio_axis,
    render_comparison_html,
    render_comparison_markdown,
    render_comparison_vegalite,
    render_count_chart,
    render_html,
    render_markdown,
    render_ratio_svg,
    render_svg_chart,
    render_vegalite_spec,
    repeat_order,
    run_table,
    sidecar_path,
    slugify,
    suite_list,
    suite_measurements,
    suite_name,
    suite_warnings,
    throughput_MiBs,
    throughput_ratio,
    xml_files,
)

# ---------------------------------------------------------------------------
# Minimal Catch2 XML fixture
# ---------------------------------------------------------------------------

SAMPLE_XML = """\
<?xml version="1.0" encoding="UTF-8"?>
<Catch2TestRun name="beman.transcode.benchmarks.smoke" rng-seed="42" xml-format-version="3" catch2-version="3.10.0" filters="[smoke]">
  <TestCase name="smoke benchmark" tags="[benchmark][smoke]" filename="smoke.bench.cpp" line="12">
    <BenchmarkResults name="whatwg_decode utf_8 en_mars corpus" samples="100" resamples="100000" iterations="6" clockResolution="20.0" estimatedDuration="2000000">
      <!-- All values in nano seconds -->
      <mean value="3500.0" lowerBound="3400.0" upperBound="3700.0" ci="0.95"/>
      <standardDeviation value="100.0" lowerBound="50.0" upperBound="200.0" ci="0.95"/>
      <outliers variance="0.01" lowMild="0" lowSevere="0" highMild="1" highSevere="0"/>
    </BenchmarkResults>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <TestCase name="multi-corpus benchmark" tags="[benchmark][smoke]" filename="smoke.bench.cpp" line="20">
    <BenchmarkResults name="UTF-8 decode: Arabic (multibyte-heavy)" samples="50" resamples="50000" iterations="4" clockResolution="20.0" estimatedDuration="1000000">
      <mean value="5000.0" lowerBound="4800.0" upperBound="5200.0" ci="0.95"/>
      <standardDeviation value="200.0" lowerBound="100.0" upperBound="300.0" ci="0.95"/>
      <outliers variance="0.02" lowMild="0" lowSevere="0" highMild="0" highSevere="0"/>
    </BenchmarkResults>
    <BenchmarkResults name="Single-byte decode: windows-1251 Russian" samples="50" resamples="50000" iterations="4" clockResolution="20.0" estimatedDuration="1000000">
      <mean value="2000.0" lowerBound="1900.0" upperBound="2100.0" ci="0.95"/>
      <standardDeviation value="80.0" lowerBound="40.0" upperBound="120.0" ci="0.95"/>
      <outliers variance="0.01" lowMild="0" lowSevere="0" highMild="0" highSevere="0"/>
    </BenchmarkResults>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <TestCase name="suspicious" tags="[benchmark][smoke]" filename="smoke.bench.cpp" line="30">
    <BenchmarkResults name="optimized away" samples="50" resamples="50000" iterations="4000" clockResolution="20.0" estimatedDuration="1000000">
      <mean value="0.27" lowerBound="0.26" upperBound="0.28" ci="0.95"/>
      <standardDeviation value="0.01" lowerBound="0.005" upperBound="0.02" ci="0.95"/>
      <outliers variance="0.9" lowMild="0" lowSevere="0" highMild="0" highSevere="3"/>
    </BenchmarkResults>
    <BenchmarkResults name="missing corpus" samples="50" resamples="50000" iterations="4000" clockResolution="20.0" estimatedDuration="1000000">
      <mean value="0.44" lowerBound="0.43" upperBound="0.45" ci="0.95"/>
      <standardDeviation value="0.01" lowerBound="0.005" upperBound="0.02" ci="0.95"/>
      <outliers variance="0.0" lowMild="0" lowSevere="0" highMild="0" highSevere="0"/>
    </BenchmarkResults>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <OverallResults successes="0" failures="0" expectedFailures="0" skips="0"/>
  <OverallResultsCases successes="3" failures="0" expectedFailures="0" skips="0"/>
</Catch2TestRun>
"""

UTF = "UTF-8 → UTF-32"
SAMPLE_SIDECAR = {
    "format": SIDECAR_FORMAT,
    "environment": {
        "timestamp": "2026-10-24T00:00:00Z",
        "host": "testhost",
        "cpu": "Test CPU",
        "compiler": "gcc 16.0.1",
        "build_config": "Perf",
        "optimized": True,
        "sanitizers": [],
        "catch2": "3.14.0",
    },
    "benchmarks": [
        {
            "name": "whatwg_decode utf_8 en_mars corpus",
            "conversion": UTF,
            "library": "beman.transcode",
            "api": "whatwg_decode view",
            "input": "en_mars_utf8.txt",
            "input_source": "corpus",
            "input_bytes": 586,
        },
        {
            "name": "UTF-8 decode: Arabic (multibyte-heavy)",
            "conversion": UTF,
            "library": "iconv",
            "api": "iconv_transcode_view",
            "input": "en_mars_utf8.txt",
            "input_source": "corpus",
            "input_bytes": 586,
        },
        {
            "name": "Single-byte decode: windows-1251 Russian",
            "conversion": "windows-1251 → UTF-32",
            "library": "beman.transcode",
            "api": "whatwg_decode view",
            "input": "ru_mars_windows1251.bin",
            "input_source": "corpus",
            "input_bytes": 435,
        },
        {
            "name": "optimized away",
            "conversion": "windows-1251 → UTF-32",
            "library": "beman.transcode",
            "api": "whatwg_decode view",
            "input": "4K × 0xC0",
            "input_source": "synthetic",
            "input_bytes": 4096,
        },
        {
            "name": "missing corpus",
            "conversion": "Shift_JIS → UTF-32",
            "library": "beman.transcode",
            "api": "whatwg_decode view",
            "input": "genji_monogatari_shiftjis.bin",
            "input_source": "missing",
            "input_bytes": 0,
        },
    ],
}

UNKNOWN_CORPUS_XML = """\
<?xml version="1.0" encoding="UTF-8"?>
<Catch2TestRun name="beman.transcode.benchmarks.smoke" rng-seed="1" xml-format-version="3" catch2-version="3.10.0">
  <TestCase name="tc" tags="[benchmark]" filename="x.cpp" line="1">
    <BenchmarkResults name="mystery benchmark with no corpus hint" samples="10" resamples="1000" iterations="1" clockResolution="1.0" estimatedDuration="100000">
      <mean value="1000.0" lowerBound="900.0" upperBound="1100.0" ci="0.95"/>
      <standardDeviation value="50.0" lowerBound="25.0" upperBound="75.0" ci="0.95"/>
      <outliers variance="0.0" lowMild="0" lowSevere="0" highMild="0" highSevere="0"/>
    </BenchmarkResults>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <OverallResults successes="0" failures="0" expectedFailures="0" skips="0"/>
  <OverallResultsCases successes="1" failures="0" expectedFailures="0" skips="0"/>
</Catch2TestRun>
"""


@pytest.fixture
def sample_xml_path(tmp_path: Path) -> Path:
    p = tmp_path / "smoke.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    return p


@pytest.fixture
def run_xml_path(tmp_path: Path) -> Path:
    """SAMPLE_XML with its sidecar beside it."""
    p = tmp_path / "smoke.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    sidecar_path(p).write_text(json.dumps(SAMPLE_SIDECAR), encoding="utf-8")
    return p


@pytest.fixture
def unknown_xml_path(tmp_path: Path) -> Path:
    p = tmp_path / "unknown.xml"
    p.write_text(UNKNOWN_CORPUS_XML, encoding="utf-8")
    return p


# ---------------------------------------------------------------------------
# parse_catch2_xml
# ---------------------------------------------------------------------------


def test_parse_returns_correct_count(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert len(results) == 5


def test_parse_binary_name(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].binary == "beman.transcode.benchmarks.smoke"


def test_parse_benchmark_name(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].name == "whatwg_decode utf_8 en_mars corpus"


def test_parse_mean_ns(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].mean_ns == pytest.approx(3500.0)


def test_parse_ci_bounds(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].low_ns == pytest.approx(3400.0)
    assert results[0].high_ns == pytest.approx(3700.0)


def test_parse_samples(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].samples == 100


def test_parse_iterations(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].iterations == 6


def test_parse_second_test_case_names(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    names = {r.name for r in results}
    assert "UTF-8 decode: Arabic (multibyte-heavy)" in names
    assert "Single-byte decode: windows-1251 Russian" in names


def test_parse_empty_xml_no_benchmarks(tmp_path: Path) -> None:
    p = tmp_path / "empty.xml"
    p.write_text(
        '<?xml version="1.0"?>'
        '<Catch2TestRun name="x"><OverallResults successes="0" failures="0"'
        ' expectedFailures="0" skips="0"/></Catch2TestRun>',
        encoding="utf-8",
    )
    assert parse_catch2_xml(p) == []


def test_parse_outlier_variance_and_std_dev(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert results[0].std_dev_ns == pytest.approx(100.0)
    assert results[3].outlier_variance == pytest.approx(0.9)


# ---------------------------------------------------------------------------
# throughput_MiBs
# ---------------------------------------------------------------------------


def test_throughput_known_value() -> None:
    # 1 MiB at 1 second = 1 MiB/s
    mib = 2**20
    result = throughput_MiBs(mib, 1e9)
    assert result == pytest.approx(1.0, rel=1e-6)


def test_throughput_double_corpus() -> None:
    mib = 2**20
    result = throughput_MiBs(2 * mib, 1e9)
    assert result == pytest.approx(2.0, rel=1e-6)


def test_throughput_zero_mean_returns_zero() -> None:
    assert throughput_MiBs(1000, 0.0) == 0.0


def test_throughput_small_corpus_fast_mean() -> None:
    # 586 bytes at 3500 ns ~ 159 MiB/s
    result = throughput_MiBs(586, 3500.0)
    assert result == pytest.approx(586 / (3500e-9) / 2**20, rel=1e-6)


# ---------------------------------------------------------------------------
# sidecar join and flags
# ---------------------------------------------------------------------------


def test_sidecar_path_replaces_extension() -> None:
    assert sidecar_path(Path("r/utf-Perf.xml")) == Path("r/utf-Perf.meta.json")


def test_suite_name_is_last_component() -> None:
    assert suite_name("beman.transcode.benchmarks.utf") == "utf"


def test_load_run_joins_sidecar(run_xml_path: Path) -> None:
    run = load_suite(run_xml_path)
    assert run.has_sidecar
    assert run.suite == "smoke"
    assert run.environment["compiler"] == "gcc 16.0.1"
    m = run.measurements[0]
    assert (m.conversion, m.library, m.api) == (
        UTF,
        "beman.transcode",
        "whatwg_decode view",
    )
    assert m.input_bytes == 586
    assert m.throughput == pytest.approx(throughput_MiBs(586, 3500.0))


def test_throughput_ci_uses_opposite_time_bounds(run_xml_path: Path) -> None:
    m = load_suite(run_xml_path).measurements[0]
    assert m.throughput_ci == pytest.approx(
        (throughput_MiBs(586, 3700.0), throughput_MiBs(586, 3400.0))
    )


def test_clean_results_have_no_flags(run_xml_path: Path) -> None:
    run = load_suite(run_xml_path)
    assert [m.flags for m in run.measurements[:3]] == [[], [], []]


def test_sub_nanosecond_result_is_implausible_and_noisy(run_xml_path: Path) -> None:
    m = load_suite(run_xml_path).measurements[3]
    assert m.flags == ["implausible", "noisy"]
    assert not m.plotted
    assert m.throughput is None and m.throughput_ci is None


def test_implausible_by_bytes_per_ns(tmp_path: Path) -> None:
    xml = SAMPLE_XML.replace(
        'value="3500.0" lowerBound="3400.0"', 'value="10.0" lowerBound="3400.0"'
    )
    p = tmp_path / "fast.xml"
    p.write_text(xml, encoding="utf-8")
    sidecar_path(p).write_text(json.dumps(SAMPLE_SIDECAR), encoding="utf-8")
    m = load_suite(p).measurements[0]
    assert IMPLAUSIBLE_BYTES_PER_NS < 586 / 10.0
    assert m.flags == ["implausible"]


def test_missing_corpus_is_empty_input(run_xml_path: Path) -> None:
    m = load_suite(run_xml_path).measurements[4]
    assert m.flags == ["empty-input"]
    assert m.throughput is None


def test_no_sidecar_flags_every_result(sample_xml_path: Path) -> None:
    run = load_suite(sample_xml_path)
    assert not run.has_sidecar
    assert all("no-metadata" in m.flags for m in run.measurements)
    assert all(m.throughput is None for m in run.measurements)


def test_unknown_sidecar_format_is_rejected(tmp_path: Path) -> None:
    p = tmp_path / "x.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    sidecar_path(p).write_text(json.dumps({"format": "other/9"}), encoding="utf-8")
    with pytest.raises(ValueError, match="unknown sidecar format"):
        load_suite(p)


# ---------------------------------------------------------------------------
# grouping
# ---------------------------------------------------------------------------


def test_group_compared_conversions_first(run_xml_path: Path) -> None:
    groups = group_by_conversion(load_suite(run_xml_path).measurements)
    assert groups[0].conversion == UTF
    assert groups[0].libraries == ["beman.transcode", "iconv"]


def test_group_inputs_run_small_to_large(run_xml_path: Path) -> None:
    groups = group_by_conversion(load_suite(run_xml_path).measurements)
    w1251 = next(g for g in groups if g.conversion == "windows-1251 → UTF-32")
    assert [g.input_bytes for g in w1251.inputs] == [435, 4096]


def test_log_domain_is_decade_aligned_and_ignores_excluded(run_xml_path: Path) -> None:
    run = load_suite(run_xml_path)
    assert log_domain(run.measurements) == (2, 3)


def test_log_domain_with_nothing_plotted(sample_xml_path: Path) -> None:
    assert log_domain(load_suite(sample_xml_path).measurements) == (1.0, 4.0)


def test_slugify_names_files_for_the_conversion() -> None:
    assert slugify("Shift_JIS → UTF-8") == "shift-jis-to-utf-8"
    assert slugify("mock iconv: E2BIG every byte") == "mock-iconv-e2big-every-byte"


# ---------------------------------------------------------------------------
# render_markdown
# ---------------------------------------------------------------------------


def test_markdown_has_environment_and_conversion_sections(run_xml_path: Path) -> None:
    md = render_markdown([load_suite(run_xml_path)], label="GCC Perf")
    assert "## Benchmark Results — GCC Perf" in md
    assert "**Compiler:** gcc 16.0.1" in md
    assert f"### {UTF}" in md
    assert "### windows-1251 → UTF-32" in md


def test_markdown_excluded_rows_show_dash_and_flag(run_xml_path: Path) -> None:
    md = render_markdown([load_suite(run_xml_path)])
    row = next(line for line in md.splitlines() if "4K × 0xC0" in line)
    assert "| — |" in row
    assert "implausible, noisy" in row
    assert "- `implausible` (1):" in md


def test_markdown_escapes_pipes(tmp_path: Path) -> None:
    p = tmp_path / "pipe.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    sidecar = json.loads(json.dumps(SAMPLE_SIDECAR))
    sidecar["benchmarks"][0]["api"] = "decode | to"
    sidecar_path(p).write_text(json.dumps(sidecar), encoding="utf-8")
    assert "decode \\| to" in render_markdown([load_suite(p)])


def test_markdown_warns_for_sanitized_build(tmp_path: Path) -> None:
    p = tmp_path / "asan.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    sidecar = json.loads(json.dumps(SAMPLE_SIDECAR))
    sidecar["environment"]["sanitizers"] = ["address"]
    sidecar_path(p).write_text(json.dumps(sidecar), encoding="utf-8")
    assert "built with address sanitizer" in render_markdown([load_suite(p)])


def test_markdown_warns_for_missing_sidecar(sample_xml_path: Path) -> None:
    assert "no smoke.meta.json beside it" in render_markdown(
        [load_suite(sample_xml_path)]
    )


def test_markdown_contains_formula_note(run_xml_path: Path) -> None:
    assert "Throughput = input bytes / mean time" in render_markdown(
        [load_suite(run_xml_path)]
    )


# ---------------------------------------------------------------------------
# SVG and HTML
# ---------------------------------------------------------------------------


def test_standalone_svg_has_no_custom_properties(run_xml_path: Path) -> None:
    group = group_by_conversion(load_suite(run_xml_path).measurements)[0]
    svg = render_svg_chart(group, standalone=True)
    assert svg.startswith('<svg xmlns="http://www.w3.org/2000/svg"')
    assert "var(" not in svg
    assert UTF in svg


def test_svg_has_one_row_per_measurement_and_a_legend(run_xml_path: Path) -> None:
    group = group_by_conversion(load_suite(run_xml_path).measurements)[0]
    svg = render_svg_chart(group)
    assert svg.count('class="bt-row"') == 2
    assert "iconv_transcode_view" in svg
    assert svg.count("<circle") >= 2  # beman.transcode marks plus its legend key
    assert '<rect class="bt-mark lib-2"' in svg  # iconv is drawn as a square


def test_svg_keeps_excluded_rows_with_reason(run_xml_path: Path) -> None:
    groups = group_by_conversion(load_suite(run_xml_path).measurements)
    sjis = next(g for g in groups if g.conversion == "Shift_JIS → UTF-32")
    svg = render_svg_chart(sjis)
    assert "not plotted: input was empty" in svg
    assert "bt-ci" not in svg


def test_html_is_self_contained(run_xml_path: Path) -> None:
    page = render_html([load_suite(run_xml_path)], label="GCC Perf")
    assert page.startswith("<title>")
    assert "<script src" not in page
    assert "<link" not in page
    assert 'id="windows-1251-to-utf-32"' in page
    assert "Flagged results" in page
    assert page.count("<tr>") == 1 + 5  # header row, then one per result


# ---------------------------------------------------------------------------
# render_vegalite_spec
# ---------------------------------------------------------------------------


def test_vegalite_spec_schema(run_xml_path: Path) -> None:
    spec = render_vegalite_spec([load_suite(run_xml_path)])
    assert spec["$schema"].startswith("https://vega.github.io/schema/vega-lite/")


def test_vegalite_omits_excluded_results(run_xml_path: Path) -> None:
    spec = render_vegalite_spec([load_suite(run_xml_path)])
    names = [v["benchmark"] for v in spec["data"]["values"]]
    assert names == [
        "whatwg_decode utf_8 en_mars corpus",
        "UTF-8 decode: Arabic (multibyte-heavy)",
        "Single-byte decode: windows-1251 Russian",
    ]


def test_vegalite_ci_brackets_mean(run_xml_path: Path) -> None:
    for v in render_vegalite_spec([load_suite(run_xml_path)])["data"]["values"]:
        assert v["ci_low"] <= v["throughput_MiBs"] <= v["ci_high"]


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------


def test_main_writes_every_output(run_xml_path: Path, tmp_path: Path) -> None:
    out = tmp_path / "out"
    rc = main(
        [
            "--markdown", str(out / "r.md"),
            "--html", str(out / "r.html"),
            "--svg-dir", str(out / "svg"),
            "--vegalite", str(out / "r.vl.json"),
            str(run_xml_path),
        ]
    )  # fmt: skip
    assert rc == 0
    assert (out / "r.md").read_text(encoding="utf-8").startswith("## Benchmark Results")
    assert (out / "r.html").is_file()
    assert sorted(p.name for p in (out / "svg").iterdir()) == [
        "shift-jis-to-utf-32.svg",
        "utf-8-to-utf-32.svg",
        "windows-1251-to-utf-32.svg",
    ]
    json.loads((out / "r.vl.json").read_text(encoding="utf-8"))


def test_main_missing_file_fails(tmp_path: Path) -> None:
    assert main([str(tmp_path / "nope.xml")]) == 1


def test_main_no_results_fails(unknown_xml_path: Path, tmp_path: Path) -> None:
    empty = tmp_path / "empty.xml"
    empty.write_text(
        '<?xml version="1.0"?><Catch2TestRun name="x"></Catch2TestRun>',
        encoding="utf-8",
    )
    assert main([str(empty)]) == 1
    assert main([str(unknown_xml_path)]) == 0


# ---------------------------------------------------------------------------
# Several runs
# ---------------------------------------------------------------------------


def write_run(
    run_dir: Path,
    drop: tuple[str, ...] = (),
    benchmarks: dict[str, dict[str, object]] | None = None,
    speedup: float = 1.0,
    name: str = "smoke.xml",
    **environment: object,
) -> Path:
    """SAMPLE_XML and its sidecar in run_dir, with environment overrides.

    drop removes benchmarks from the XML; benchmarks overrides sidecar entries;
    speedup divides every mean and bound, making each result that much faster.
    """
    run_dir.mkdir(parents=True, exist_ok=True)
    xml = re.sub(
        r'(<mean value|lowerBound|upperBound)="([0-9.]+)"',
        lambda m: f'{m.group(1)}="{float(m.group(2)) / speedup}"',
        SAMPLE_XML,
    )
    for dropped in drop:
        start = xml.index(f'<BenchmarkResults name="{dropped}"')
        end = xml.index("</BenchmarkResults>", start) + len("</BenchmarkResults>")
        xml = xml[:start] + xml[end:]
    p = run_dir / name
    p.write_text(xml, encoding="utf-8")
    sidecar = json.loads(json.dumps(SAMPLE_SIDECAR))
    sidecar["environment"].update(environment)
    for entry in sidecar["benchmarks"]:
        entry.update((benchmarks or {}).get(entry["name"], {}))
    sidecar_path(p).write_text(json.dumps(sidecar), encoding="utf-8")
    return run_dir


def load(label: str, run_dir: Path) -> Run:
    return Run(label, [load_suite(f) for f in xml_files([run_dir])])


def test_xml_files_expands_directories(tmp_path: Path) -> None:
    (tmp_path / "b.xml").write_text("", encoding="utf-8")
    (tmp_path / "a.xml").write_text("", encoding="utf-8")
    (tmp_path / "a.meta.json").write_text("", encoding="utf-8")
    single = tmp_path / "elsewhere.xml"
    assert xml_files([tmp_path, single]) == [
        tmp_path / "a.xml",
        tmp_path / "b.xml",
        single,
    ]


def test_environment_summary_shows_commit(tmp_path: Path) -> None:
    run = load("", write_run(tmp_path / "a", commit="abc123"))
    assert ("Commit", "abc123") in environment_summary(run.suites)


def test_labels_come_from_the_differing_fields(tmp_path: Path) -> None:
    runs = [
        load("", write_run(tmp_path / "a", compiler="gcc 16.0.1 20260322 (exp)")),
        load("", write_run(tmp_path / "b", compiler="clang 22.1.0 (https://x)")),
        load("mine", write_run(tmp_path / "c", compiler="clang 21")),
    ]
    assign_labels(runs)
    assert [r.label for r in runs] == ["gcc 16.0.1", "clang 22.1.0", "mine"]


def test_label_says_when_a_field_was_not_recorded(tmp_path: Path) -> None:
    runs = [
        load("", write_run(tmp_path / "a")),
        load("", write_run(tmp_path / "b", commit="bbb")),
    ]
    assign_labels(runs)
    assert [r.label for r in runs] == ["commit unknown", "bbb"]


def test_indistinguishable_runs_are_numbered(tmp_path: Path) -> None:
    runs = [
        load("", write_run(tmp_path / "a")),
        load("", write_run(tmp_path / "b")),
    ]
    assign_labels(runs)
    assert [r.label for r in runs] == ["run 1", "run 2"]


def test_repeated_labels_get_a_suffix(tmp_path: Path) -> None:
    runs = [
        load("x", write_run(tmp_path / "a")),
        load("x", write_run(tmp_path / "b")),
    ]
    assign_labels(runs)
    assert [r.label for r in runs] == ["x", "x #2"]


def test_match_runs_pairs_by_suite_and_name(tmp_path: Path) -> None:
    runs = [
        load("a", write_run(tmp_path / "a")),
        load("b", write_run(tmp_path / "b", drop=("missing corpus",))),
    ]
    rows, duplicates = match_runs(runs)
    assert duplicates == []
    assert len(rows) == 5
    by_name = {r.name: r for r in rows}
    assert by_name["missing corpus"].missing_from == [1]
    assert all(c is not None for c in by_name["optimized away"].cells)
    assert by_name["optimized away"].cells[0] is runs[0].measurements[3]


def test_match_runs_warns_about_duplicates(tmp_path: Path) -> None:
    run_dir = write_run(tmp_path / "a")
    xml = (run_dir / "smoke.xml").read_text(encoding="utf-8")
    twice = xml.replace(
        '<TestCase name="suspicious"',
        '<TestCase name="again"><BenchmarkResults name="optimized away" samples="1" '
        'iterations="1"><mean value="1.0" lowerBound="1.0" upperBound="1.0"/>'
        '</BenchmarkResults></TestCase><TestCase name="suspicious"',
    )
    (run_dir / "smoke.xml").write_text(twice, encoding="utf-8")
    runs = [load("a", run_dir), load("b", write_run(tmp_path / "b"))]
    rows, duplicates = match_runs(runs)
    assert len(rows) == 5
    assert duplicates == [
        "a: smoke / optimized away appears more than once in smoke.xml; using the first."
    ]


def test_changed_input_is_noted(tmp_path: Path) -> None:
    runs = [
        load("a", write_run(tmp_path / "a")),
        load(
            "b",
            write_run(
                tmp_path / "b",
                benchmarks={"optimized away": {"input_bytes": 8192}},
            ),
        ),
    ]
    rows, _ = match_runs(runs)
    differs = [r.name for r in rows if r.input_differs]
    assert differs == ["optimized away"]
    md = render_comparison_markdown(runs)
    assert "input differs" in md


def test_warns_when_runs_differ_in_more_than_one_field(tmp_path: Path) -> None:
    runs = [
        load("a", write_run(tmp_path / "a")),
        load("b", write_run(tmp_path / "b", compiler="clang 22", host="other")),
    ]
    warnings = comparison_warnings(runs)
    assert warnings[0].startswith("The runs differ in compiler and host")


def test_one_differing_field_is_not_a_warning(tmp_path: Path) -> None:
    runs = [
        load("a", write_run(tmp_path / "a")),
        load("b", write_run(tmp_path / "b", compiler="clang 22")),
    ]
    assert comparison_warnings(runs) == []


def test_warns_about_mixed_suites_dirty_trees_and_sanitizers(tmp_path: Path) -> None:
    write_run(tmp_path / "a", commit="abc-dirty", sanitizers=["address"])
    other = json.loads(json.dumps(SAMPLE_SIDECAR))
    other["environment"]["build_config"] = "Debug"
    other["environment"]["commit"] = "abc-dirty"
    other["environment"]["sanitizers"] = ["address"]
    (tmp_path / "a" / "utf.xml").write_text(
        SAMPLE_XML.replace("benchmarks.smoke", "benchmarks.utf"), encoding="utf-8"
    )
    (tmp_path / "a" / "utf.meta.json").write_text(json.dumps(other), encoding="utf-8")
    runs = [load("a", tmp_path / "a"), load("b", write_run(tmp_path / "b"))]
    warnings = comparison_warnings(runs)
    assert "a: its suites differ in build config (Debug; Perf)." in warnings
    assert any(w.startswith("a: built from a modified tree") for w in warnings)
    assert any(w.startswith("a: smoke: built with address sanitizer") for w in warnings)


def test_other_load_discounts_the_benchmark_thread() -> None:
    assert other_load({}) is None
    assert other_load({"load_start": [0.5, 0, 0]}) == 0.5
    assert other_load({"load_end": [0.5, 0, 0]}) == 0.0
    assert other_load({"load_start": [0.5, 0, 0], "load_end": [4.0, 0, 0]}) == 3.0


def test_busy_suite_is_warned_about(tmp_path: Path) -> None:
    busy = load("", write_run(tmp_path / "a", load_start=[21.5, 20, 18], cpus=20))
    (warning,) = suite_warnings(busy.suites)
    assert warning == (
        "smoke: the machine was busy (1-minute load 21.50 at the start, on 20 "
        "CPUs), so other work may have slowed these timings."
    )
    quiet = load(
        "",
        write_run(
            tmp_path / "b", load_start=[BUSY_LOAD, 1, 1], load_end=[BUSY_LOAD + 1, 1, 1]
        ),
    )
    assert suite_warnings(quiet.suites) == []


def test_environment_summary_shows_the_machine_state(tmp_path: Path) -> None:
    run = load(
        "",
        write_run(
            tmp_path / "a",
            cpus=20,
            governor="powersave",
            turbo=True,
            kernel="7.0.0",
            load_start=[1.43, 1, 1],
            load_end=[2.1, 1, 1],
        ),
    )
    rows = environment_summary(run.suites)
    for row in [
        ("CPUs", "20"),
        ("Governor", "powersave"),
        ("Turbo", "on"),
        ("Kernel", "7.0.0"),
        ("Load (1 min)", "1.43 – 2.10"),
    ]:
        assert row in rows


def test_comparison_shows_each_runs_load(tmp_path: Path) -> None:
    runs = [
        load("quiet", write_run(tmp_path / "a", load_start=[0.4, 0, 0])),
        load("busy", write_run(tmp_path / "b", load_start=[21.5, 0, 0])),
    ]
    md = render_comparison_markdown(runs)
    assert "| CPU | Other load | Run at | Suites |" in md
    assert "| Test CPU | 0.40 |" in md
    assert "| Test CPU | 21.50 (busy) |" in md
    assert any(
        w.startswith("busy: smoke: the machine was busy")
        for w in comparison_warnings(runs)
    )


def test_turbo_label_names_the_field(tmp_path: Path) -> None:
    runs = [
        load("", write_run(tmp_path / "a", turbo=True)),
        load("", write_run(tmp_path / "b", turbo=False)),
    ]
    assign_labels(runs)
    assert [r.label for r in runs] == ["turbo on", "turbo off"]


def test_comparison_markdown_has_a_column_per_run(tmp_path: Path) -> None:
    runs = [
        load("before", write_run(tmp_path / "a", commit="aaa")),
        load(
            "after", write_run(tmp_path / "b", commit="bbb", drop=("missing corpus",))
        ),
    ]
    md = render_comparison_markdown(runs, label="gcc")
    assert md.startswith("## Benchmark Comparison — gcc")
    assert "| Run | Compiler | Build config | Commit |" in md
    assert "| before | gcc 16.0.1 | Perf | aaa |" in md
    assert "| Input | Library | API | before | after | after vs before | Notes |" in md
    assert "| — | not run | — | not in after; before: empty-input |" in md
    assert "| 160 | 160 | no clear change |" in md
    assert "### Not in every run" in md
    assert "- smoke / missing corpus: not in after" in md
    assert "before: implausible, noisy; after: implausible, noisy" in md


def test_comparison_html_is_self_contained_and_escaped(tmp_path: Path) -> None:
    runs = [
        load("<before>", write_run(tmp_path / "a")),
        load("after", write_run(tmp_path / "b")),
    ]
    page = render_comparison_html(runs)
    assert "&lt;before&gt;" in page
    assert "<before>" not in page
    assert "<script src" not in page
    assert "<link" not in page
    assert (
        'aria-label="UTF-8 → UTF-32: after vs &lt;before&gt; throughput ratio"' in page
    )


def test_comparison_vegalite_facets_by_run(tmp_path: Path) -> None:
    runs = [
        load("before", write_run(tmp_path / "a")),
        load("after", write_run(tmp_path / "b")),
    ]
    spec = render_comparison_vegalite(runs, title="t")
    throughput, ratios = spec["vconcat"]
    assert throughput["facet"]["column"]["sort"] == ["before", "after"]
    assert {v["run"] for v in throughput["data"]["values"]} == {"before", "after"}
    assert {v["run"] for v in ratios["data"]["values"]} == {"after vs before"}
    assert all(v["ratio"] == 1.0 for v in ratios["data"]["values"])


def test_main_compares_runs(tmp_path: Path) -> None:
    a = write_run(tmp_path / "a")
    b = write_run(tmp_path / "b", compiler="clang 22")
    out = tmp_path / "out"
    rc = main(
        [
            "--run", "gcc", str(a),
            "--run", "clang", str(b / "smoke.xml"),
            "--markdown", str(out / "c.md"),
            "--html", str(out / "c.html"),
            "--svg-dir", str(out / "svg"),
            "--vegalite", str(out / "c.vl.json"),
        ]
    )  # fmt: skip
    assert rc == 0
    md = (out / "c.md").read_text(encoding="utf-8")
    assert md.startswith("## Benchmark Comparison")
    assert "| gcc | clang |" in md
    assert "benchmark comparison" in (out / "c.html").read_text(encoding="utf-8")
    assert sorted(p.name for p in (out / "svg").iterdir()) == [
        "clang",
        "clang-vs-gcc",
        "gcc",
    ]
    assert (out / "svg" / "clang-vs-gcc" / "utf-8-to-utf-32.svg").is_file()
    json.loads((out / "c.vl.json").read_text(encoding="utf-8"))


def test_main_positional_files_and_run_are_compared(tmp_path: Path) -> None:
    a = write_run(tmp_path / "a")
    b = write_run(tmp_path / "b", commit="bbb")
    out = tmp_path / "c.md"
    assert main([str(a), "--run", "new", str(b), "--markdown", str(out)]) == 0
    assert "| commit unknown | new |" in out.read_text(encoding="utf-8")


def test_main_empty_label_is_derived(tmp_path: Path) -> None:
    a = write_run(tmp_path / "a", commit="aaa")
    b = write_run(tmp_path / "b", commit="bbb")
    out = tmp_path / "c.md"
    assert main(["--run", "", str(a), "--run", "", str(b), "--markdown", str(out)]) == 0
    assert "| aaa | bbb |" in out.read_text(encoding="utf-8")


def test_main_run_without_files_fails(tmp_path: Path) -> None:
    assert main(["--run", "lonely"]) == 1


def test_main_empty_directory_fails(tmp_path: Path) -> None:
    assert main(["--run", "x", str(tmp_path)]) == 1


def test_main_without_input_fails() -> None:
    assert main([]) == 1


# ---------------------------------------------------------------------------
# Ratios
# ---------------------------------------------------------------------------


def test_throughput_ratio_is_faster_above_one(tmp_path: Path) -> None:
    (before,) = load("", write_run(tmp_path / "a")).suites
    (after,) = load("", write_run(tmp_path / "b", speedup=2.0)).suites
    ratio = throughput_ratio(after.measurements[0], before.measurements[0])
    assert ratio is not None
    assert ratio.value == pytest.approx(2.0)
    # The interval spans the far ends of both CIs: 3400/(3700/2) to 3700/(3400/2).
    assert ratio.low == pytest.approx(2 * 3400 / 3700)
    assert ratio.high == pytest.approx(2 * 3700 / 3400)
    assert ratio.clear


def test_throughput_ratio_needs_the_same_plotted_input(run_xml_path: Path) -> None:
    ms = {m.name: m for m in load_suite(run_xml_path).measurements}
    en = ms["whatwg_decode utf_8 en_mars corpus"]
    assert throughput_ratio(en, ms["Single-byte decode: windows-1251 Russian"]) is None
    assert throughput_ratio(en, ms["optimized away"]) is None
    assert throughput_ratio(en, None) is None


def test_fmt_ratio_reports_only_clear_differences() -> None:
    assert fmt_ratio(Ratio(1.5, 1.2, 1.8)) == "1.50× faster"
    assert fmt_ratio(Ratio(0.04, 0.03, 0.05)) == "25.0× slower"
    assert fmt_ratio(Ratio(1.02, 0.97, 1.08)) == "no clear change"
    assert fmt_ratio(Ratio(1.02, 0.97, 1.08), unclear="x") == "x"
    assert fmt_ratio(None) == "—"


def test_ratio_axis_is_centred_on_one() -> None:
    (lo, hi), ticks = ratio_axis([Ratio(1.4, 1.3, 1.6), Ratio(0.9, 0.8, 1.0)])
    assert lo == -hi
    assert 10**hi == pytest.approx(2.0)
    assert [t for _, t in ticks] == [
        "0.5×",
        "0.67×",
        "0.8×",
        "1×",
        "1.25×",
        "1.5×",
        "2×",
    ]
    (lo, hi), ticks = ratio_axis([])
    assert 10**hi == pytest.approx(1.1)
    _, ticks = ratio_axis([Ratio(0.13, 0.12, 0.14)])
    assert [t for _, t in ticks] == ["0.1×", "0.2×", "0.5×", "1×", "2×", "5×", "10×"]
    _, ticks = ratio_axis([Ratio(300, 250, 350)])
    assert [t for _, t in ticks] == [
        "0.001×",
        "0.01×",
        "0.1×",
        "1×",
        "10×",
        "100×",
        "1000×",
    ]


def test_markdown_compares_libraries_with_the_reference(run_xml_path: Path) -> None:
    md = render_markdown([load_suite(run_xml_path)])
    assert f"| MiB/s | vs {REFERENCE_LIBRARY} | Notes |" in md
    rows = {line.split(" | ")[1]: line for line in md.splitlines() if "en_mars" in line}
    assert "| reference |" in rows["beman.transcode"]
    assert "| 1.43× slower |" in rows["iconv"]
    # A conversion only one library ran gets no ratio column.
    section = md.split("### windows-1251 → UTF-32")[1]
    assert "| MiB/s | Notes |" in section


def test_reference_library_can_be_chosen(run_xml_path: Path, tmp_path: Path) -> None:
    out = tmp_path / "r.md"
    rc = main(
        ["--reference-library", "iconv", "--markdown", str(out), str(run_xml_path)]
    )
    assert rc == 0
    md = out.read_text(encoding="utf-8")
    assert "| MiB/s | vs iconv | Notes |" in md
    assert "| 1.43× faster |" in md


def test_comparison_reports_a_clear_change(tmp_path: Path) -> None:
    runs = [
        load("before", write_run(tmp_path / "a")),
        load("after", write_run(tmp_path / "b", speedup=0.5)),
    ]
    md = render_comparison_markdown(runs)
    assert "| 160 | 79.8 | 2.00× slower |" in md


def test_ratio_chart_marks_one_and_no_clear_change(tmp_path: Path) -> None:
    runs = [
        load("before", write_run(tmp_path / "a")),
        load("after", write_run(tmp_path / "b", drop=("missing corpus",))),
    ]
    rows, _ = match_runs(runs)
    groups, by_reference = comparison_groups(rows)
    svgs = {
        g.conversion: render_ratio_svg(g, by_reference, runs, 1, standalone=True)
        for g in groups
    }
    utf = svgs[UTF]
    assert ">1×</text>" in utf
    assert "no clear change" in utf
    assert "UTF-8 → UTF-32: after vs before" in utf
    assert "var(--" not in utf
    # The missing corpus was excluded in before, and is absent from after.
    assert "not plotted in before" in svgs["Shift_JIS → UTF-32"]
    assert "not plotted in before" in svgs["windows-1251 → UTF-32"]


# ---------------------------------------------------------------------------
# Repeats
# ---------------------------------------------------------------------------


def write_repeats(run_dir: Path, speedups: list[float], **kwargs: object) -> Path:
    """One smoke.N.xml per speedup, a second apart."""
    for i, speedup in enumerate(speedups, 1):
        write_run(
            run_dir,
            speedup=speedup,
            name=f"smoke.{i}.xml",
            timestamp=f"2026-10-24T00:00:{i:02}Z",
            **kwargs,  # type: ignore[arg-type]
        )
    return run_dir


def by_name(measurements: list[Measurement]) -> dict[str, Measurement]:
    return {m.name: m for m in measurements}


EN = "whatwg_decode utf_8 en_mars corpus"
AR = "UTF-8 decode: Arabic (multibyte-heavy)"


def test_repeats_are_combined_into_one_result(tmp_path: Path) -> None:
    run = load("", write_repeats(tmp_path / "a", [1.0, 2.0, 0.5]))
    ms = by_name(run.measurements)
    assert len(ms) == 5
    en = ms[EN]
    assert en.repeat_count == 3
    assert en.result.mean_ns == pytest.approx(3500)  # the median
    assert en.result.low_ns == pytest.approx(1750)
    assert en.result.high_ns == pytest.approx(7000)
    assert en.best_ns == pytest.approx(1750)
    assert en.spread == pytest.approx(4.0)
    assert run.repeats == 3


def test_a_single_file_is_not_combined(run_xml_path: Path) -> None:
    suite = load_suite(run_xml_path)
    assert suite_measurements([suite]) == suite.measurements


def test_repeats_are_ordered_by_time_then_number(tmp_path: Path) -> None:
    d = tmp_path / "a"
    for name in ("smoke.10.xml", "smoke.9.xml"):
        write_run(d, name=name)
    write_run(d, name="smoke.11.xml", timestamp="2026-10-23T00:00:00Z")
    suites = sorted((load_suite(f) for f in xml_files([d])), key=repeat_order)
    assert [s.path.name for s in suites] == [
        "smoke.11.xml",
        "smoke.9.xml",
        "smoke.10.xml",
    ]


def test_repeats_show_median_fastest_and_range(tmp_path: Path) -> None:
    d = write_repeats(tmp_path / "a", [1.0, 2.0])
    write_run(d, name="smoke.3.xml", drop=(AR,), timestamp="2026-10-24T00:00:03Z")
    run = load("", d)
    assert ("Suites", "smoke ×3") in environment_summary(run.suites)
    md = render_markdown(run.suites)
    assert "| Median | Fastest | Range | MiB/s | vs beman.transcode | Notes |" in md
    assert "| 3.5 µs | 1.75 µs | 1.75 µs – 3.5 µs (×2.00) |" in md
    assert "2 of 3 repeats" in md
    assert "median of its repeats' means" in md
    page = render_html(run.suites)
    assert "<th>Median</th><th>Fastest</th><th>Range</th>" in page
    assert "range across repeats" in page


def test_libraries_are_compared_within_each_repeat(tmp_path: Path) -> None:
    # Each repeat slows every library by the same factor, so the ranges
    # overlap but the ratio within every repeat is the same.
    run = load("", write_repeats(tmp_path / "a", [1.0, 2.0, 0.5]))
    ms = by_name(run.measurements)
    ratio = library_ratio(ms[AR], ms[EN])
    assert ratio is not None
    assert ratio.value == pytest.approx(0.7)
    assert ratio.low == pytest.approx(0.7)
    assert ratio.high == pytest.approx(0.7)
    assert throughput_ratio(ms[AR], ms[EN]) is not None
    assert not throughput_ratio(ms[AR], ms[EN]).clear  # type: ignore[union-attr]
    assert "| 1.43× slower |" in render_markdown(run.suites)


def test_runs_with_repeats_are_compared_by_their_ranges(tmp_path: Path) -> None:
    base = load("base", write_repeats(tmp_path / "a", [1.0, 1.05, 0.95]))
    faster = load("faster", write_repeats(tmp_path / "b", [2.0, 2.1, 1.9]))
    same = load("same", write_repeats(tmp_path / "c", [1.02, 0.9, 1.1]))
    md = render_comparison_markdown([base, faster, same])
    row = next(
        line
        for line in md.splitlines()
        if "whatwg_decode view" in line and "en_mars" in line
    )
    assert "| 2.00× faster |" in row
    assert row.endswith("| no clear change |  |")
    header, rows = run_table([base, faster])
    assert "Repeats" in header
    assert rows[0][header.index("Repeats")] == "3"
    assert rows[0][header.index("Suites")] == "smoke ×3"


def test_run_table_has_no_repeats_column_without_repeats(tmp_path: Path) -> None:
    header, _ = run_table(
        [load("a", write_run(tmp_path / "a")), load("b", write_run(tmp_path / "b"))]
    )
    assert "Repeats" not in header


def test_warnings_name_the_repeat_file(tmp_path: Path) -> None:
    run = load("", write_repeats(tmp_path / "a", [1.0, 1.0], load_start=[9.0, 0, 0]))
    warnings = suite_warnings(run.suites)
    assert [w.split(":")[0] for w in warnings] == ["smoke.1.xml", "smoke.2.xml"]


def test_single_run_warns_about_duplicate_names(run_xml_path: Path) -> None:
    xml = run_xml_path.read_text(encoding="utf-8")
    run_xml_path.write_text(
        xml.replace(
            "</TestCase>",
            "</TestCase>" + xml.split("<TestCase")[1].join(["<TestCase", ""]),
            1,
        ),
        encoding="utf-8",
    )
    suite = load_suite(run_xml_path)
    assert duplicate_warnings([suite]) == [
        f"smoke / {EN} appears more than once in smoke.xml; using the first."
    ]
    assert any(
        "appears more than once" in line
        for line in render_markdown([suite]).splitlines()
    )


# ---------------------------------------------------------------------------
# Instruction counts
# ---------------------------------------------------------------------------


def callgrind_dump(name: str | None, total: int, summary: bool = True) -> str:
    """A callgrind dump as bench::measure leaves one, or as the program's end does."""
    trigger = f"Client Request: {name}" if name is not None else "Program termination"
    return (
        "# callgrind format\nversion: 1\ncreator: callgrind-3.26.0\npid: 1\n"
        f"cmd: smoke\npart: 1\n\ndesc: I1 cache: \ndesc: Trigger: {trigger}\n"
        "positions: line\nevents: Ir\n"
        + (f"summary: {total}\n" if summary else "")
        + f"\nfn=(1) main\n0 {total}\ntotals: {total}\n"
    )


def write_counted(
    run_dir: Path,
    per_call: Mapping[str, float],
    count_runs: int = 2,
    **kwargs: object,
) -> Path:
    """write_run in count mode, with a callgrind dump per benchmark in per_call
    holding that many instructions per call."""
    write_run(run_dir, count_runs=count_runs, **kwargs)  # type: ignore[arg-type]
    for i, (name, n) in enumerate(per_call.items(), 1):
        (run_dir / f"smoke.callgrind.4242.{i}").write_text(
            callgrind_dump(name, round(n * count_runs)), encoding="utf-8"
        )
    (run_dir / "smoke.callgrind.4242").write_text(
        callgrind_dump(None, 0), encoding="utf-8"
    )
    return run_dir


def load_dirs(label: str, *run_dirs: Path) -> Run:
    return Run(label, [load_suite(f) for f in xml_files(list(run_dirs))])


# 8 instructions per byte for beman.transcode on the 586-byte input, 16 for iconv.
COUNTS = {EN: 8 * 586, AR: 16 * 586}


def test_parse_callgrind_dump_reads_the_name_and_total() -> None:
    assert parse_callgrind_dump(callgrind_dump(EN, 9376)) == (EN, 9376)
    assert parse_callgrind_dump(callgrind_dump(EN, 9376, summary=False)) == (EN, 9376)
    assert parse_callgrind_dump(callgrind_dump(None, 0)) == (None, 0)


def test_counted_suite_gives_counts_not_times(tmp_path: Path) -> None:
    run = load("", write_counted(tmp_path / "c", COUNTS))
    (suite,) = run.suites
    assert suite.counted and suite.count_runs == 2
    ms = by_name(run.measurements)
    assert not ms[EN].timed
    assert ms[EN].instructions == pytest.approx(8 * 586)
    assert ms[EN].instructions_per_byte == pytest.approx(8.0)
    assert ms[EN].throughput is None
    assert ms["Single-byte decode: windows-1251 Russian"].instructions is None
    # Valgrind's timings do not make a result implausible or noisy.
    assert ms["optimized away"].flags == []


def test_few_instructions_per_byte_is_flagged_and_excluded(tmp_path: Path) -> None:
    run = load("", write_counted(tmp_path / "c", {"optimized away": 4}))
    m = by_name(run.measurements)["optimized away"]
    assert m.flags == ["few-instructions"]
    assert m.instructions_per_byte is None


def test_counts_join_the_timed_results(tmp_path: Path) -> None:
    write_run(tmp_path / "t", drop=(AR,))
    write_counted(tmp_path / "c", COUNTS)
    run = load_dirs("", tmp_path / "t", tmp_path / "c")
    ms = by_name(run.measurements)
    assert len(ms) == 5
    assert ms[EN].timed and ms[EN].instructions_per_byte == pytest.approx(8.0)
    assert ms[EN].throughput is not None
    # A count with no timed result is kept as one of its own.
    assert not ms[AR].timed and ms[AR].instructions_per_byte == pytest.approx(16.0)
    assert suite_list(run.suites) == "smoke, smoke (counted)"
    assert run.repeats == 1


def test_count_ratio_has_a_small_tolerance(tmp_path: Path) -> None:
    run = load("", write_counted(tmp_path / "c", COUNTS))
    ms = by_name(run.measurements)
    ratio = count_ratio(ms[AR], ms[EN])
    assert ratio is not None and ratio.value == pytest.approx(0.5)
    assert fmt_count_ratio(ratio) == "2.00× more"
    assert fmt_count_ratio(Ratio(1.005, 1.005 / 1.01, 1.005 * 1.01)) == (
        "no clear change"
    )
    assert fmt_count_ratio(Ratio(1.03, 1.03 / 1.01, 1.03 * 1.01)) == "1.03× fewer"


def test_markdown_shows_instructions_beside_times(tmp_path: Path) -> None:
    write_run(tmp_path / "t")
    write_counted(tmp_path / "c", COUNTS)
    md = render_markdown(load_dirs("", tmp_path / "t", tmp_path / "c").suites)
    assert "| MiB/s | vs beman.transcode | Instr/B | instr vs beman.transcode |" in md
    assert "| 8.00 | reference |" in md
    assert "| 16.0 | 2.00× more |" in md
    assert COUNT_NOTE in md


def test_markdown_of_counts_alone_has_no_time_columns(tmp_path: Path) -> None:
    md = render_markdown(load("", write_counted(tmp_path / "c", COUNTS)).suites)
    assert "MiB/s" not in md
    assert "Mean" not in md
    assert (
        "| Input | Library | API | Instr/B | instr vs beman.transcode | Notes |" in md
    )
    assert "Throughput =" not in md


def test_counted_suite_is_not_warned_about_load(tmp_path: Path) -> None:
    run = load(
        "",
        write_counted(tmp_path / "c", COUNTS, load_start=[BUSY_LOAD + 5, 0, 0], cpus=4),
    )
    assert suite_warnings(run.suites) == []


def test_counted_suite_without_dumps_is_warned_about(tmp_path: Path) -> None:
    run = load("", write_run(tmp_path / "c", count_runs=3))
    (warning,) = suite_warnings(run.suites)
    assert warning.startswith("smoke (counted): run in count mode, but no callgrind")


def test_count_chart_plots_instructions_per_byte(tmp_path: Path) -> None:
    run = load("", write_counted(tmp_path / "c", COUNTS))
    group = next(
        g for g in group_by_conversion(run.measurements) if g.conversion == UTF
    )
    svg = render_count_chart(group, standalone=True)
    assert "instructions per byte" in svg
    assert ">8.00<" in svg and ">16.0<" in svg


def test_html_draws_a_count_chart_beside_the_throughput_chart(tmp_path: Path) -> None:
    write_run(tmp_path / "t")
    write_counted(tmp_path / "c", COUNTS)
    page = render_html(load_dirs("", tmp_path / "t", tmp_path / "c").suites)
    assert 'aria-label="UTF-8 → UTF-32 throughput"' in page
    assert 'aria-label="UTF-8 → UTF-32: instructions per byte chart"' in page
    assert "<th>Instr/B</th>" in page


def test_comparison_shows_count_ratios(tmp_path: Path) -> None:
    before = load("before", write_counted(tmp_path / "a", COUNTS))
    after = load("after", write_counted(tmp_path / "b", {EN: 4 * 586, AR: 16.05 * 586}))
    runs = [before, after]
    md = render_comparison_markdown(runs)
    assert "| before instr/B | after instr/B | after vs before instr |" in md
    assert "| 8.00 | 4.00 | 2.00× fewer |" in md
    assert "| 16.0 | 16.1 | no clear change |" in md
    assert "MiB/s |" not in md.split("### Runs")[1].split("\n\n")[0]
    page = render_comparison_html(runs)
    assert "instruction count ratio" in page
    rows, _ = match_runs(runs)
    groups, by_reference = comparison_groups(rows)
    utf = next(g for g in groups if g.conversion == UTF)
    svg = render_ratio_svg(utf, by_reference, runs, 1, standalone=True, counts=True)
    assert ">2.00×<" in svg
    assert "no clear change" in svg


def test_vegalite_carries_instruction_counts(tmp_path: Path) -> None:
    write_run(tmp_path / "t")
    write_counted(tmp_path / "c", COUNTS)
    spec = render_vegalite_spec(load_dirs("", tmp_path / "t", tmp_path / "c").suites)
    ar = next(v for v in spec["data"]["values"] if v["benchmark"] == AR)
    assert ar["instructions_per_byte"] == 16.0
    assert ar["instructions_vs_reference"] == 0.5
    before = load("before", write_counted(tmp_path / "a", COUNTS))
    spec = render_comparison_vegalite([before, before], "t")
    entry = spec["vconcat"][1]["data"]["values"][0]
    assert entry["instructions_ratio"] == 1.0
    assert "ratio" not in entry


def test_main_writes_count_charts(tmp_path: Path) -> None:
    write_run(tmp_path / "t")
    write_counted(tmp_path / "c", COUNTS)
    out = tmp_path / "svg"
    assert main([str(tmp_path / "t"), str(tmp_path / "c"), "--svg-dir", str(out)]) == 0
    names = sorted(p.name for p in out.iterdir())
    assert "utf-8-to-utf-32.svg" in names
    assert "utf-8-to-utf-32.instructions.svg" in names
