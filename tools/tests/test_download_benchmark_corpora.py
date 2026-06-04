# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/download_benchmark_corpora.py."""

import hashlib
import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from download_benchmark_corpora import (
    CONVERSIONS,
    GUTENBERG_WAR_AND_PEACE_FILE,
    GUTENBERG_WAR_AND_PEACE_URL,
    WIKI_CORPORA,
    CorpusEntry,
    build_manifest_entry,
    convert_encoding,
    derive_utf16,
    extract_article_text,
    sha256_of,
    write_manifest_json,
    write_source_md,
)

# ---------------------------------------------------------------------------
# sha256_of
# ---------------------------------------------------------------------------


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


# ---------------------------------------------------------------------------
# WIKI_CORPORA / CONVERSIONS constants
# ---------------------------------------------------------------------------


def test_wiki_corpora_contains_six_languages() -> None:
    langs = [lang for lang, *_ in WIKI_CORPORA]
    assert "en" in langs
    assert "ar" in langs
    assert "ru" in langs
    assert "ja" in langs
    assert "zh" in langs
    assert "ko" in langs
    assert len(WIKI_CORPORA) == 6


def test_wiki_corpora_filenames_are_utf8() -> None:
    for _lang, filename, *_ in WIKI_CORPORA:
        assert filename.endswith("_utf8.txt")


def test_wiki_corpora_zh_uses_tw_variant() -> None:
    zh_entries = [e for e in WIKI_CORPORA if e[0] == "zh"]
    assert zh_entries, "zh entry missing"
    extra = zh_entries[0][3]
    assert "zh-tw" in extra, "zh should use zh-tw variant for Big5 compatibility"


def test_wiki_corpora_ko_uses_korean_title() -> None:
    ko_entries = [e for e in WIKI_CORPORA if e[0] == "ko"]
    assert ko_entries, "ko entry missing"
    title = ko_entries[0][2]
    assert title != "Mars", "ko should use Korean title, not 'Mars'"


def test_wiki_corpora_ar_uses_arabic_title() -> None:
    ar_entries = [e for e in WIKI_CORPORA if e[0] == "ar"]
    assert ar_entries, "ar entry missing"
    title = ar_entries[0][2]
    assert title == "المريخ", f"ar should use Arabic title 'المريخ', got {title!r}"


def test_wiki_corpora_ja_uses_japanese_title() -> None:
    ja_entries = [e for e in WIKI_CORPORA if e[0] == "ja"]
    assert ja_entries, "ja entry missing"
    title = ja_entries[0][2]
    assert title == "火星", f"ja should use Japanese title '火星', got {title!r}"


def test_conversions_contains_all_encodings() -> None:
    encodings = [enc for _, _, enc, _ in CONVERSIONS]
    assert "windows-1251" in encodings
    assert "shift_jis" in encodings
    assert "gb18030" in encodings
    assert "big5" in encodings
    assert "euc_kr" in encodings
    assert "euc_jp" in encodings
    assert "iso2022_jp" in encodings
    assert len(CONVERSIONS) == 7


def test_conversions_have_lang_codes() -> None:
    for _, _, _, lang in CONVERSIONS:
        assert len(lang) == 2


def test_gutenberg_constants_set() -> None:
    assert GUTENBERG_WAR_AND_PEACE_URL.startswith("https://")
    assert GUTENBERG_WAR_AND_PEACE_FILE.endswith(".txt")


# ---------------------------------------------------------------------------
# extract_article_text
# ---------------------------------------------------------------------------


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


# ---------------------------------------------------------------------------
# convert_encoding
# ---------------------------------------------------------------------------


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


def test_convert_encoding_gb18030_chinese() -> None:
    text = "火星"
    result = convert_encoding(text, "gb18030")
    assert result == "火星".encode("gb18030")


def test_convert_encoding_big5_traditional_chinese() -> None:
    text = "火星"
    result = convert_encoding(text, "big5")
    assert result == "火星".encode("big5")


def test_convert_encoding_euc_kr_korean() -> None:
    text = "화성"
    result = convert_encoding(text, "euc_kr")
    assert result == "화성".encode("euc_kr")


def test_convert_encoding_euc_jp_japanese() -> None:
    text = "火星"
    result = convert_encoding(text, "euc_jp")
    assert result == "火星".encode("euc_jp")


