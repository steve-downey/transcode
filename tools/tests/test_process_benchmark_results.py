# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ruff: noqa: E501
"""Tests for tools/process_benchmark_results.py."""

import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from process_benchmark_results import (
    IMPLAUSIBLE_BYTES_PER_NS,
    SIDECAR_FORMAT,
    group_by_conversion,
    load_run,
    log_domain,
    main,
    parse_catch2_xml,
    render_html,
    render_markdown,
    render_svg_chart,
    render_vegalite_spec,
    sidecar_path,
    slugify,
    suite_name,
    throughput_MiBs,
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
    run = load_run(run_xml_path)
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
    m = load_run(run_xml_path).measurements[0]
    assert m.throughput_ci == pytest.approx(
        (throughput_MiBs(586, 3700.0), throughput_MiBs(586, 3400.0))
    )


def test_clean_results_have_no_flags(run_xml_path: Path) -> None:
    run = load_run(run_xml_path)
    assert [m.flags for m in run.measurements[:3]] == [[], [], []]


def test_sub_nanosecond_result_is_implausible_and_noisy(run_xml_path: Path) -> None:
    m = load_run(run_xml_path).measurements[3]
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
    m = load_run(p).measurements[0]
    assert IMPLAUSIBLE_BYTES_PER_NS < 586 / 10.0
    assert m.flags == ["implausible"]


def test_missing_corpus_is_empty_input(run_xml_path: Path) -> None:
    m = load_run(run_xml_path).measurements[4]
    assert m.flags == ["empty-input"]
    assert m.throughput is None


def test_no_sidecar_flags_every_result(sample_xml_path: Path) -> None:
    run = load_run(sample_xml_path)
    assert not run.has_sidecar
    assert all("no-metadata" in m.flags for m in run.measurements)
    assert all(m.throughput is None for m in run.measurements)


def test_unknown_sidecar_format_is_rejected(tmp_path: Path) -> None:
    p = tmp_path / "x.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    sidecar_path(p).write_text(json.dumps({"format": "other/9"}), encoding="utf-8")
    with pytest.raises(ValueError, match="unknown sidecar format"):
        load_run(p)


# ---------------------------------------------------------------------------
# grouping
# ---------------------------------------------------------------------------


def test_group_compared_conversions_first(run_xml_path: Path) -> None:
    groups = group_by_conversion(load_run(run_xml_path).measurements)
    assert groups[0].conversion == UTF
    assert groups[0].libraries == ["beman.transcode", "iconv"]


def test_group_inputs_run_small_to_large(run_xml_path: Path) -> None:
    groups = group_by_conversion(load_run(run_xml_path).measurements)
    w1251 = next(g for g in groups if g.conversion == "windows-1251 → UTF-32")
    assert [g.input_bytes for g in w1251.inputs] == [435, 4096]


def test_log_domain_is_decade_aligned_and_ignores_excluded(run_xml_path: Path) -> None:
    run = load_run(run_xml_path)
    assert log_domain(run.measurements) == (2, 3)


def test_log_domain_with_nothing_plotted(sample_xml_path: Path) -> None:
    assert log_domain(load_run(sample_xml_path).measurements) == (1.0, 4.0)


def test_slugify_names_files_for_the_conversion() -> None:
    assert slugify("Shift_JIS → UTF-8") == "shift-jis-to-utf-8"
    assert slugify("mock iconv: E2BIG every byte") == "mock-iconv-e2big-every-byte"


# ---------------------------------------------------------------------------
# render_markdown
# ---------------------------------------------------------------------------


def test_markdown_has_environment_and_conversion_sections(run_xml_path: Path) -> None:
    md = render_markdown([load_run(run_xml_path)], label="GCC Perf")
    assert "## Benchmark Results — GCC Perf" in md
    assert "**Compiler:** gcc 16.0.1" in md
    assert f"### {UTF}" in md
    assert "### windows-1251 → UTF-32" in md


def test_markdown_excluded_rows_show_dash_and_flag(run_xml_path: Path) -> None:
    md = render_markdown([load_run(run_xml_path)])
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
    assert "decode \\| to" in render_markdown([load_run(p)])


def test_markdown_warns_for_sanitized_build(tmp_path: Path) -> None:
    p = tmp_path / "asan.xml"
    p.write_text(SAMPLE_XML, encoding="utf-8")
    sidecar = json.loads(json.dumps(SAMPLE_SIDECAR))
    sidecar["environment"]["sanitizers"] = ["address"]
    sidecar_path(p).write_text(json.dumps(sidecar), encoding="utf-8")
    assert "built with address sanitizer" in render_markdown([load_run(p)])


def test_markdown_warns_for_missing_sidecar(sample_xml_path: Path) -> None:
    assert "no smoke.meta.json beside it" in render_markdown(
        [load_run(sample_xml_path)]
    )


def test_markdown_contains_formula_note(run_xml_path: Path) -> None:
    assert "Throughput = input bytes / mean time" in render_markdown(
        [load_run(run_xml_path)]
    )


# ---------------------------------------------------------------------------
# SVG and HTML
# ---------------------------------------------------------------------------


def test_standalone_svg_has_no_custom_properties(run_xml_path: Path) -> None:
    group = group_by_conversion(load_run(run_xml_path).measurements)[0]
    svg = render_svg_chart(group, standalone=True)
    assert svg.startswith('<svg xmlns="http://www.w3.org/2000/svg"')
    assert "var(" not in svg
    assert UTF in svg


def test_svg_has_one_row_per_measurement_and_a_legend(run_xml_path: Path) -> None:
    group = group_by_conversion(load_run(run_xml_path).measurements)[0]
    svg = render_svg_chart(group)
    assert svg.count('class="bt-row"') == 2
    assert "iconv_transcode_view" in svg
    assert svg.count("<circle") >= 2  # beman.transcode marks plus its legend key
    assert '<rect class="bt-mark lib-2"' in svg  # iconv is drawn as a square


def test_svg_keeps_excluded_rows_with_reason(run_xml_path: Path) -> None:
    groups = group_by_conversion(load_run(run_xml_path).measurements)
    sjis = next(g for g in groups if g.conversion == "Shift_JIS → UTF-32")
    svg = render_svg_chart(sjis)
    assert "not plotted: input was empty" in svg
    assert "bt-ci" not in svg


def test_html_is_self_contained(run_xml_path: Path) -> None:
    page = render_html([load_run(run_xml_path)], label="GCC Perf")
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
    spec = render_vegalite_spec([load_run(run_xml_path)])
    assert spec["$schema"].startswith("https://vega.github.io/schema/vega-lite/")


def test_vegalite_omits_excluded_results(run_xml_path: Path) -> None:
    spec = render_vegalite_spec([load_run(run_xml_path)])
    names = [v["benchmark"] for v in spec["data"]["values"]]
    assert names == [
        "whatwg_decode utf_8 en_mars corpus",
        "UTF-8 decode: Arabic (multibyte-heavy)",
        "Single-byte decode: windows-1251 Russian",
    ]


def test_vegalite_ci_brackets_mean(run_xml_path: Path) -> None:
    for v in render_vegalite_spec([load_run(run_xml_path)])["data"]["values"]:
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
