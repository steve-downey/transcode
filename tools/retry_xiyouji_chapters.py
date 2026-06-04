#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Retry missing Journey to the West chapters with gentle rate limiting.

Reads the existing xiyouji_utf8.txt to find which chapters are present,
fetches missing ones from zh.wikisource.org with 5-second delays, then
rebuilds the full text file and legacy encoding conversions.
"""

import re
import subprocess
import sys
import time
import urllib.parse
from pathlib import Path

WIKISOURCE_BASE = "https://zh.wikisource.org/w/index.php"
OUT_DIR = Path("data/benchmarks")
UTF8_FILE = OUT_DIR / "xiyouji_utf8.txt"
TOTAL_CHAPTERS = 100
DELAY_SECONDS = 5


def fetch_url(url: str) -> bytes:
    result = subprocess.run(
        ["/usr/bin/curl", "-sL", "--fail", "--max-time", "60", url],
        capture_output=True,
        check=True,
    )
    return result.stdout


def strip_wikitext(raw: str) -> str:
    text = raw
    text = re.sub(r"\{\{[^}]*\}\}", "", text)
    text = re.sub(r"\[\[(?:Category|File|分類|分类):[^\]]*\]\]", "", text)
    text = re.sub(r"\[\[[^|\]]*\|([^\]]*)\]\]", r"\1", text)
    text = re.sub(r"\[\[([^\]]*)\]\]", r"\1", text)
    text = re.sub(r"<[^>]+>", "", text)
    text = re.sub(r"^=+\s*.*?\s*=+$", "", text, flags=re.MULTILINE)
    text = "\n".join(line.strip() for line in text.split("\n"))
    text = re.sub(r"\n{3,}", "\n\n", text)
    return text.strip()


def find_present_chapters(content: str) -> set[int]:
    present: set[int] = set()
    for m in re.finditer(r"第(\d{1,3})回", content):
        present.add(int(m.group(1)))
    return present


def main() -> int:
    if not UTF8_FILE.exists():
        print(
            f"Error: {UTF8_FILE} does not exist."
            " Run download_benchmark_corpora.py first."
        )
        return 1

    existing_text = UTF8_FILE.read_text("utf-8")
    present = find_present_chapters(existing_text)
    missing = sorted(set(range(1, TOTAL_CHAPTERS + 1)) - present)

    if not missing:
        print("All 100 chapters are already present.")
        return 0

    print(f"Found {len(present)} chapters present, {len(missing)} missing.")
    print(f"Missing: {missing}")
    print(f"Fetching with {DELAY_SECONDS}s delay between requests...")

    # Store chapters by number for ordered reconstruction
    chapters: dict[int, str] = {}

    # Parse existing chapters
    # Split on the chapter marker pattern, keeping the markers
    parts = re.split(r"(?=第\d{1,3}回)", existing_text)
    for part in parts:
        part = part.strip()
        if not part:
            continue
        m = re.match(r"第(\d{1,3})回", part)
        if m:
            chapters[int(m.group(1))] = part

    fetched = 0
    for chapter_num in missing:
        title = f"西遊記/第{chapter_num:03d}回"
        url = f"{WIKISOURCE_BASE}?title={urllib.parse.quote(title)}&action=raw"
        try:
            raw = fetch_url(url).decode("utf-8")
            text = strip_wikitext(raw)
            if text:
                chapters[chapter_num] = text
                fetched += 1
                print(f"  chapter {chapter_num:03d}: {len(text):,} chars")
            else:
                print(f"  chapter {chapter_num:03d}: empty after stripping")
        except subprocess.CalledProcessError:
            print(f"  chapter {chapter_num:03d}: FAILED (will retry on next run)")

        time.sleep(DELAY_SECONDS)

    if fetched == 0:
        print("No new chapters fetched.")
        return 0

    # Rebuild the full text in chapter order
    full_text = "\n\n".join(chapters[k] for k in sorted(chapters.keys()))
    data = full_text.encode("utf-8")
    UTF8_FILE.write_bytes(data)
    print(f"\nRebuilt: {UTF8_FILE} ({len(data):,} bytes, {len(chapters)} chapters)")

    # Regenerate legacy encodings
    for filename, encoding in [
        ("xiyouji_gb18030.bin", "gb18030"),
        ("xiyouji_big5.bin", "big5"),
    ]:
        encoded = full_text.encode(encoding, errors="replace")
        (OUT_DIR / filename).write_bytes(encoded)
        print(f"Converted: {filename} ({len(encoded):,} bytes)")

    print(f"\nDone. Fetched {fetched} new chapters. Total: {len(chapters)}/100.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
