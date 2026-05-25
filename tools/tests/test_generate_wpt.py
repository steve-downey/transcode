# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/generate_wpt_vectors.py."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from generate_wpt_vectors import (
    parse_bom_vectors,
    parse_encoder_surrogates_vectors,
    parse_eof_vectors,
    parse_fatal_single_byte_cases,
    parse_fatal_vectors,
    parse_gb18030_decode_vectors,
    parse_iso2022jp_decode_vectors,
    parse_js_string,
    parse_single_byte_indexes,
    parse_surrogates_utf8_vectors,
    parse_utf8_mistake_vectors,
    parse_utf16_surrogate_vectors,
    render_bom_vectors_hpp,
    render_encoder_surrogates_vectors_hpp,
    render_eof_vectors_hpp,
    render_fatal_single_byte_vectors_hpp,
    render_fatal_vectors_hpp,
    render_gb18030_vectors_hpp,
    render_iso2022jp_vectors_hpp,
    render_single_byte_vectors_hpp,
    render_surrogates_utf8_vectors_hpp,
    render_utf8_vectors_hpp,
    render_utf16_surrogates_vectors_hpp,
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


def test_parse_js_string_surrogate_pair() -> None:
    assert parse_js_string("\\uD834\\uDD1E") == [0x1D11E]


def test_parse_js_string_surrogate_pair_private() -> None:
    assert parse_js_string("\\uDBFF\\uDFFD") == [0x10FFFD]


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


def test_parse_js_string_hex_escape() -> None:
    assert parse_js_string("\\x0D") == [0x0D]


def test_parse_js_string_literal_multibyte() -> None:
    assert parse_js_string("¥") == [0xA5]


def test_parse_iso2022jp_decode_vectors() -> None:
    content = 'decode([0x1b, 0x24], "\\ufffd$", "Error ESC")'
    vectors = parse_iso2022jp_decode_vectors(content)
    assert len(vectors) == 1
    assert vectors[0]["input"] == [0x1B, 0x24]
    assert vectors[0]["expected"] == [0xFFFD, 0x24]
    assert vectors[0]["description"] == "Error ESC"


def test_parse_iso2022jp_literal_utf8() -> None:
    content = (
        'decode([0x1b, 0x28, 0x4A, 0x5C, 0x5D, 0x7E], "¥]‾", "Roman ESC, characters")'
    )
    vectors = parse_iso2022jp_decode_vectors(content)
    assert vectors[0]["expected"] == [0xA5, 0x5D, 0x203E]


def test_parse_single_byte_indexes() -> None:
    content = """
    singleByteIndexes = {
  "IBM866":[1040,1041,null,1043],
  "ISO-8859-2":[128,129,130,131]
}
"""
    indexes = parse_single_byte_indexes(content)
    assert len(indexes) == 2
    assert indexes[0]["name"] == "IBM866"
    assert indexes[0]["codepoints"] == [1040, 1041, 0xFFFD, 1043]
    assert indexes[1]["name"] == "ISO-8859-2"
    assert indexes[1]["codepoints"] == [128, 129, 130, 131]


def test_render_iso2022jp_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"input": [0x1B, 0x24], "expected": [0xFFFD, 0x24], "description": "test"},
    ]
    out = tmp_path / "wpt_iso2022jp_vectors.hpp"
    render_iso2022jp_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "iso2022jp_wpt_decode_vectors" in content
    assert "0x1B" in content


def test_render_single_byte_vectors_hpp(tmp_path: Path) -> None:
    indexes: list[dict[str, object]] = [
        {"name": "IBM866", "codepoints": [1040, 1041, 0xFFFD, 1043]},
    ]
    out = tmp_path / "wpt_single_byte_vectors.hpp"
    render_single_byte_vectors_hpp(indexes, out)
    content = out.read_text()
    assert "wpt_single_byte_indexes" in content
    assert "IBM866" in content


_UTF16_SURR_SAMPLE = """
var bad = [
    {
        encoding: 'utf-16le',
        input: [0x00, 0xd8],
        expected: '\\uFFFD',
        name: 'lone surrogate lead'
    },
    {
        encoding: 'utf-16le',
        input: [0x00, 0xdc, 0x00, 0x00],
        expected: '\\uFFFD\\u0000',
        name: 'unmatched surrogate trail'
    }
];
"""


