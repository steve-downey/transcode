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
INCLUDE_TABLES = Path("include/beman/transcode/detail/tables")

GBK_LEAD_MIN = 0x81
GBK_LEAD_MAX = 0xFE
GBK_TRAIL_MIN = 0x40
GBK_TRAIL_MAX = 0xFE
GBK_TRAIL_EXCLUDE = 0x7F
GBK_POINTER_COUNT = (GBK_LEAD_MAX - GBK_LEAD_MIN + 1) * 190  # 126 * 190 = 23940

BIG5_LEAD_MIN = 0x81
BIG5_LEAD_MAX = 0xFE
BIG5_TRAIL_RANGE1_MIN = 0x40
BIG5_TRAIL_RANGE1_MAX = 0x7E  # 63 values: offset 0-62
BIG5_TRAIL_RANGE2_MIN = 0xA1
BIG5_TRAIL_RANGE2_MAX = 0xFE  # 94 values: offset 63-156
BIG5_TRAIL_COUNT = 63 + 94  # 157
BIG5_POINTER_COUNT = (
    BIG5_LEAD_MAX - BIG5_LEAD_MIN + 1
) * BIG5_TRAIL_COUNT  # 126 * 157 = 19782

SHIFTJIS_TRAIL_COUNT = 63 + 125  # 188: 0x40-0x7E + 0x80-0xFC
SHIFTJIS_LEAD_COUNT = 31 + 29  # 60: 0x81-0x9F + 0xE0-0xFC
SHIFTJIS_POINTER_COUNT = SHIFTJIS_LEAD_COUNT * SHIFTJIS_TRAIL_COUNT  # 60 * 188 = 11280

# EUC-JP JIS X 0212: lead 0xA1-0xFE (94), trail 0xA1-0xFE (94)
# pointer = (b1 - 0xA1) * 94 + (b2 - 0xA1)
EUC_JP_JIS0212_POINTER_COUNT = 94 * 94  # 8836

# EUC-KR (UHC/CP949): lead 0x81-0xFE (126), trail 0x41-0xFE (190, includes 0x7F slot)
# pointer = (lead - 0x81) * 190 + (trail - 0x41) - (1 if trail > 0x7F else 0)
EUC_KR_POINTER_COUNT = 126 * 190  # 23940

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


def parse_gbk_index(path: Path) -> list[int]:
    """Parse WHATWG index-gb18030.txt; return 23940-entry GBK decode table.

    Each entry is the Unicode codepoint at that GBK pointer position, or 0 if
    unmapped. Only pointers 0–23939 are relevant for GBK; the file may contain
    additional entries for GB18030 ranges which are ignored here.
    """
    table: list[int] = [0] * GBK_POINTER_COUNT
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) < 2:
            continue
        pointer = int(parts[0].strip())
        cp_str = parts[1].strip()
        if pointer < 0 or pointer >= GBK_POINTER_COUNT:
            continue
        codepoint = int(cp_str, 16)
        table[pointer] = codepoint
    return table


def _format_gbk_row(values: list[int], start_idx: int) -> str:
    """Format up to 8 GBK table entries as a C++ initializer row."""
    hex_vals = [f"0x{v:04X}" if v != 0 else "0" for v in values]
    end_idx = start_idx + len(values) - 1
    entries = ", ".join(hex_vals)
    return f"    {entries},  // [{start_idx}–{end_idx}]"


def render_gbk_hpp(table: list[int]) -> str:
    """Render the GBK decode table as a C++ header."""
    guard = "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GBK_HPP"
    n = len(table)
    rows: list[str] = []
    for i in range(0, n, 8):
        chunk = table[i : i + 8]
        rows.append(_format_gbk_row(chunk, i))
    rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-gb18030.txt

#ifndef {guard}
#define {guard}