def test_convert_encoding_iso2022jp_japanese() -> None:
    text = "火星"
    result = convert_encoding(text, "iso2022_jp")
    assert isinstance(result, bytes)
    assert len(result) > 0
    # ISO-2022-JP uses escape sequences; result is longer than UTF-8
    assert b"\x1b" in result  # ESC character


def test_convert_encoding_replaces_unmappable() -> None:
    text = "�"  # U+FFFD replacement character not in windows-1251
    result = convert_encoding(text, "windows-1251")
    assert result == b"?"


# ---------------------------------------------------------------------------
# build_manifest_entry
# ---------------------------------------------------------------------------


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


# ---------------------------------------------------------------------------
# derive_utf16
# ---------------------------------------------------------------------------


def test_derive_utf16_creates_both_files(tmp_path: Path) -> None:
    text = "Hello, World! こんにちは"
    src = tmp_path / "test_utf8.txt"
    src.write_text(text, encoding="utf-8")
    le_entry, be_entry = derive_utf16(tmp_path, "test_utf8.txt")
    assert (tmp_path / "test_utf16le.bin").exists()
    assert (tmp_path / "test_utf16be.bin").exists()
    assert le_entry.encoding == "utf-16-le"
    assert be_entry.encoding == "utf-16-be"


def test_derive_utf16_le_content_is_correct(tmp_path: Path) -> None:
    text = "AB"
    src = tmp_path / "sample_utf8.txt"
    src.write_text(text, encoding="utf-8")
    le_entry, _ = derive_utf16(tmp_path, "sample_utf8.txt")
    le_bytes = (tmp_path / "sample_utf16le.bin").read_bytes()
    assert le_bytes == text.encode("utf-16-le")
    assert le_entry.bytes == len(le_bytes)


def test_derive_utf16_be_content_is_correct(tmp_path: Path) -> None:
    text = "AB"
    src = tmp_path / "sample_utf8.txt"
    src.write_text(text, encoding="utf-8")
    _, be_entry = derive_utf16(tmp_path, "sample_utf8.txt")
    be_bytes = (tmp_path / "sample_utf16be.bin").read_bytes()
    assert be_bytes == text.encode("utf-16-be")
    assert be_entry.bytes == len(be_bytes)


def test_derive_utf16_sha256_matches_file(tmp_path: Path) -> None:
    text = "Test text"
    (tmp_path / "x_utf8.txt").write_text(text, encoding="utf-8")
    le_entry, _ = derive_utf16(tmp_path, "x_utf8.txt")
    expected = sha256_of(text.encode("utf-16-le"))
    assert le_entry.sha256 == expected


# ---------------------------------------------------------------------------
# write_manifest_json
# ---------------------------------------------------------------------------


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


# ---------------------------------------------------------------------------
# write_source_md
# ---------------------------------------------------------------------------


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


def test_write_source_md_contains_gutenberg_attribution(tmp_path: Path) -> None:
    entries = [
        CorpusEntry(
            file="war_and_peace_utf8.txt",
            lang="en",
            encoding="utf-8",
            source_url=GUTENBERG_WAR_AND_PEACE_URL,
            bytes=3359613,
            sha256="e" * 64,
        )
    ]
    write_source_md(tmp_path, entries)
    content = (tmp_path / "SOURCE.md").read_text()
    assert "Gutenberg" in content
    assert "Tolstoy" in content


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
        for f in ["en_mars_utf8.txt", "ru_mars_windows1251.bin", "zh_mars_gb18030.bin"]
    ]
    write_source_md(tmp_path, entries)
    content = (tmp_path / "SOURCE.md").read_text()
    for entry in entries:
        assert entry.file in content


@pytest.mark.parametrize(
    "filename,encoding",
    [
        ("ru_mars_windows1251.bin", "windows-1251"),
        ("ja_mars_shiftjis.bin", "shift_jis"),
        ("zh_mars_gb18030.bin", "gb18030"),
        ("zh_tw_mars_big5.bin", "big5"),
        ("ko_mars_euckr.bin", "euc_kr"),
        ("ja_mars_eucjp.bin", "euc_jp"),
        ("ja_mars_iso2022jp.bin", "iso2022_jp"),
    ],
)
def test_conversions_entry_exists(filename: str, encoding: str) -> None:
    found = any(dest == filename and enc == encoding for _, dest, enc, _ in CONVERSIONS)
    assert found, f"No CONVERSIONS entry for {filename} ({encoding})"
