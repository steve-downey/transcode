# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/generate_wpt_vectors.py."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from generate_wpt_vectors import (
    parse_gb18030_decode_vectors,
    parse_js_string,
    parse_utf8_mistake_vectors,
    render_gb18030_vectors_hpp,
    render_utf8_vectors_hpp,
)


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


def test_parse_utf8_mistake_vectors() -> None:
    content = """
const invalid8 = [
    { bytes: [0x80], charcodes: [0xff_fd] },
    { bytes: [0xf0, 0x90, 0x80], charcodes: [0xff_fd] },
    { bytes: [0xf0, 0x80, 0x80], charcodes: [0xff_fd, 0xff_fd, 0xff_fd] },
]
"""
    vectors = parse_utf8_mistake_vectors(content)
    assert len(vectors) == 3
    assert vectors[0]["input"] == [0x80]
    assert vectors[0]["expected"] == [0xFFFD]
    assert vectors[1]["input"] == [0xF0, 0x90, 0x80]
    assert vectors[1]["expected"] == [0xFFFD]
    assert vectors[2]["input"] == [0xF0, 0x80, 0x80]
    assert vectors[2]["expected"] == [0xFFFD, 0xFFFD, 0xFFFD]


def test_parse_utf8_mistake_vectors_with_zero() -> None:
    content = """
const invalid8 = [
    { bytes: [0, 254, 255], charcodes: [0, 0xff_fd, 0xff_fd] },
]
"""
    vectors = parse_utf8_mistake_vectors(content)
    assert len(vectors) == 1
    assert vectors[0]["input"] == [0, 254, 255]
    assert vectors[0]["expected"] == [0, 0xFFFD, 0xFFFD]


def test_render_gb18030_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"input": [0x73], "expected": [0x73], "description": "ASCII"},
        {"input": [0x80], "expected": [0x20AC], "description": "euro"},
    ]
    out = tmp_path / "wpt_gb18030_vectors.hpp"
    render_gb18030_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_GB18030_VECTORS_HPP" in content
    assert "gb18030_wpt_decode_vectors" in content
    assert "{0x73}" in content
    assert "0x20AC" in content


def test_render_utf8_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"input": [0x80], "expected": [0xFFFD], "description": "invalid"},
    ]
    out = tmp_path / "wpt_utf8_vectors.hpp"
    render_utf8_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP" in content
    assert "utf8_wpt_decode_vectors" in content
    assert "0xFFFD" in content