namespace beman::transcoding::detail::tables {{

inline constexpr char32_t gbk[{n}] = {{
{array_body}
}};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def generate_gbk(
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate GBK decode table HPP into the include tree."""
    index_path = in_dir / "index-gb18030.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping GBK")
        return

    table = parse_gbk_index(index_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    hpp_path = out_dir / "gbk.hpp"
    hpp_content = render_gbk_hpp(table)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {hpp_path} ({len(table)} entries)")


def parse_gb18030_ranges(path: Path) -> list[tuple[int, int]]:
    """Parse WHATWG index-gb18030-ranges.txt; return (pointer, codepoint) tuples.

    Each entry is a pair: pointer (decimal) and codepoint (hex with 0x prefix).
    Returns entries sorted by pointer.
    """
    entries: list[tuple[int, int]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        pointer = int(parts[0])
        codepoint = int(parts[1], 16)
        entries.append((pointer, codepoint))
    return sorted(entries, key=lambda e: e[0])


def render_gb18030_ranges_hpp(ranges: list[tuple[int, int]]) -> str:
    """Render the GB18030 ranges table as a C++ header."""
    guard = "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GB18030_RANGES_HPP"
    n = len(ranges)

    rows: list[str] = []
    for pointer, codepoint in ranges:
        rows.append(f"    {{{pointer}u, 0x{codepoint:04X}u}},")
    if rows:
        rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-gb18030-ranges.txt

#ifndef {guard}
#define {guard}

#include <cstdint>

namespace beman::transcoding::detail::tables {{

struct gb18030_range {{
    std::uint32_t pointer;
    char32_t      codepoint;
}};

inline constexpr gb18030_range gb18030_ranges[{n}] = {{
{array_body}
}};

inline constexpr int gb18030_ranges_count = {n};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def generate_gb18030_ranges(
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate GB18030 ranges table HPP into the include tree."""
    index_path = in_dir / "index-gb18030-ranges.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping GB18030 ranges")
        return