def test_parse_utf16_surrogate_vectors_count() -> None:
    vectors = parse_utf16_surrogate_vectors(_UTF16_SURR_SAMPLE)
    assert len(vectors) == 2


def test_parse_utf16_surrogate_vectors_first() -> None:
    vectors = parse_utf16_surrogate_vectors(_UTF16_SURR_SAMPLE)
    assert vectors[0]["encoding"] == "utf-16le"
    assert vectors[0]["input"] == [0x00, 0xD8]
    assert vectors[0]["expected"] == [0xFFFD]
    assert vectors[0]["description"] == "lone surrogate lead"


def test_parse_utf16_surrogate_vectors_multi_expected() -> None:
    vectors = parse_utf16_surrogate_vectors(_UTF16_SURR_SAMPLE)
    assert vectors[1]["input"] == [0x00, 0xDC, 0x00, 0x00]
    assert vectors[1]["expected"] == [0xFFFD, 0x0000]
    assert vectors[1]["description"] == "unmatched surrogate trail"


def test_render_utf16_surrogates_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {
            "encoding": "utf-16le",
            "input": [0x00, 0xD8],
            "expected": [0xFFFD],
            "description": "lone surrogate lead",
        },
    ]
    out = tmp_path / "wpt_utf16_surrogates_vectors.hpp"
    render_utf16_surrogates_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP" in content
    assert "utf16le_surrogate_wpt_decode_vectors" in content
    assert "0x00, 0xD8" in content
    assert "0xFFFD" in content
    assert "lone surrogate lead" in content


_FATAL_SAMPLE = """
var bad = [
    { encoding: 'utf-8', input: [0xFF], name: 'invalid code' },
    { encoding: 'utf-8', input: [0xC0], name: 'ends early' },
    { encoding: 'utf-16le', input: [0x00], name: 'truncated code unit' },
];
"""


def test_parse_fatal_vectors_count() -> None:
    vectors = parse_fatal_vectors(_FATAL_SAMPLE)
    assert len(vectors) == 3


def test_parse_fatal_vectors_utf8() -> None:
    vectors = parse_fatal_vectors(_FATAL_SAMPLE)
    assert vectors[0]["encoding"] == "utf-8"
    assert vectors[0]["input"] == [0xFF]
    assert vectors[0]["description"] == "invalid code"


def test_parse_fatal_vectors_utf16le() -> None:
    vectors = parse_fatal_vectors(_FATAL_SAMPLE)
    assert vectors[2]["encoding"] == "utf-16le"
    assert vectors[2]["input"] == [0x00]
    assert vectors[2]["description"] == "truncated code unit"


_BOM_SAMPLE = """
var testCases = [
    {
        encoding: 'utf-8',
        bom: [0xEF, 0xBB, 0xBF],
        bytes: [0x7A, 0xC2, 0xA2]
    },
    {
        encoding: 'utf-16le',
        bom: [0xff, 0xfe],
        bytes: [0x7A, 0x00]
    }
];
var string = 'z\\xA2\\uD834\\uDD1E';
"""


def test_parse_bom_vectors_count() -> None:
    vectors = parse_bom_vectors(_BOM_SAMPLE)
    assert len(vectors) == 2


def test_parse_bom_vectors_first() -> None:
    vectors = parse_bom_vectors(_BOM_SAMPLE)
    assert vectors[0]["encoding"] == "utf-8"
    assert vectors[0]["bom"] == [0xEF, 0xBB, 0xBF]
    assert vectors[0]["bytes"] == [0x7A, 0xC2, 0xA2]
    assert vectors[0]["expected"] == [0x7A, 0xA2, 0x1D11E]


def test_parse_bom_vectors_second() -> None:
    vectors = parse_bom_vectors(_BOM_SAMPLE)
    assert vectors[1]["encoding"] == "utf-16le"
    assert vectors[1]["bom"] == [0xFF, 0xFE]
    assert vectors[1]["bytes"] == [0x7A, 0x00]
    assert vectors[1]["expected"] == [0x7A, 0xA2, 0x1D11E]


