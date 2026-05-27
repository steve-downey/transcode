# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/download_benchmark_corpora.py."""

import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from download_benchmark_corpora import (
    CONVERSIONS,
    CORPORA,
    CorpusEntry,
    build_manifest_entry,
    convert_encoding,
    extract_article_text,
    sha256_of,
    write_manifest_json,
    write_source_md,
)


def test_sha256_of_known_value() -> None:
    data = b"hello"
    expected = hashlib.sha256(b"hello").hexdigest()
    assert sha256_of(data) == expected


def test_sha256_of_empty() -> None:
    expected = hashlib.sha256(b"").hexdigest()
    assert sha256_of(b"") == expected


def test_sha256_returns_64_hex_chars() -> None:
    result = sha256_of(b"test data")
    assert len(result) == 64
    assert all(c in "0123456789abcdef" for c in result)


def test_corpora_contains_four_languages() -> None:
    langs = [lang for lang, _ in CORPORA]
    assert "en" in langs
    assert "ar" in langs
    assert "ru" in langs
    assert "ja" in langs
    assert len(CORPORA) == 4


def test_corpora_filenames_are_utf8() -> None:
    for _lang, filename in CORPORA:
        assert filename.endswith("_utf8.txt")


def test_conversions_contains_russian_and_japanese() -> None:
    encodings = [enc for _, _, enc in CONVERSIONS]
    assert "windows-1251" in encodings
    assert "shift_jis" in encodings
    assert len(CONVERSIONS) == 2


def test_extract_article_text_valid() -> None:
    payload = {
        "query": {
            "pages": {
                "123": {
                    "pageid": 123,
                    "title": "Mars",
                    "extract": "Mars is the fourth planet.",
                }
            }
        }
    }
    data = json.dumps(payload).encode("utf-8")
    assert extract_article_text(data) == "Mars is the fourth planet."


def test_extract_article_text_missing_extract() -> None:
    payload = {"query": {"pages": {"123": {"pageid": 123, "title": "Mars"}}}}
    data = json.dumps(payload).encode("utf-8")
    assert extract_article_text(data) == ""


def test_convert_encoding_windows1251_cyrillic() -> None:
    text = "Марс"
    result = convert_encoding(text, "windows-1251")
    assert isinstance(result, bytes)
    assert result == "Марс".encode("windows-1251")


def test_convert_encoding_shiftjis_katakana() -> None:
    text = "火星"
    result = convert_encoding(text, "shift_jis")
    assert isinstance(result, bytes)
    assert result == "火星".encode("shift_jis")


def test_convert_encoding_replaces_unmappable() -> None:
    # U+FFFD is not in windows-1251; should be replaced with ?
    text = "�"
    result = convert_encoding(text, "windows-1251")
    assert result == b"?"


def test_build_manifest_entry_fields() -> None:
    data = b"test content"
    entry = build_manifest_entry(
        "en_mars_utf8.txt", "en", "utf-8", "https://example.com/api", data
    )
    assert entry.file == "en_mars_utf8.txt"
    assert entry.lang == "en"
    assert entry.encoding == "utf-8"
    assert entry.source_url == "https://example.com/api"
    assert entry.bytes == len(data)
    assert entry.sha256 == sha256_of(data)


def test_build_manifest_entry_sha256_matches() -> None:
    data = b"some corpus text"
    entry = build_manifest_entry("f.txt", "en", "utf-8", "http://x", data)
    assert entry.sha256 == hashlib.sha256(data).hexdigest()


def test_write_manifest_json_schema(tmp_path: Path) -> None:
    entries = [
        CorpusEntry(
            file="en_mars_utf8.txt",
            lang="en",
            encoding="utf-8",
            source_url="https://en.wikipedia.org/w/api.php?...",
            bytes=1234,
            sha256="a" * 64,
        ),
        CorpusEntry(
            file="ru_mars_windows1251.bin",
            lang="ru",
            encoding="windows-1251",
            source_url="https://ru.wikipedia.org/w/api.php?...",
            bytes=567,
            sha256="b" * 64,
        ),
    ]
    write_manifest_json(tmp_path, entries)
    manifest_path = tmp_path / "corpus_manifest.json"
    assert manifest_path.exists()
    manifest = json.loads(manifest_path.read_text())
    assert isinstance(manifest, list)
    assert len(manifest) == 2
    first = manifest[0]
    assert first["file"] == "en_mars_utf8.txt"
    assert first["lang"] == "en"
    assert first["encoding"] == "utf-8"
    assert first["bytes"] == 1234
    assert first["sha256"] == "a" * 64
    assert "source_url" in first


def test_write_manifest_json_is_valid_json(tmp_path: Path) -> None:
    entries = [
        CorpusEntry(
            file="ja_mars_utf8.txt",
            lang="ja",
            encoding="utf-8",
            source_url="https://ja.wikipedia.org/w/api.php?...",
            bytes=999,
            sha256="c" * 64,
        )
    ]
    write_manifest_json(tmp_path, entries)
    content = (tmp_path / "corpus_manifest.json").read_text()
    parsed = json.loads(content)
    assert parsed[0]["lang"] == "ja"


def test_write_source_md_contains_license(tmp_path: Path) -> None:
    entries = [
        CorpusEntry(
            file="en_mars_utf8.txt",
            lang="en",
            encoding="utf-8",
            source_url="https://en.wikipedia.org/w/api.php?...",
            bytes=500,
            sha256="d" * 64,
        )
    ]
    write_source_md(tmp_path, entries)
    content = (tmp_path / "SOURCE.md").read_text()
    assert "CC BY-SA 4.0" in content
    assert "Wikipedia" in content
    assert "en_mars_utf8.txt" in content


def test_write_source_md_contains_full_checksum(tmp_path: Path) -> None:
    sha = "e" * 64
    entries = [
        CorpusEntry(
            file="ar_mars_utf8.txt",
            lang="ar",
            encoding="utf-8",
            source_url="https://ar.wikipedia.org/w/api.php?...",
            bytes=200,
            sha256=sha,
        )
    ]
    write_source_md(tmp_path, entries)
    content = (tmp_path / "SOURCE.md").read_text()
    assert sha in content


def test_write_source_md_contains_all_files(tmp_path: Path) -> None:
    entries = [
        CorpusEntry(f, "en", "utf-8", "http://x", 100, "a" * 64)
        for f in ["en_mars_utf8.txt", "ru_mars_windows1251.bin"]
    ]
    write_source_md(tmp_path, entries)
    content = (tmp_path / "SOURCE.md").read_text()
    assert "en_mars_utf8.txt" in content
    assert "ru_mars_windows1251.bin" in content
