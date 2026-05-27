# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ruff: noqa: E501
"""Tests for tools/process_benchmark_results.py."""

import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from process_benchmark_results import (
    CORPUS_NAME_HINTS,
    infer_corpus_file,
    load_corpus_sizes,
    parse_catch2_xml,
    render_markdown_table,
    render_vegalite_spec,
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
  <OverallResults successes="0" failures="0" expectedFailures="0" skips="0"/>
  <OverallResultsCases successes="2" failures="0" expectedFailures="0" skips="0"/>
</Catch2TestRun>
"""

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
def unknown_xml_path(tmp_path: Path) -> Path:
    p = tmp_path / "unknown.xml"
    p.write_text(UNKNOWN_CORPUS_XML, encoding="utf-8")
    return p


# ---------------------------------------------------------------------------
# parse_catch2_xml
# ---------------------------------------------------------------------------


def test_parse_returns_correct_count(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    assert len(results) == 3


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


# ---------------------------------------------------------------------------
# infer_corpus_file
# ---------------------------------------------------------------------------


def test_infer_en_mars_stem() -> None:
    assert infer_corpus_file("whatwg_decode utf_8 en_mars corpus") == "en_mars_utf8.txt"


def test_infer_ar_mars_stem() -> None:
    assert infer_corpus_file("ar_mars benchmark") == "ar_mars_utf8.txt"


def test_infer_ru_mars_stem() -> None:
    assert infer_corpus_file("ru_mars something") == "ru_mars_utf8.txt"


def test_infer_ja_mars_stem() -> None:
    assert infer_corpus_file("ja_mars utf8 bench") == "ja_mars_utf8.txt"


def test_infer_english_keyword() -> None:
    assert (
        infer_corpus_file("UTF-8 decode: English (ASCII-heavy)") == "en_mars_utf8.txt"
    )


def test_infer_arabic_keyword() -> None:
    assert (
        infer_corpus_file("UTF-8 decode: Arabic (multibyte-heavy)")
        == "ar_mars_utf8.txt"
    )


def test_infer_russian_keyword() -> None:
    assert infer_corpus_file("decode: Russian text") == "ru_mars_utf8.txt"


def test_infer_windows_1251_takes_priority_over_russian() -> None:
    # "windows-1251 Russian" — more specific hint wins
    result = infer_corpus_file("Single-byte decode: windows-1251 Russian")
    assert result == "ru_mars_windows1251.bin"


def test_infer_shift_jis() -> None:
    assert (
        infer_corpus_file("Multi-byte decode: Shift-JIS Japanese")
        == "ja_mars_shiftjis.bin"
    )


def test_infer_unknown_returns_none() -> None:
    assert infer_corpus_file("mystery benchmark with no corpus hint") is None


def test_corpus_hints_list_is_non_empty() -> None:
    assert len(CORPUS_NAME_HINTS) > 0


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
# load_corpus_sizes
# ---------------------------------------------------------------------------


def test_load_from_corpus_dir(tmp_path: Path) -> None:
    (tmp_path / "en_mars_utf8.txt").write_bytes(b"x" * 100)
    sizes = load_corpus_sizes(corpus_dir=tmp_path)
    assert sizes["en_mars_utf8.txt"] == 100


def test_load_from_manifest(tmp_path: Path) -> None:
    manifest = tmp_path / "corpus_manifest.json"
    manifest.write_text(
        json.dumps(
            [
                {
                    "file": "en_mars_utf8.txt",
                    "lang": "en",
                    "encoding": "utf-8",
                    "source_url": "https://example.com",
                    "bytes": 999,
                    "sha256": "abc",
                }
            ]
        ),
        encoding="utf-8",
    )
    sizes = load_corpus_sizes(manifest_path=manifest)
    assert sizes["en_mars_utf8.txt"] == 999


def test_manifest_overrides_corpus_dir(tmp_path: Path) -> None:
    corpus_dir = tmp_path / "corpus"
    corpus_dir.mkdir()
    (corpus_dir / "en_mars_utf8.txt").write_bytes(b"x" * 100)
    manifest = tmp_path / "corpus_manifest.json"
    manifest.write_text(
        json.dumps(
            [
                {
                    "file": "en_mars_utf8.txt",
                    "lang": "en",
                    "encoding": "utf-8",
                    "source_url": "https://example.com",
                    "bytes": 9999,
                    "sha256": "abc",
                }
            ]
        ),
        encoding="utf-8",
    )
    sizes = load_corpus_sizes(manifest_path=manifest, corpus_dir=corpus_dir)
    assert sizes["en_mars_utf8.txt"] == 9999


def test_load_missing_manifest_is_ignored(tmp_path: Path) -> None:
    sizes = load_corpus_sizes(manifest_path=tmp_path / "nonexistent.json")
    assert sizes == {}


def test_load_missing_corpus_dir_is_ignored(tmp_path: Path) -> None:
    sizes = load_corpus_sizes(corpus_dir=tmp_path / "nonexistent")
    assert sizes == {}


# ---------------------------------------------------------------------------
# render_markdown_table
# ---------------------------------------------------------------------------


def test_render_markdown_contains_header(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    table = render_markdown_table(results, {})
    assert "## Benchmark Results" in table


def test_render_markdown_contains_benchmark_name(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    table = render_markdown_table(results, {})
    assert "en_mars corpus" in table


def test_render_markdown_shows_throughput_when_size_known(
    sample_xml_path: Path,
) -> None:
    results = parse_catch2_xml(sample_xml_path)
    sizes = {"en_mars_utf8.txt": 586}
    table = render_markdown_table(results, sizes)
    # should contain a non-? throughput value for en_mars
    lines = [ln for ln in table.splitlines() if "en_mars corpus" in ln]
    assert lines
    # the throughput column should not be "?" for a known corpus
    assert "| ? |" not in lines[0]


def test_render_markdown_shows_question_mark_for_unknown_corpus(
    unknown_xml_path: Path,
) -> None:
    results = parse_catch2_xml(unknown_xml_path)
    table = render_markdown_table(results, {})
    assert "| ? |" in table


def test_render_markdown_label_appears_in_heading(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    table = render_markdown_table(results, {}, label="GCC -O3 -flto")
    assert "GCC -O3 -flto" in table


def test_render_markdown_contains_formula_note(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    table = render_markdown_table(results, {})
    assert "MiB" in table


# ---------------------------------------------------------------------------
# render_vegalite_spec
# ---------------------------------------------------------------------------


def test_vegalite_spec_schema(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    sizes = {
        "en_mars_utf8.txt": 586,
        "ar_mars_utf8.txt": 640,
        "ru_mars_windows1251.bin": 435,
    }
    spec = render_vegalite_spec(results, sizes)
    assert "$schema" in spec
    assert "vega-lite" in str(spec["$schema"])


def test_vegalite_spec_has_data(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    sizes = {"en_mars_utf8.txt": 586}
    spec = render_vegalite_spec(results, sizes)
    values = spec["data"]["values"]
    assert len(values) >= 1


def test_vegalite_omits_unknown_corpus(unknown_xml_path: Path) -> None:
    results = parse_catch2_xml(unknown_xml_path)
    spec = render_vegalite_spec(results, {})
    values = spec["data"]["values"]
    assert len(values) == 0


def test_vegalite_throughput_values_are_positive(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    sizes = {
        "en_mars_utf8.txt": 586,
        "ar_mars_utf8.txt": 640,
        "ru_mars_windows1251.bin": 435,
    }
    spec = render_vegalite_spec(results, sizes)
    values = spec["data"]["values"]
    for entry in values:
        assert entry["throughput_MiBs"] > 0


def test_vegalite_spec_has_mark_bar(sample_xml_path: Path) -> None:
    results = parse_catch2_xml(sample_xml_path)
    spec = render_vegalite_spec(results, {})
    assert spec["mark"] == "bar"
