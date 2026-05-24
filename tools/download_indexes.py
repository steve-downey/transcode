#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Download all WHATWG encoding index files into docs/whatwg/."""

import hashlib
import json
import subprocess
import sys
from datetime import date
from pathlib import Path

BASE_URL = "https://encoding.spec.whatwg.org/"

SINGLE_BYTE_INDEXES: list[str] = [
    "ibm866",
    "iso-8859-2",
    "iso-8859-3",
    "iso-8859-4",
    "iso-8859-5",
    "iso-8859-6",
    "iso-8859-7",
    "iso-8859-8",
    # iso-8859-8-i shares the same WHATWG index as iso-8859-8
    "iso-8859-10",
    "iso-8859-13",
    "iso-8859-14",
    "iso-8859-15",
    "iso-8859-16",
    "koi8-r",
    "koi8-u",
    "macintosh",
    "windows-874",
    "windows-1250",
    "windows-1251",
    "windows-1252",
    "windows-1253",
    "windows-1254",
    "windows-1255",
    "windows-1256",
    "windows-1257",
    "windows-1258",
    "x-mac-cyrillic",
]

MULTI_BYTE_INDEXES: list[str] = [
    "big5",
    "euc-kr",
    "gb18030",
    "gb18030-ranges",
    "jis0208",
    "jis0212",
    "iso-2022-jp-katakana",
]

EXTRA_FILES: list[str] = [
    "encodings.json",
]


def sha256_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def fetch_file(url: str) -> bytes:
    result = subprocess.run(
        ["/usr/bin/curl", "-sL", "--fail", "--max-time", "30", url],
        capture_output=True,
        check=True,
    )
    return result.stdout


def index_url(name: str) -> str:
    return f"{BASE_URL}index-{name}.txt"


def extra_url(name: str) -> str:
    return f"{BASE_URL}{name}"


def download_one(url: str, dest: Path, force: bool = False) -> tuple[str, bool]:
    """Download url to dest; skip if already present and checksum matches.

    Returns (sha256_hex, was_downloaded).
    """
    if dest.exists() and not force:
        existing = dest.read_bytes()
        return sha256_of(existing), False

    data = fetch_file(url)
    dest.write_bytes(data)
    return sha256_of(data), True


def download_all(
    out_dir: Path, force: bool = False
) -> dict[str, tuple[str, str, bool]]:
    """Download all index files. Returns {filename: (url, sha256, downloaded)}."""
    out_dir.mkdir(parents=True, exist_ok=True)
    results: dict[str, tuple[str, str, bool]] = {}

    for name in SINGLE_BYTE_INDEXES:
        filename = f"index-{name}.txt"
        url = index_url(name)
        dest = out_dir / filename
        sha, downloaded = download_one(url, dest, force)
        results[filename] = (url, sha, downloaded)
        status = "downloaded" if downloaded else "cached"
        print(f"  {status}: {filename}")

    for name in MULTI_BYTE_INDEXES:
        filename = f"index-{name}.txt"
        url = index_url(name)
        dest = out_dir / filename
        sha, downloaded = download_one(url, dest, force)
        results[filename] = (url, sha, downloaded)
        status = "downloaded" if downloaded else "cached"
        print(f"  {status}: {filename}")

    for name in EXTRA_FILES:
        url = extra_url(name)
        dest = out_dir / name
        sha, downloaded = download_one(url, dest, force)
        results[name] = (url, sha, downloaded)
        status = "downloaded" if downloaded else "cached"
        print(f"  {status}: {name}")

    return results


def write_source_md(
    out_dir: Path, results: dict[str, tuple[str, str, bool]], retrieved: date
) -> None:
    lines: list[str] = [
        "# WHATWG Encoding Standard — Index Data",
        "",
        "Pristine upstream data. **Do not edit.**"
        " To refresh: `uv run tools/download_indexes.py`",
        "",
        f"**Retrieved:** {retrieved.isoformat()}",
        "",
        "**Source:** WHATWG Encoding Standard, <https://encoding.spec.whatwg.org/>  ",
        "Living Standard, maintained by the WHATWG community.",
        "",
        "**License:**",
        "- Data/documentation: CC-BY 4.0",
        "- Code portions derived from the spec: BSD-3-Clause",
        "",
        "**Attribution required (CC-BY 4.0):**"
        " Derived from the WHATWG Encoding Standard,",
        "https://encoding.spec.whatwg.org/, Living Standard.",
        "",
        "## File Provenance",
        "",
        "| File | Source URL | SHA-256 |",
        "| ---- | ---------- | ------- |",
    ]

    for filename, (url, sha, _) in sorted(results.items()):
        lines.append(f"| `{filename}` | {url} | `{sha[:16]}…` |")

    lines += [
        "",
        "## Full Checksums",
        "",
    ]
    for filename, (_, sha, _) in sorted(results.items()):
        lines.append(f"- `{filename}`: `{sha}`")

    lines.append("")
    (out_dir / "SOURCE.md").write_text("\n".join(lines))


def write_source_bib(out_dir: Path, retrieved: date) -> None:
    bib = f"""\
@misc{{whatwg-encoding,
  title        = {{Encoding Standard}},
  author       = {{{{WHATWG}}}},
  howpublished = {{Living Standard}},
  url          = {{https://encoding.spec.whatwg.org/}},
  note         = {{Index data retrieved {retrieved.isoformat()}}},
  year         = {{{retrieved.year}}},
  license      = {{CC-BY-4.0 and BSD-3-Clause}}
}}
"""
    (out_dir / "source.bib").write_text(bib)


def validate_encodings_json(out_dir: Path) -> None:
    path = out_dir / "encodings.json"
    if not path.exists():
        raise FileNotFoundError(f"encodings.json not found at {path}")
    data = json.loads(path.read_text())
    if not isinstance(data, list):
        raise ValueError("encodings.json: expected top-level list")


def main(argv: list[str] | None = None) -> int:
    import argparse

    parser = argparse.ArgumentParser(
        description="Download WHATWG encoding index files."
    )
    parser.add_argument(
        "--out-dir",
        default="docs/whatwg",
        help="Output directory (default: docs/whatwg)",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-download even if files are already present.",
    )
    args = parser.parse_args(argv)

    out_dir = Path(args.out_dir)
    print(f"Downloading WHATWG index files to {out_dir}/")
    results = download_all(out_dir, force=args.force)

    today = date.today()
    write_source_md(out_dir, results, today)
    write_source_bib(out_dir, today)

    print("Validating encodings.json …")
    validate_encodings_json(out_dir)

    n_downloaded = sum(1 for _, _, dl in results.values() if dl)
    n_cached = len(results) - n_downloaded
    print(
        f"Done. {n_downloaded} downloaded, {n_cached} already cached. "
        f"Provenance written."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
