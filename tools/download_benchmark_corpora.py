#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Download benchmark corpora for beman.transcode.

Downloads the Mars article from Wikipedia in six languages, the Tolstoy
"War and Peace" novel from Project Gutenberg, converts to the target encodings
where applicable, computes checksums, and writes a manifest.

Outputs (data/benchmarks/):
  en_mars_utf8.txt           ar_mars_utf8.txt
  ru_mars_utf8.txt           ja_mars_utf8.txt
  zh_mars_utf8.txt           ko_mars_utf8.txt
  ru_mars_windows1251.bin    ja_mars_shiftjis.bin
  zh_mars_gb18030.bin        zh_tw_mars_big5.bin
  ko_mars_euckr.bin          ja_mars_eucjp.bin
  ja_mars_iso2022jp.bin
  war_and_peace_utf8.txt
  war_and_peace_utf16le.bin  war_and_peace_utf16be.bin
  corpus_manifest.json       SOURCE.md
"""

import hashlib
import json
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

MEDIAWIKI_API = "https://{lang}.wikipedia.org/w/api.php"
_ARTICLE_BASE = "action=query&prop=extracts&explaintext=true&format=json"

# (lang_code, output_filename, article_title, extra_url_params)
# zh uses variant=zh-tw so Traditional Chinese characters convert to Big5
# ar, ja, ko require native-language titles because "Mars" doesn't redirect.
WIKI_CORPORA: list[tuple[str, str, str, str]] = [
    ("en", "en_mars_utf8.txt", "Mars", ""),
    ("ar", "ar_mars_utf8.txt", "المريخ", ""),
    ("ru", "ru_mars_utf8.txt", "Mars", ""),
    ("ja", "ja_mars_utf8.txt", "火星", ""),
    ("zh", "zh_mars_utf8.txt", "火星", "&variant=zh-tw"),
    ("ko", "ko_mars_utf8.txt", "화성", ""),
]

# Keep a legacy alias so external code importing CORPORA still works
CORPORA: list[tuple[str, str]] = [(lang, fn) for lang, fn, _, _ in WIKI_CORPORA]

# (src_utf8_filename, dest_filename, python_encoding, lang)
CONVERSIONS: list[tuple[str, str, str, str]] = [
    ("ru_mars_utf8.txt", "ru_mars_windows1251.bin", "windows-1251", "ru"),
    ("ja_mars_utf8.txt", "ja_mars_shiftjis.bin", "shift_jis", "ja"),
    ("zh_mars_utf8.txt", "zh_mars_gb18030.bin", "gb18030", "zh"),
    ("zh_mars_utf8.txt", "zh_tw_mars_big5.bin", "big5", "zh"),
    ("ko_mars_utf8.txt", "ko_mars_euckr.bin", "euc_kr", "ko"),
    ("ja_mars_utf8.txt", "ja_mars_eucjp.bin", "euc_jp", "ja"),
    ("ja_mars_utf8.txt", "ja_mars_iso2022jp.bin", "iso2022_jp", "ja"),
]

GUTENBERG_WAR_AND_PEACE_URL = "https://www.gutenberg.org/cache/epub/2600/pg2600.txt"
GUTENBERG_WAR_AND_PEACE_FILE = "war_and_peace_utf8.txt"


def sha256_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def fetch_url(url: str) -> bytes:
    result = subprocess.run(
        ["/usr/bin/curl", "-sL", "--fail", "--max-time", "60", url],
        capture_output=True,
        check=True,
    )
    return result.stdout


def extract_article_text(api_response: bytes) -> str:
    """Extract plain text from a MediaWiki API JSON response."""
    data: dict[str, object] = json.loads(api_response.decode("utf-8"))
    query = data["query"]
    assert isinstance(query, dict)
    pages = query["pages"]
    assert isinstance(pages, dict)
    page = next(iter(pages.values()))
    assert isinstance(page, dict)
    extract = page.get("extract", "")
    assert isinstance(extract, str)
    return extract


def convert_encoding(text: str, encoding: str) -> bytes:
    return text.encode(encoding, errors="replace")


@dataclass
class CorpusEntry:
    file: str
    lang: str
    encoding: str
    source_url: str
    bytes: int
    sha256: str


def build_manifest_entry(
    filename: str,
    lang: str,
    encoding: str,
    source_url: str,
    data: bytes,
) -> CorpusEntry:
    return CorpusEntry(
        file=filename,
        lang=lang,
        encoding=encoding,
        source_url=source_url,
        bytes=len(data),
        sha256=sha256_of(data),
    )


def write_manifest_json(out_dir: Path, entries: list[CorpusEntry]) -> None:
    manifest = [
        {
            "file": e.file,
            "lang": e.lang,
            "encoding": e.encoding,
            "source_url": e.source_url,
            "bytes": e.bytes,
            "sha256": e.sha256,
        }
        for e in entries
    ]
    (out_dir / "corpus_manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n"
    )


def download_wiki_corpus(
    out_dir: Path,
    lang: str,
    filename: str,
    title: str,
    extra_params: str,
) -> tuple[bytes, str]:
    """Download one Wikipedia article. Returns (utf8_data, source_url)."""
    import urllib.parse

    encoded_title = urllib.parse.quote(title)
    params = f"?{_ARTICLE_BASE}&titles={encoded_title}{extra_params}"
    url = MEDIAWIKI_API.format(lang=lang) + params
    dest = out_dir / filename
    if dest.exists():
        print(f"  cached: {filename}")
        return dest.read_bytes(), url
    api_bytes = fetch_url(url)
    text = extract_article_text(api_bytes)
    data = text.encode("utf-8")
    dest.write_bytes(data)
    print(f"  downloaded: {filename} ({len(data):,} bytes)")
    return data, url


# Keep the old signature for backward compat with tests that call download_corpus
def download_corpus(out_dir: Path, lang: str, filename: str) -> tuple[bytes, str]:
    """Download Wikipedia Mars article (English title). Returns (data, url)."""
    return download_wiki_corpus(out_dir, lang, filename, "Mars", "")


def download_gutenberg_corpus(out_dir: Path, force: bool = False) -> CorpusEntry:
    """Download War and Peace from Project Gutenberg; return manifest entry."""
    dest = out_dir / GUTENBERG_WAR_AND_PEACE_FILE
    if dest.exists() and not force:
        print(f"  cached: {GUTENBERG_WAR_AND_PEACE_FILE}")
        raw = dest.read_bytes()
    else:
        print(f"  downloading: {GUTENBERG_WAR_AND_PEACE_FILE} ...")
        raw = fetch_url(GUTENBERG_WAR_AND_PEACE_URL)
        # Strip UTF-8 BOM if present
        if raw.startswith(b"\xef\xbb\xbf"):
            raw = raw[3:]
        # Normalise CRLF → LF
        raw = raw.replace(b"\r\n", b"\n")
        dest.write_bytes(raw)
        print(f"  saved: {GUTENBERG_WAR_AND_PEACE_FILE} ({len(raw):,} bytes)")
    return build_manifest_entry(
        GUTENBERG_WAR_AND_PEACE_FILE,
        "en",
        "utf-8",
        GUTENBERG_WAR_AND_PEACE_URL,
        raw,
    )


def derive_conversion(
    out_dir: Path,
    src_filename: str,
    dest_filename: str,
    encoding: str,
) -> bytes:
    """Convert a UTF-8 file to an alternate encoding."""
    src = out_dir / src_filename
    dest = out_dir / dest_filename
    text = src.read_bytes().decode("utf-8")
    data = convert_encoding(text, encoding)
    dest.write_bytes(data)
    print(f"  converted: {dest_filename} ({len(data):,} bytes, {encoding})")
    return data


def derive_utf16(out_dir: Path, src_filename: str) -> tuple[CorpusEntry, CorpusEntry]:
    """Write UTF-16LE and UTF-16BE variants of a UTF-8 file."""
    src = out_dir / src_filename
    text = src.read_bytes().decode("utf-8")
    base = src_filename.replace("_utf8.txt", "").replace("_utf8", "")
    source_url = GUTENBERG_WAR_AND_PEACE_URL

    le_filename = f"{base}_utf16le.bin"
    be_filename = f"{base}_utf16be.bin"

    le_data = text.encode("utf-16-le")
    be_data = text.encode("utf-16-be")
    (out_dir / le_filename).write_bytes(le_data)
    (out_dir / be_filename).write_bytes(be_data)
    print(f"  converted: {le_filename} ({len(le_data):,} bytes, utf-16-le)")
    print(f"  converted: {be_filename} ({len(be_data):,} bytes, utf-16-be)")

    le_entry = build_manifest_entry(le_filename, "en", "utf-16-le", source_url, le_data)
    be_entry = build_manifest_entry(be_filename, "en", "utf-16-be", source_url, be_data)
    return le_entry, be_entry


def write_source_md(out_dir: Path, entries: list[CorpusEntry]) -> None:
    from datetime import date

    retrieved = date.today().isoformat()
    lines = [
        "# Benchmark Corpora",
        "",
        "Generated corpus data. **Do not edit.**"
        " To refresh: `uv run tools/download_benchmark_corpora.py`",
        "",
        f"**Retrieved:** {retrieved}",
        "",
        "## Wikipedia Mars Article",
        "",
        "**Source:** Wikipedia, <https://www.wikipedia.org/>  ",
        "Content available under the Creative Commons"
        " Attribution-ShareAlike License 4.0 (CC BY-SA 4.0).",
        "",
        "**License:** CC BY-SA 4.0  ",
        "See <https://creativecommons.org/licenses/by-sa/4.0/>",
        "",
        "**Attribution:**",
        "- English: Wikipedia contributors, 'Mars', Wikipedia, The Free Encyclopedia",
        "- Arabic: Wikipedia contributors, 'Mars (المريخ)',"
        " Wikipedia (ar.wikipedia.org)",
        "- Russian: Wikipedia contributors, 'Mars (Марс)',"
        " Wikipedia (ru.wikipedia.org)",
        "- Japanese: Wikipedia contributors, 'Mars (火星)',"
        " Wikipedia (ja.wikipedia.org)",
        "- Chinese: Wikipedia contributors, '火星', Wikipedia (zh.wikipedia.org)",
        "- Korean: Wikipedia contributors, '화성', Wikipedia (ko.wikipedia.org)",
        "",
        "## War and Peace (Project Gutenberg)",
        "",
        "**Source:** Project Gutenberg, <https://www.gutenberg.org/ebooks/2600>  ",
        "**Author:** Leo Tolstoy (translated by Louise and Aylmer Maude)",
        "**License:** Public domain in the USA (Project Gutenberg License)",
        "See <https://www.gutenberg.org/policy/license.html>",
        "",
        "## Corpus Files",
        "",
        "| File | Language | Encoding | Bytes | SHA-256 (prefix) |",
        "| ---- | -------- | -------- | ----- | ---------------- |",
    ]
    for e in entries:
        lines.append(
            f"| `{e.file}` | {e.lang} | {e.encoding}"
            f" | {e.bytes:,} | `{e.sha256[:16]}…` |"
        )
    lines += [
        "",
        "## Full Checksums",
        "",
    ]
    for e in entries:
        lines.append(f"- `{e.file}`: `{e.sha256}`")
    lines.append("")
    (out_dir / "SOURCE.md").write_text("\n".join(lines))


def main(argv: list[str] | None = None) -> int:
    import argparse

    parser = argparse.ArgumentParser(
        description="Download benchmark corpora for beman.transcode."
    )
    parser.add_argument(
        "--out-dir",
        default="data/benchmarks",
        help="Output directory (default: data/benchmarks)",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-download even if files are already present.",
    )
    args = parser.parse_args(argv)

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    print(f"Downloading benchmark corpora to {out_dir}/")

    entries: list[CorpusEntry] = []

    print("\n--- Wikipedia Mars articles ---")
    for lang, filename, title, extra in WIKI_CORPORA:
        if args.force and (out_dir / filename).exists():
            (out_dir / filename).unlink()
        data, source_url = download_wiki_corpus(out_dir, lang, filename, title, extra)
        entries.append(build_manifest_entry(filename, lang, "utf-8", source_url, data))

    print("\n--- Encoding conversions ---")
    for src_filename, dest_filename, encoding, lang in CONVERSIONS:
        if args.force and (out_dir / dest_filename).exists():
            (out_dir / dest_filename).unlink()
        data = derive_conversion(out_dir, src_filename, dest_filename, encoding)
        source_url = MEDIAWIKI_API.format(lang=lang) + f"?{_ARTICLE_BASE}&titles=Mars"
        entries.append(
            build_manifest_entry(dest_filename, lang, encoding, source_url, data)
        )

    print("\n--- Project Gutenberg: War and Peace ---")
    wp_entry = download_gutenberg_corpus(out_dir, force=args.force)
    entries.append(wp_entry)
    le_entry, be_entry = derive_utf16(out_dir, GUTENBERG_WAR_AND_PEACE_FILE)
    entries.append(le_entry)
    entries.append(be_entry)

    write_manifest_json(out_dir, entries)
    write_source_md(out_dir, entries)

    total = sum(e.bytes for e in entries)
    print(f"\nDone. {len(entries)} files, {total:,} total bytes. Manifest written.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