def test_render_bom_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {
            "encoding": "utf-8",
            "bom": [0xEF, 0xBB, 0xBF],
            "bytes": [0x7A],
            "expected": [0x7A],
        },
    ]
    out = tmp_path / "wpt_bom_vectors.hpp"
    render_bom_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP" in content
    assert "wpt_bom_cases" in content
    assert "0xEF, 0xBB, 0xBF" in content
    assert "utf-8" in content


def test_render_fatal_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"encoding": "utf-8", "input": [0xFF], "description": "invalid code"},
        {"encoding": "utf-16le", "input": [0x00], "description": "truncated code unit"},
    ]
    out = tmp_path / "wpt_fatal_vectors.hpp"
    render_fatal_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP" in content
    assert "utf8_fatal_wpt_vectors" in content
    assert "utf16le_fatal_wpt_vectors" in content
    assert "0xFF" in content
    assert "invalid code" in content
    assert "truncated code unit" in content


_SBFATAL_SAMPLE = """
var singleByteEncodings = [
     {encoding: 'IBM866', bad: []},
     {encoding: 'ISO-8859-3', bad: [0xA5, 0xAE, 0xBE]},
     {encoding: 'ISO-8859-8-I', bad: [0xA1, 0xBF, 0xFF]},
];
"""


def test_parse_fatal_single_byte_cases_count() -> None:
    cases = parse_fatal_single_byte_cases(_SBFATAL_SAMPLE)
    assert len(cases) == 3


def test_parse_fatal_single_byte_cases_empty_bad() -> None:
    cases = parse_fatal_single_byte_cases(_SBFATAL_SAMPLE)
    assert cases[0]["encoding"] == "IBM866"
    assert cases[0]["bad"] == []


def test_parse_fatal_single_byte_cases_with_bad() -> None:
    cases = parse_fatal_single_byte_cases(_SBFATAL_SAMPLE)
    assert cases[1]["encoding"] == "ISO-8859-3"
    assert cases[1]["bad"] == [0xA5, 0xAE, 0xBE]


def test_parse_fatal_single_byte_cases_hyphen_name() -> None:
    cases = parse_fatal_single_byte_cases(_SBFATAL_SAMPLE)
    assert cases[2]["encoding"] == "ISO-8859-8-I"
    assert cases[2]["bad"] == [0xA1, 0xBF, 0xFF]


def test_render_fatal_single_byte_vectors_hpp(tmp_path: Path) -> None:
    cases: list[dict[str, object]] = [
        {"encoding": "IBM866", "bad": []},
        {"encoding": "ISO-8859-3", "bad": [0xA5, 0xAE]},
    ]
    out = tmp_path / "wpt_fatal_single_byte_vectors.hpp"
    render_fatal_single_byte_vectors_hpp(cases, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_FATAL_SINGLE_BYTE_VECTORS_HPP" in content
    assert "WptFatalSingleByteCase" in content
    assert "wpt_fatal_single_byte_cases" in content
    assert "IBM866" in content
    assert "ISO-8859-3" in content
    assert "0xA5" in content


_EOF_SAMPLE = """\
test(() => {
  assert_equals(new TextDecoder().decode(new Uint8Array([0xF0])), "\\uFFFD");
  assert_equals(new TextDecoder("Big5").decode(new Uint8Array([0x81])), "\\uFFFD");
}, "TextDecoder end-of-queue handling");

test(() => {
  assert_equals(new TextDecoder().decode(new Uint8Array([0x41])), "A");
}, "TextDecoder end-of-queue handling using stream: true");
"""


def test_parse_eof_vectors_count() -> None:
    vectors = parse_eof_vectors(_EOF_SAMPLE)
    assert len(vectors) == 2


def test_parse_eof_vectors_utf8_default_encoding() -> None:
    vectors = parse_eof_vectors(_EOF_SAMPLE)
    assert vectors[0]["encoding"] == "utf-8"
    assert vectors[0]["input"] == [0xF0]
    assert vectors[0]["expected"] == [0xFFFD]


def test_parse_eof_vectors_big5() -> None:
    vectors = parse_eof_vectors(_EOF_SAMPLE)
    assert vectors[1]["encoding"] == "Big5"
    assert vectors[1]["input"] == [0x81]
    assert vectors[1]["expected"] == [0xFFFD]


def test_parse_eof_vectors_skips_streaming() -> None:
    vectors = parse_eof_vectors(_EOF_SAMPLE)
    # streaming block is excluded; only 2 non-streaming cases in sample
    assert len(vectors) == 2


def test_render_eof_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"encoding": "utf-8", "input": [0xF0], "expected": [0xFFFD]},
        {"encoding": "Big5", "input": [0x81, 0x40], "expected": [0xFFFD, 0x0040]},
    ]
    out = tmp_path / "wpt_eof_vectors.hpp"
    render_eof_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP" in content
    assert "WptEofVector" in content
    assert "wpt_eof_vectors" in content
    assert "utf-8" in content
    assert "Big5" in content
    assert "0xFFFD" in content
    assert "wpt_eof_vectors[]" in content


