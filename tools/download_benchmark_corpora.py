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

# Aozora Bunko: 源氏物語 (Tale of Genji), Yosano Akiko modernization (1912-1913)
# Public domain in Japan (author died 1942, translation published 1912-1913).
# 56 chapters, card IDs 5016-5071 under person 000052 (紫式部/与謝野晶子).
AOZORA_GENJI_BASE = "https://www.aozora.gr.jp/cards/000052"
AOZORA_GENJI_CARDS = list(range(5016, 5072))  # 5016..5071 inclusive
GENJI_UTF8_FILE = "genji_monogatari_utf8.txt"

# Wikisource: 西遊記 (Journey to the West), classical Chinese, 100 chapters.
# Public domain (16th century).
WIKISOURCE_XYJI_BASE = "https://zh.wikisource.org/w/index.php"
WIKISOURCE_XYJI_CHAPTERS = 100
XYJI_UTF8_FILE = "xiyouji_utf8.txt"


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


def download_aozora_genji(out_dir: Path, force: bool = False) -> CorpusEntry:
    """Download Tale of Genji from Aozora Bunko; return manifest entry."""
    import io
    import re
    import zipfile

    dest = out_dir / GENJI_UTF8_FILE
    if dest.exists() and not force:
        print(f"  cached: {GENJI_UTF8_FILE}")
        raw = dest.read_bytes()
        return build_manifest_entry(
            GENJI_UTF8_FILE, "ja", "utf-8", AOZORA_GENJI_BASE, raw
        )

    print(f"  downloading Tale of Genji ({len(AOZORA_GENJI_CARDS)} chapters) ...")
    all_text: list[str] = []
    for card_id in AOZORA_GENJI_CARDS:
        card_url = f"{AOZORA_GENJI_BASE}/card{card_id}.html"
        try:
            card_html = fetch_url(card_url).decode("shift_jis", errors="replace")
        except subprocess.CalledProcessError:
            print(f"    warning: could not fetch card {card_id}, skipping")
            continue
        # Extract zip filename from card page (pattern: files/NNNN_ruby_MMMM.zip)
        match = re.search(r"files/(\d+_ruby_\d+\.zip)", card_html)
        if not match:
            print(f"    warning: no zip link on card {card_id}, skipping")
            continue
        zip_name = match.group(1)
        zip_url = f"{AOZORA_GENJI_BASE}/files/{zip_name}"
        try:
            zip_data = fetch_url(zip_url)
        except subprocess.CalledProcessError:
            print(f"    warning: could not fetch {zip_name}, skipping")
            continue
        # Extract text from zip
        with zipfile.ZipFile(io.BytesIO(zip_data)) as zf:
            txt_names = [n for n in zf.namelist() if n.endswith(".txt")]
            if not txt_names:
                continue
            raw_text = zf.read(txt_names[0]).decode("shift_jis", errors="replace")
        # Strip Aozora ruby markup: ｜text《ruby》 → text, and bare 《ruby》
        raw_text = re.sub(r"｜([^《]+)《[^》]+》", r"\1", raw_text)
        raw_text = re.sub(r"《[^》]+》", "", raw_text)
        # Strip header (everything before the first blank line after metadata)
        # and footer (lines starting with 底本：onwards)
        lines = raw_text.split("\n")
        start = 0
        for i, line in enumerate(lines):
            if line.startswith("---") and i > 5:
                start = i + 1
                break
        end = len(lines)
        for i in range(len(lines) - 1, -1, -1):
            if lines[i].startswith("底本："):
                end = i
                break
        chapter_text = "\n".join(lines[start:end]).strip()
        if chapter_text:
            all_text.append(chapter_text)
        print(f"    chapter {card_id - 5015:02d}: {len(chapter_text):,} chars")

    full_text = "\n\n".join(all_text)
    data = full_text.encode("utf-8")
    dest.write_bytes(data)
    print(f"  saved: {GENJI_UTF8_FILE} ({len(data):,} bytes, {len(all_text)} chapters)")
    return build_manifest_entry(GENJI_UTF8_FILE, "ja", "utf-8", AOZORA_GENJI_BASE, data)


