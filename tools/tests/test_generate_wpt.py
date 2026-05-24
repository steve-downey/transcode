# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/generate_wpt_vectors.py."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from generate_wpt_vectors import parse_js_string, parse_gb18030_decode_vectors


def test_parse_js_string_ascii() -> None:
    assert parse_js_string("s") == [0x73]


def test_parse_js_string_unicode_escape_4() -> None:
    assert parse_js_string("\\u20AC") == [0x20AC]


def test_parse_js_string_unicode_escape_brace() -> None:
    assert parse_js_string("\\u{10000}") == [0x10000]


def test_parse_js_string_replacement() -> None:
    assert parse_js_string("\\ufffd") == [0xFFFD]


def test_parse_js_string_multi() -> None:
    assert parse_js_string("\\ufffd(") == [0xFFFD, 0x28]


def test_parse_js_string_multi_surrogates() -> None:
    assert parse_js_string("\\ufffd\\u007f") == [0xFFFD, 0x7F]


def test_parse_js_string_supplementary() -> None:
    assert parse_js_string("\\u{10FFFF}") == [0x10FFFF]


def test_parse_gb18030_decode_vectors_ascii() -> None:
    content = 'decode([115], "s", "ASCII");'
    vectors = parse_gb18030_decode_vectors(content)
    assert len(vectors) == 1
    assert vectors[0]["input"] == [0x73]
    assert vectors[0]["expected"] == [0x73]
    assert vectors[0]["description"] == "ASCII"


def test_parse_gb18030_decode_vectors_euro() -> None:
    content = 'decode([0x80], "\\u20AC", "euro");'
    vectors = parse_gb18030_decode_vectors(content)
    assert len(vectors) == 1
    assert vectors[0]["input"] == [0x80]
    assert vectors[0]["expected"] == [0x20AC]
    assert vectors[0]["description"] == "euro"


def test_parse_gb18030_decode_vectors_replacement() -> None:
    content = 'decode([0xFF], "\\uFFFD", "initial byte out of accepted ranges");'
    vectors = parse_gb18030_decode_vectors(content)
    assert len(vectors) == 1
    assert vectors[0]["expected"] == [0xFFFD]


def test_parse_gb18030_decode_vectors_multi_expected() -> None:
    content = 'decode([0x81, 0x28], "\\ufffd(", "two bytes 0x81 0x28");'
    vectors = parse_gb18030_decode_vectors(content)
    assert vectors[0]["input"] == [0x81, 0x28]
    assert vectors[0]["expected"] == [0xFFFD, 0x28]


def test_parse_gb18030_decode_vectors_supplementary() -> None:
    content = 'decode([0x90, 0x30, 0x81, 0x30], "\\u{10000}", "pointer 189000");'
    vectors = parse_gb18030_decode_vectors(content)
    assert vectors[0]["input"] == [0x90, 0x30, 0x81, 0x30]
    assert vectors[0]["expected"] == [0x10000]


def test_parse_gb18030_decode_vectors_no_description() -> None:
    content = 'decode([0xA3, 0xA0], "\\u3000");'
    vectors = parse_gb18030_decode_vectors(content)
    assert len(vectors) == 1
    assert vectors[0]["input"] == [0xA3, 0xA0]
    assert vectors[0]["expected"] == [0x3000]


def test_parse_gb18030_decode_vectors_multiple() -> None:
    content = """
decode([115], "s", "ASCII");
decode([0x80], "\\u20AC", "euro");
decode([0xFF], "\\uFFFD", "initial byte out of accepted ranges");
"""
    vectors = parse_gb18030_decode_vectors(content)
    assert len(vectors) == 3
    assert vectors[0]["input"] == [0x73]
    assert vectors[1]["input"] == [0x80]
    assert vectors[2]["input"] == [0xFF]