_SURR_SAMPLE = """\
var badStrings = [
    { input: 'a', expected: [0x61], decoded: 'a', name: 'ASCII' },
    { input: '\\uD800', expected: [0xef, 0xbf, 0xbd], decoded: '\\uFFFD',
      name: 'Lone high surrogate' },
];
"""


def test_parse_surrogates_utf8_count() -> None:
    vectors = parse_surrogates_utf8_vectors(_SURR_SAMPLE)
    assert len(vectors) == 2


def test_parse_surrogates_utf8_ascii() -> None:
    vectors = parse_surrogates_utf8_vectors(_SURR_SAMPLE)
    assert vectors[0]["input"] == [0x61]
    assert vectors[0]["expected"] == [0x61]
    assert vectors[0]["decoded"] == [0x61]


def test_parse_surrogates_utf8_surrogate() -> None:
    vectors = parse_surrogates_utf8_vectors(_SURR_SAMPLE)
    assert vectors[1]["input"] == [0xD800]
    assert vectors[1]["expected"] == [0xEF, 0xBF, 0xBD]
    assert vectors[1]["decoded"] == [0xFFFD]


def test_render_surrogates_utf8_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"input": [0x61], "expected": [0x61], "decoded": [0x61], "name": "a"},
    ]
    out = tmp_path / "wpt_surrogates_utf8_vectors.hpp"
    render_surrogates_utf8_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_SURROGATES_UTF8_VECTORS_HPP" in content
    assert "WptSurrogatesUtf8Vector" in content
    assert "wpt_surrogates_utf8_vectors" in content


_ENCODER_SURR_SAMPLE = """\
var bad = [
    { input: '\\uD800', expected: '\\uFFFD', name: 'lone surrogate lead' },
    { input: '\\uD834\\uDD1E', expected: '\\uD834\\uDD1E',
      name: 'proper pair' },
];
"""


def test_parse_encoder_surrogates_count() -> None:
    vectors = parse_encoder_surrogates_vectors(_ENCODER_SURR_SAMPLE)
    assert len(vectors) == 2


def test_parse_encoder_surrogates_lone() -> None:
    vectors = parse_encoder_surrogates_vectors(_ENCODER_SURR_SAMPLE)
    assert vectors[0]["input"] == [0xD800]
    assert vectors[0]["expected"] == [0xFFFD]


def test_parse_encoder_surrogates_pair() -> None:
    vectors = parse_encoder_surrogates_vectors(_ENCODER_SURR_SAMPLE)
    assert vectors[1]["input"] == [0x1D11E]
    assert vectors[1]["expected"] == [0x1D11E]


def test_render_encoder_surrogates_vectors_hpp(tmp_path: Path) -> None:
    vectors: list[dict[str, object]] = [
        {"input": [0xD800], "expected": [0xFFFD], "name": "lone"},
    ]
    out = tmp_path / "wpt_encoder_surrogates_vectors.hpp"
    render_encoder_surrogates_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "#ifndef TESTS_BEMAN_TRANSCODE_WPT_ENCODER_SURROGATES_VECTORS_HPP" in content
    assert "WptEncoderSurrogatesVector" in content
    assert "wpt_encoder_surrogates_vectors" in content