def download_wikisource_xiyouji(out_dir: Path, force: bool = False) -> CorpusEntry:
    """Download Journey to the West from zh.wikisource.org; return manifest entry."""
    import re
    import time

    dest = out_dir / XYJI_UTF8_FILE
    source_url = f"{WIKISOURCE_XYJI_BASE}?title=西遊記"
    if dest.exists() and not force:
        print(f"  cached: {XYJI_UTF8_FILE}")
        raw = dest.read_bytes()
        return build_manifest_entry(XYJI_UTF8_FILE, "zh", "utf-8", source_url, raw)

    print(
        f"  downloading Journey to the West ({WIKISOURCE_XYJI_CHAPTERS} chapters) ..."
    )
    all_text: list[str] = []
    for chapter_num in range(1, WIKISOURCE_XYJI_CHAPTERS + 1):
        title = f"西遊記/第{chapter_num:03d}回"
        import urllib.parse

        url = f"{WIKISOURCE_XYJI_BASE}?title={urllib.parse.quote(title)}&action=raw"
        try:
            raw_wikitext = fetch_url(url).decode("utf-8")
        except subprocess.CalledProcessError:
            print(f"    warning: could not fetch chapter {chapter_num:03d}, skipping")
            continue
        # Strip wikitext markup
        text = raw_wikitext
        # Remove {{templates}}
        text = re.sub(r"\{\{[^}]*\}\}", "", text)
        # Remove [[Category:...]] and [[File:...]] links
        text = re.sub(r"\[\[(?:Category|File|分類|分类):[^\]]*\]\]", "", text)
        # Convert [[link|display]] to display, [[link]] to link
        text = re.sub(r"\[\[[^|\]]*\|([^\]]*)\]\]", r"\1", text)
        text = re.sub(r"\[\[([^\]]*)\]\]", r"\1", text)
        # Remove HTML tags
        text = re.sub(r"<[^>]+>", "", text)
        # Remove section headers (== ... ==)
        text = re.sub(r"^=+\s*.*?\s*=+$", "", text, flags=re.MULTILINE)
        # Remove leading/trailing whitespace per line
        text = "\n".join(line.strip() for line in text.split("\n"))
        # Collapse multiple blank lines
        text = re.sub(r"\n{3,}", "\n\n", text)
        text = text.strip()
        if text:
            all_text.append(text)
        print(f"    chapter {chapter_num:03d}: {len(text):,} chars")
        # Be polite to Wikisource
        if chapter_num % 10 == 0:
            time.sleep(1)

    full_text = "\n\n".join(all_text)
    data = full_text.encode("utf-8")
    dest.write_bytes(data)
    print(f"  saved: {XYJI_UTF8_FILE} ({len(data):,} bytes, {len(all_text)} chapters)")
    return build_manifest_entry(XYJI_UTF8_FILE, "zh", "utf-8", source_url, data)


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
        "## Tale of Genji 源氏物語 (Aozora Bunko)",
        "",
        "**Source:** Aozora Bunko (青空文庫), <https://www.aozora.gr.jp/>  ",
        "**Author:** Murasaki Shikibu (紫式部), modernized by"
        " Yosano Akiko (与謝野晶子, 1912-1913)  ",
        "**License:** Public domain in Japan (author died 1942;"
        " translation published 1912-1913, copyright expired)",
        "",
        "## Journey to the West 西遊記 (Wikisource)",
        "",
        "**Source:** Chinese Wikisource, <https://zh.wikisource.org/wiki/西遊記>  ",
        "**Author:** Wu Cheng'en (吳承恩, 16th century)  ",
        "**License:** Public domain (published 16th century)  ",
        "Content available under CC BY-SA 4.0.",
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

    print("\n--- Aozora Bunko: Tale of Genji (源氏物語) ---")
    genji_entry = download_aozora_genji(out_dir, force=args.force)
    entries.append(genji_entry)
    # Legacy encoding conversions
    genji_sjis = derive_conversion(
        out_dir, GENJI_UTF8_FILE, "genji_monogatari_shiftjis.bin", "shift_jis"
    )
    entries.append(
        build_manifest_entry(
            "genji_monogatari_shiftjis.bin",
            "ja",
            "shift_jis",
            AOZORA_GENJI_BASE,
            genji_sjis,
        )
    )
    genji_eucjp = derive_conversion(
        out_dir, GENJI_UTF8_FILE, "genji_monogatari_eucjp.bin", "euc_jp"
    )
    entries.append(
        build_manifest_entry(
            "genji_monogatari_eucjp.bin", "ja", "euc_jp", AOZORA_GENJI_BASE, genji_eucjp
        )
    )

    print("\n--- Wikisource: Journey to the West (西遊記) ---")
    xyji_entry = download_wikisource_xiyouji(out_dir, force=args.force)
    entries.append(xyji_entry)
    # Legacy encoding conversions
    xyji_gb = derive_conversion(
        out_dir, XYJI_UTF8_FILE, "xiyouji_gb18030.bin", "gb18030"
    )
    entries.append(
        build_manifest_entry(
            "xiyouji_gb18030.bin",
            "zh",
            "gb18030",
            f"{WIKISOURCE_XYJI_BASE}?title=西遊記",
            xyji_gb,
        )
    )
    xyji_big5 = derive_conversion(out_dir, XYJI_UTF8_FILE, "xiyouji_big5.bin", "big5")
    entries.append(
        build_manifest_entry(
            "xiyouji_big5.bin",
            "zh",
            "big5",
            f"{WIKISOURCE_XYJI_BASE}?title=西遊記",
            xyji_big5,
        )
    )

    write_manifest_json(out_dir, entries)
    write_source_md(out_dir, entries)

    total = sum(e.bytes for e in entries)
    print(f"\nDone. {len(entries)} files, {total:,} total bytes. Manifest written.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