    ranges = parse_gb18030_ranges(index_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    hpp_path = out_dir / "gb18030_ranges.hpp"
    hpp_content = render_gb18030_ranges_hpp(ranges)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {hpp_path} ({len(ranges)} entries)")


def parse_big5_index(path: Path) -> list[int]:
    """Parse WHATWG index-big5.txt; return 19782-entry Big5 decode table.

    Each entry is the Unicode codepoint at that Big5 pointer position, or 0 if
    unmapped. Lead bytes are 0x81–0xFE (126 values). Trail bytes are 0x40–0x7E
    (63 values, offset 0–62) and 0xA1–0xFE (94 values, offset 63–156), giving
    157 trail values. Total pointers: 126 × 157 = 19782.
    """
    table: list[int] = [0] * BIG5_POINTER_COUNT
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        pointer = int(parts[0])
        cp_str = parts[1]
        if pointer < 0 or pointer >= BIG5_POINTER_COUNT:
            continue
        codepoint = int(cp_str, 16)
        table[pointer] = codepoint
    return table


def _format_big5_row(values: list[int], start_idx: int) -> str:
    """Format up to 8 Big5 table entries as a C++ initializer row."""
    hex_vals = [f"0x{v:04X}" if v != 0 else "0" for v in values]
    end_idx = start_idx + len(values) - 1
    entries = ", ".join(hex_vals)
    return f"    {entries},  // [{start_idx}–{end_idx}]"


def render_big5_hpp(table: list[int]) -> str:
    """Render the Big5 decode table as a C++ header."""
    guard = "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_BIG5_HPP"
    n = len(table)
    rows: list[str] = []
    for i in range(0, n, 8):
        chunk = table[i : i + 8]
        rows.append(_format_big5_row(chunk, i))
    rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-big5.txt

#ifndef {guard}
#define {guard}

namespace beman::transcoding::detail::tables {{

inline constexpr char32_t big5[{n}] = {{
{array_body}
}};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def generate_big5(
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate Big5 decode table HPP into the include tree."""
    index_path = in_dir / "index-big5.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping Big5")
        return

    table = parse_big5_index(index_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    hpp_path = out_dir / "big5.hpp"
    hpp_content = render_big5_hpp(table)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {hpp_path} ({len(table)} entries)")


def parse_shift_jis_index(path: Path) -> list[int]:
    """Parse WHATWG index-jis0208.txt; return 11280-entry Shift_JIS decode table.

    Each entry is the Unicode codepoint at that Shift_JIS pointer position, or 0 if
    unmapped. Lead bytes are 0x81–0x9F (31 values, lead_offset 0–30) and 0xE0–0xFC
    (29 values, lead_offset 31–59). Trail bytes are 0x40–0x7E (63 values,
    trail_offset 0–62) and 0x80–0xFC (125 values, trail_offset 63–187), giving 188
    trail values. Total pointers: 60 × 188 = 11280.
    """
    table: list[int] = [0] * SHIFTJIS_POINTER_COUNT
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        pointer = int(parts[0])
        cp_str = parts[1]
        if pointer < 0 or pointer >= SHIFTJIS_POINTER_COUNT:
            continue
        codepoint = int(cp_str, 16)
        table[pointer] = codepoint
    return table


def _format_shift_jis_row(values: list[int], start_idx: int) -> str:
    """Format up to 8 Shift_JIS table entries as a C++ initializer row."""
    hex_vals = [f"0x{v:04X}" if v != 0 else "0" for v in values]
    end_idx = start_idx + len(values) - 1
    entries = ", ".join(hex_vals)
    return f"    {entries},  // [{start_idx}–{end_idx}]"


def render_shift_jis_hpp(table: list[int]) -> str:
    """Render the Shift_JIS decode table as a C++ header."""
    guard = "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_SHIFT_JIS_HPP"
    n = len(table)
    rows: list[str] = []
    for i in range(0, n, 8):
        chunk = table[i : i + 8]
        rows.append(_format_shift_jis_row(chunk, i))
    rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-jis0208.txt

#ifndef {guard}
#define {guard}

namespace beman::transcoding::detail::tables {{

inline constexpr char32_t shift_jis[{n}] = {{
{array_body}
}};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def generate_shift_jis(
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate Shift_JIS decode table HPP into the include tree."""
    index_path = in_dir / "index-jis0208.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping Shift_JIS")
        return

    table = parse_shift_jis_index(index_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    hpp_path = out_dir / "shift_jis.hpp"
    hpp_content = render_shift_jis_hpp(table)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {hpp_path} ({len(table)} entries)")


def parse_euc_jp_jis0212_index(path: Path) -> list[int]:
    """Parse WHATWG index-jis0212.txt; return 8836-entry JIS X 0212 decode table.

    Pointer formula: pointer = (b1 - 0xA1) * 94 + (b2 - 0xA1), where b1 and b2
    are both in 0xA1–0xFE (94 values each). Total pointers: 94 × 94 = 8836.
    """
    table: list[int] = [0] * EUC_JP_JIS0212_POINTER_COUNT
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        pointer = int(parts[0])
        cp_str = parts[1]
        if pointer < 0 or pointer >= EUC_JP_JIS0212_POINTER_COUNT:
            continue
        codepoint = int(cp_str, 16)
        table[pointer] = codepoint
    return table


def _format_euc_jp_jis0212_row(values: list[int], start_idx: int) -> str:
    """Format up to 8 JIS X 0212 table entries as a C++ initializer row."""
    hex_vals = [f"0x{v:04X}" if v != 0 else "0" for v in values]
    end_idx = start_idx + len(values) - 1
    entries = ", ".join(hex_vals)
    return f"    {entries},  // [{start_idx}–{end_idx}]"


def render_euc_jp_jis0212_hpp(table: list[int]) -> str:
    """Render the EUC-JP JIS X 0212 decode table as a C++ header."""
    guard = "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_JP_JIS0212_HPP"
    n = len(table)
    rows: list[str] = []
    for i in range(0, n, 8):
        chunk = table[i : i + 8]
        rows.append(_format_euc_jp_jis0212_row(chunk, i))
    rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-jis0212.txt

#ifndef {guard}
#define {guard}

namespace beman::transcoding::detail::tables {{

inline constexpr char32_t euc_jp_jis0212[{n}] = {{
{array_body}
}};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def generate_euc_jp_jis0212(
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate EUC-JP JIS X 0212 decode table HPP into the include tree."""
    index_path = in_dir / "index-jis0212.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping EUC-JP JIS X 0212")
        return

    table = parse_euc_jp_jis0212_index(index_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    hpp_path = out_dir / "euc_jp_jis0212.hpp"
    hpp_content = render_euc_jp_jis0212_hpp(table)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {hpp_path} ({len(table)} entries)")


def parse_euc_kr_index(path: Path) -> list[int]:
    """Parse WHATWG index-euc-kr.txt; return 23940-entry EUC-KR decode table.

    Pointer formula: pointer = (lead - 0x81) * 190 + (trail - 0x41) - offset,
    where offset=1 if trail > 0x7F else 0. Lead in 0x81-0xFE (126 values),
    trail in 0x41-0xFE (190 slots including 0x7F). Total pointers: 126 x 190 = 23940.
    """
    table: list[int] = [0] * EUC_KR_POINTER_COUNT
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        pointer = int(parts[0])
        cp_str = parts[1]
        if pointer < 0 or pointer >= EUC_KR_POINTER_COUNT:
            continue
        codepoint = int(cp_str, 16)
        table[pointer] = codepoint
    return table


def _format_euc_kr_row(values: list[int], start_idx: int) -> str:
    """Format up to 8 EUC-KR table entries as a C++ initializer row."""
    hex_vals = [f"0x{v:04X}" if v != 0 else "0" for v in values]
    end_idx = start_idx + len(values) - 1
    entries = ", ".join(hex_vals)
    return f"    {entries},  // [{start_idx}–{end_idx}]"


def render_euc_kr_hpp(table: list[int]) -> str:
    """Render the EUC-KR decode table as a C++ header."""
    guard = "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_KR_HPP"
    n = len(table)
    rows: list[str] = []
    for i in range(0, n, 8):
        chunk = table[i : i + 8]
        rows.append(_format_euc_kr_row(chunk, i))
    rows[-1] = rows[-1].rstrip(",")

    array_body = "\n".join(rows)

    return f"""\
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-euc-kr.txt

#ifndef {guard}
#define {guard}

namespace beman::transcoding::detail::tables {{

inline constexpr char32_t euc_kr[{n}] = {{
{array_body}
}};

}} // namespace beman::transcoding::detail::tables

#endif // {guard}
"""


def generate_euc_kr(
    in_dir: Path,
    out_dir: Path,
    run_clang_format: bool = True,
) -> None:
    """Generate EUC-KR decode table HPP into the include tree."""
    index_path = in_dir / "index-euc-kr.txt"
    if not index_path.exists():
        print(f"  WARNING: {index_path} not found, skipping EUC-KR")
        return

    table = parse_euc_kr_index(index_path)
    out_dir.mkdir(parents=True, exist_ok=True)

    hpp_path = out_dir / "euc_kr.hpp"
    hpp_content = render_euc_kr_hpp(table)
    hpp_path.write_text(hpp_content, encoding="utf-8")

    if run_clang_format:
        clang_format_file(hpp_path)

    print(f"  generated: {hpp_path} ({len(table)} entries)")


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
    parser.add_argument(
        "--include-tables-dir",
        default=str(INCLUDE_TABLES),
        help="Output directory for include-tree table HPPs"
        " (default: include/beman/transcode/detail/tables)",
    )
    args = parser.parse_args(argv)

    in_dir = Path(args.in_dir)
    out_dir = Path(args.out_dir)
    include_tables_dir = Path(args.include_tables_dir)
    run_cf = not args.no_clang_format

    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"Generating single-byte tables from {in_dir}/ → {out_dir}/")
    for name in SINGLE_BYTE_INDEXES:
        generate_single_byte(name, in_dir, out_dir, run_clang_format=run_cf)

    print(f"Generating GBK table from {in_dir}/ → {include_tables_dir}/")
    generate_gbk(in_dir, include_tables_dir, run_clang_format=run_cf)

    print(f"Generating GB18030 ranges table from {in_dir}/ → {include_tables_dir}/")
    generate_gb18030_ranges(in_dir, include_tables_dir, run_clang_format=run_cf)

    print(f"Generating Big5 table from {in_dir}/ → {include_tables_dir}/")
    generate_big5(in_dir, include_tables_dir, run_clang_format=run_cf)

    print(f"Generating Shift_JIS table from {in_dir}/ → {include_tables_dir}/")
    generate_shift_jis(in_dir, include_tables_dir, run_clang_format=run_cf)

    print(f"Generating EUC-JP JIS X 0212 table from {in_dir}/ → {include_tables_dir}/")
    generate_euc_jp_jis0212(in_dir, include_tables_dir, run_clang_format=run_cf)

    print(f"Generating EUC-KR table from {in_dir}/ → {include_tables_dir}/")
    generate_euc_kr(in_dir, include_tables_dir, run_clang_format=run_cf)

    n = len(SINGLE_BYTE_INDEXES)
    print(
        f"Done. {n} single-byte + GBK + GB18030 ranges"
        " + Big5 + Shift_JIS + EUC-JP JIS X 0212 + EUC-KR."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
