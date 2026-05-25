# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/generate_labels.py."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from generate_labels import name_to_codec_value, parse_encodings_json, render_labels_hpp

ENCODINGS_JSON = Path(__file__).parent.parent.parent / "docs" / "whatwg" / "encodings.json"


def test_name_to_codec_value_utf8() -> None:
    assert name_to_codec_value("UTF-8") == "utf_8"


def test_name_to_codec_value_shift_jis() -> None:
    assert name_to_codec_value("Shift_JIS") == "shift_jis"


def test_name_to_codec_value_iso2022jp() -> None:
    assert name_to_codec_value("ISO-2022-JP") == "iso_2022_jp"


def test_name_to_codec_value_x_user_defined() -> None:
    assert name_to_codec_value("x-user-defined") == "x_user_defined"


def test_parse_encodings_json_count() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    # 228 labels total in the WHATWG encodings.json
    assert len(entries) == 228


def test_parse_encodings_json_sorted() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    labels = [e[0] for e in entries]
    assert labels == sorted(labels)


def test_parse_encodings_json_utf8_labels() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    utf8_entries = [(lbl, val) for lbl, val in entries if val == "utf_8"]
    labels = [e[0] for e in utf8_entries]
    assert "utf-8" in labels
    assert "utf8" in labels
    assert "unicode-1-1-utf-8" in labels


def test_parse_encodings_json_shift_jis_labels() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    sj_entries = [(lbl, val) for lbl, val in entries if val == "shift_jis"]
    labels = [e[0] for e in sj_entries]
    assert "sjis" in labels
    assert "shift_jis" in labels
    assert "x-sjis" in labels


def test_render_labels_hpp_has_guard() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    hpp = render_labels_hpp(entries)
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_LABELS_HPP" in hpp


def test_render_labels_hpp_has_function() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    hpp = render_labels_hpp(entries)
    assert "get_encoding" in hpp


def test_render_labels_hpp_has_utf8_entry() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    hpp = render_labels_hpp(entries)
    assert '"utf-8", codec::utf_8' in hpp


def test_render_labels_hpp_has_shift_jis_entry() -> None:
    entries = parse_encodings_json(ENCODINGS_JSON)
    hpp = render_labels_hpp(entries)
    assert '"shift_jis", codec::shift_jis' in hpp
