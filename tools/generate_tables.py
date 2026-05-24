#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Generate C++ and binary table files from WHATWG encoding index data."""

import re
import struct
import subprocess
import sys
from pathlib import Path

from download_indexes import SINGLE_BYTE_INDEXES

DOCS_WHATWG = Path("docs/whatwg")
DATA_TABLES = Path("data/tables")

CLANG_FORMAT = "clang-format"


def codec_to_identifier(name: str) -> str:
    """Convert codec name to a valid C++ identifier (hyphens → underscores)."""
    return re.sub(r"[^a-zA-Z0-9]", "_", name)


def codec_to_guard(name: str) -> str:
    """Convert codec name to an include-guard macro name."""
    ident = codec_to_identifier(name)
    return f"DATA_TABLES_{ident.upper()}_HPP"


def parse_single_byte_index(path: Path) -> list[int]:
    """Parse a WHATWG single-byte index file; return 128-entry codepoint list.

    Missing entries are represented as 0 (unmapped / error indicator).
    """
    table: list[int] = [0] * 128
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) < 2:
            continue
        pointer = int(parts[0].strip())
        cp_str = parts[1].strip()
        if pointer < 0 or pointer >= 128:
            continue
        codepoint = int(cp_str, 16)
        table[pointer] = codepoint
    return table


def write_bin(table: list[int], dest: Path) -> None:
    """Write 128 × uint32 little-endian binary file."""
    dest.parent.mkdir(parents=True, exist_ok=True)
    data = struct.pack("<128I", *table)
    dest.write_bytes(data)


def _format_row(values: list[int], byte_start: int) -> str:
    """Format 8 codepoints as a C++ initializer row with a trailing comment."""
    hex_vals = [f"0x{v:04X}" if v != 0 else "0" for v in values]
    byte_end = byte_start + len(values) - 1
    entries = ", ".join(hex_vals)
    return f"    {entries},  // {byte_start:02X}–{byte_end:02X}"


def render_hpp(name: str, table: list[int]) -> str:
    """Render a C++ header file for the given codec table."""
    guard = codec_to_guard(name)
    ident = codec_to_identifier(name)
    index_file = f"index-{name}.txt"

    rows: list[str] = []
    for i in range(0, 128, 8):
        chunk = table[i : i + 8]
        byte_start = 0x80 + i
        rows.append(_format_row(chunk, byte_start))

    # Fix trailing comma on last row
    rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard {index_file}

#ifndef {guard}
#define {guard}

namespace beman::transcoding::detail::tables {{

inline constexpr char32_t {ident}[128] = {{
{array_body}
}};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def clang_format_file(path: Path) -> None:
    """Run clang-format -i on a file to normalize C++ formatting."""
    subprocess.run(
        [CLANG_FORMAT, "-i", str(path)],
        check=True,
    )


def generate_single_byte(
    name: str,
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate .bin and .hpp for one single-byte codec."""
    index_path = in_dir / f"index-{name}.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping")
        return

    table = parse_single_byte_index(index_path)
    ident = codec_to_identifier(name)

    bin_path = out_dir / f"{ident}.bin"
    write_bin(table, bin_path)

    hpp_path = out_dir / f"{ident}.hpp"
    hpp_content = render_hpp(name, table)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {ident}.bin + {ident}.hpp")


def main(argv: list[str] | None = None) -> int:
    import argparse

    parser = argparse.ArgumentParser(
        description="Generate C++ table files from WHATWG encoding indexes."
    )
    parser.add_argument(
        "--in-dir",
        default=str(DOCS_WHATWG),
        help="Directory containing WHATWG index files (default: docs/whatwg)",
    )
    parser.add_argument(
        "--out-dir",
        default=str(DATA_TABLES),
        help="Output directory for generated tables (default: data/tables)",
    )
    parser.add_argument(
        "--no-clang-format",
        action="store_true",
        help="Skip clang-format pass on generated .hpp files.",
    )
    args = parser.parse_args(argv)

    in_dir = Path(args.in_dir)
    out_dir = Path(args.out_dir)
    run_cf = not args.no_clang_format

    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"Generating single-byte tables from {in_dir}/ → {out_dir}/")
    for name in SINGLE_BYTE_INDEXES:
        generate_single_byte(name, in_dir, out_dir, run_clang_format=run_cf)

    print(f"Done. {len(SINGLE_BYTE_INDEXES)} single-byte codecs processed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
