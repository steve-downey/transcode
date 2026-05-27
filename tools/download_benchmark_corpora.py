#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Download Wikipedia Mars article corpora for benchmarking.

Downloads the Mars article from Wikipedia in four languages, converts to the
target encoding where applicable, computes checksums, and writes a manifest.

Outputs:
  data/benchmarks/en_mars_utf8.txt
  data/benchmarks/ar_mars_utf8.txt
  data/benchmarks/ru_mars_utf8.txt
  data/benchmarks/ru_mars_windows1251.bin
  data/benchmarks/ja_mars_utf8.txt
  data/benchmarks/ja_mars_shiftjis.bin
  data/benchmarks/corpus_manifest.json
"""

import hashlib
import json
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

MEDIAWIKI_API = "https://{lang}.wikipedia.org/w/api.php"
ARTICLE_PARAMS = "?action=query&titles=Mars&prop=extracts&explaintext=true&format=json"

CORPORA: list[tuple[str, str]] = [
    ("en", "en_mars_utf8.txt"),
    ("ar", "ar_mars_utf8.txt"),
    ("ru", "ru_mars_utf8.txt"),
    ("ja", "ja_mars_utf8.txt"),
]

CONVERSIONS: list[tuple[str, str, str]] = [
    ("ru_mars_utf8.txt", "ru_mars_windows1251.bin", "windows-1251"),
    ("ja_mars_utf8.txt", "ja_mars_shiftjis.bin", "shift_jis"),
]


def sha256_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def fetch_url(url: str) -> bytes:
    result = subprocess.run(
        ["/usr/bin/curl", "-sL", "--fail", "--max-time", "30", url],
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


def download_corpus(out_dir: Path, lang: str, filename: str) -> tuple[bytes, str]:
    """Download one Wikipedia Mars article. Returns (data, source_url)."""
    url = MEDIAWIKI_API.format(lang=lang) + ARTICLE_PARAMS
    dest = out_dir / filename
    if dest.exists():
        print(f"  cached: {filename}")
        return dest.read_bytes(), url
    api_bytes = fetch_url(url)
    text = extract_article_text(api_bytes)
    data = text.encode("utf-8")
    dest.write_bytes(data)
    print(f"  downloaded: {filename} ({len(data)} bytes)")
    return data, url


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
    print(f"  converted: {dest_filename} ({len(data)} bytes, {encoding})")
    return data


def write_source_md(out_dir: Path, entries: list[CorpusEntry]) -> None:
    from datetime import date

    retrieved = date.today().isoformat()
    lines = [
        "# Benchmark Corpora — Wikipedia Mars Article",
        "",
        "Generated corpus data. **Do not edit.**"
        " To refresh: `uv run tools/download_benchmark_corpora.py`",
        "",
        f"**Retrieved:** {retrieved}",
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
        "",
        "## Corpus Files",
        "",
        "| File | Language | Encoding | Bytes | SHA-256 (prefix) |",
        "| ---- | -------- | -------- | ----- | ---------------- |",
    ]
    for e in entries:
        lines.append(
            f"| `{e.file}` | {e.lang} | {e.encoding} | {e.bytes} | `{e.sha256[:16]}…` |"
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
        description="Download Wikipedia Mars article corpora for benchmarking."
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
    utf8_data: dict[str, bytes] = {}

    if args.force:
        for _, filename in CORPORA:
            dest = out_dir / filename
            if dest.exists():
                dest.unlink()

    for lang, filename in CORPORA:
        data, source_url = download_corpus(out_dir, lang, filename)
        utf8_data[filename] = data
        entries.append(build_manifest_entry(filename, lang, "utf-8", source_url, data))

    for src_filename, dest_filename, encoding in CONVERSIONS:
        lang = src_filename[:2]
        data = derive_conversion(out_dir, src_filename, dest_filename, encoding)
        source_url = MEDIAWIKI_API.format(lang=lang) + ARTICLE_PARAMS
        entries.append(
            build_manifest_entry(dest_filename, lang, encoding, source_url, data)
        )

    write_manifest_json(out_dir, entries)
    write_source_md(out_dir, entries)

    total = sum(e.bytes for e in entries)
    print(f"Done. {len(entries)} files, {total} total bytes. Manifest written.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
