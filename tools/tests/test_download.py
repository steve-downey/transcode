# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/download_indexes.py."""

import hashlib
import json
from datetime import date
from pathlib import Path

import pytest

import sys

sys.path.insert(0, str(Path(__file__).parent.parent))

from download_indexes import (
    EXTRA_FILES,
    MULTI_BYTE_INDEXES,
    SINGLE_BYTE_INDEXES,
    sha256_of,
    write_source_bib,
    write_source_md,
    validate_encodings_json,
)


def test_sha256_of_known_value() -> None:
    data = b"hello"
    expected = hashlib.sha256(b"hello").hexdigest()
    assert sha256_of(data) == expected


def test_single_byte_indexes_count() -> None:
    # 28 single-byte encodings minus iso-8859-8-i (shares iso-8859-8 index)
    assert len(SINGLE_BYTE_INDEXES) == 27


def test_single_byte_indexes_contains_windows_1252() -> None:
    assert "windows-1252" in SINGLE_BYTE_INDEXES


def test_single_byte_indexes_contains_iso_8859_2() -> None:
    assert "iso-8859-2" in SINGLE_BYTE_INDEXES


def test_single_byte_indexes_no_iso_8859_1() -> None:
    # iso-8859-1 is not a separate WHATWG index; it maps to windows-1252
    assert "iso-8859-1" not in SINGLE_BYTE_INDEXES


def test_multi_byte_indexes_count() -> None:
    assert len(MULTI_BYTE_INDEXES) == 7


def test_extra_files_contains_encodings_json() -> None:
    assert "encodings.json" in EXTRA_FILES


def test_write_source_md(tmp_path: Path) -> None:
    results = {
        "index-windows-1252.txt": (
            "https://encoding.spec.whatwg.org/index-windows-1252.txt",
            "a" * 64,
            True,
        ),
        "encodings.json": (
            "https://encoding.spec.whatwg.org/encodings.json",
            "b" * 64,
            False,
        ),
    }
    retrieved = date(2026, 5, 23)
    write_source_md(tmp_path, results, retrieved)

    source_md = (tmp_path / "SOURCE.md").read_text()
    assert "2026-05-23" in source_md
    assert "CC-BY 4.0" in source_md
    assert "WHATWG Encoding Standard" in source_md
    assert "index-windows-1252.txt" in source_md
    assert "encodings.json" in source_md
    assert ("a" * 16) in source_md


def test_write_source_md_contains_full_checksums(tmp_path: Path) -> None:
    sha = "c" * 64
    results = {
        "index-ibm866.txt": (
            "https://encoding.spec.whatwg.org/index-ibm866.txt",
            sha,
            True,
        ),
    }
    write_source_md(tmp_path, results, date(2026, 5, 23))
    source_md = (tmp_path / "SOURCE.md").read_text()
    assert sha in source_md


def test_write_source_bib(tmp_path: Path) -> None:
    write_source_bib(tmp_path, date(2026, 5, 23))
    bib = (tmp_path / "source.bib").read_text()
    assert "whatwg-encoding" in bib
    assert "2026-05-23" in bib
    assert "CC-BY-4.0" in bib
    assert "encoding.spec.whatwg.org" in bib


def test_validate_encodings_json_valid(tmp_path: Path) -> None:
    data = [{"heading": "Unicode", "encodings": [{"name": "UTF-8", "labels": []}]}]
    (tmp_path / "encodings.json").write_text(json.dumps(data))
    validate_encodings_json(tmp_path)  # should not raise


def test_validate_encodings_json_missing(tmp_path: Path) -> None:
    with pytest.raises(FileNotFoundError):
        validate_encodings_json(tmp_path)


def test_validate_encodings_json_not_list(tmp_path: Path) -> None:
    (tmp_path / "encodings.json").write_text(json.dumps({"key": "val"}))
    with pytest.raises(ValueError, match="expected top-level list"):
        validate_encodings_json(tmp_path)
