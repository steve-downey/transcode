# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ruff: noqa: E501
"""Tests for tools/process_benchmark_results.py."""

import json
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from process_benchmark_results import (
    BUSY_LOAD,
    IMPLAUSIBLE_BYTES_PER_NS,
    REFERENCE_LIBRARY,
    SIDECAR_FORMAT,
    Ratio,
    Run,
    assign_labels,
    comparison_groups,
    comparison_warnings,
    environment_summary,
    fmt_ratio,
    group_by_conversion,
    load_suite,
    log_domain,
    main,
    match_runs,
    other_load,
    parse_catch2_xml,
    ratio_axis,
    render_comparison_html,
    render_comparison_markdown,
    render_comparison_vegalite,
    render_html,
    render_markdown,
    render_ratio_svg,
    render_svg_chart,
    render_vegalite_spec,
    sidecar_path,
    slugify,
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
    for name in drop:
        start = xml.index(f'<BenchmarkResults name="{name}"')
        end = xml.index("</BenchmarkResults>", start) + len("</BenchmarkResults>")
        xml = xml[:start] + xml[end:]
    p = run_dir / "smoke.xml"
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
    write_run(tmp_path / "a")
    (tmp_path / "a" / "smoke2.xml").write_text(SAMPLE_XML, encoding="utf-8")
    runs = [load("a", tmp_path / "a"), load("b", write_run(tmp_path / "b"))]
    rows, duplicates = match_runs(runs)
    assert len(rows) == 5
    assert len(duplicates) == 5
    assert duplicates[0].startswith("a: smoke / ")


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
