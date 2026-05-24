# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/generate_tables.py."""

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from generate_tables import (
    BIG5_POINTER_COUNT,
    EUC_JP_JIS0212_POINTER_COUNT,
    EUC_KR_POINTER_COUNT,
    GBK_POINTER_COUNT,
    SHIFTJIS_POINTER_COUNT,
    codec_to_guard,
    codec_to_identifier,
    parse_big5_index,
    parse_euc_jp_jis0212_index,
    parse_euc_kr_index,
    parse_gb18030_ranges,
    parse_gbk_index,
    parse_shift_jis_index,
    parse_single_byte_index,
    render_big5_hpp,
    render_euc_jp_jis0212_hpp,
    render_euc_kr_hpp,
    render_gb18030_ranges_hpp,
    render_gbk_hpp,
    render_hpp,
    render_shift_jis_hpp,
    write_bin,
)


def _make_index_file(tmp_path: Path, entries: dict[int, int]) -> Path:
    """Write a minimal WHATWG index file with the given pointer→codepoint map."""
    lines = [
        "# test index file",
        "#",
    ]
    for ptr, cp in sorted(entries.items()):
        lines.append(f"  {ptr}\t0x{cp:04X}\t# comment")
    path = tmp_path / "index-test.txt"
    path.write_text("\n".join(lines) + "\n")
    return path


def test_codec_to_identifier_hyphens() -> None:
    assert codec_to_identifier("iso-8859-2") == "iso_8859_2"


def test_codec_to_identifier_ibm866() -> None:
    assert codec_to_identifier("ibm866") == "ibm866"


def test_codec_to_identifier_x_mac_cyrillic() -> None:
    assert codec_to_identifier("x-mac-cyrillic") == "x_mac_cyrillic"


def test_codec_to_guard() -> None:
    assert codec_to_guard("iso-8859-2") == "DATA_TABLES_ISO_8859_2_HPP"


def test_codec_to_guard_windows_1252() -> None:
    assert codec_to_guard("windows-1252") == "DATA_TABLES_WINDOWS_1252_HPP"


def test_parse_single_byte_index_length(tmp_path: Path) -> None:
    path = _make_index_file(tmp_path, {0: 0x00A0, 1: 0x0104})
    table = parse_single_byte_index(path)
    assert len(table) == 128


def test_parse_single_byte_index_known_entry(tmp_path: Path) -> None:
    path = _make_index_file(tmp_path, {0: 0x20AC})
    table = parse_single_byte_index(path)
    assert table[0] == 0x20AC


def test_parse_single_byte_index_unmapped_is_zero(tmp_path: Path) -> None:
    path = _make_index_file(tmp_path, {0: 0x00A0})
    table = parse_single_byte_index(path)
    # All entries except 0 should be 0 (unmapped)
    assert table[1] == 0
    assert table[127] == 0


def test_parse_single_byte_index_windows_1252_spot_check(tmp_path: Path) -> None:
    """windows-1252 index: pointer 0 → U+20AC (EURO SIGN)."""
    path = Path("docs/whatwg/index-windows-1252.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_single_byte_index(path)
    assert table[0] == 0x20AC  # byte 0x80 → EURO SIGN


def test_parse_single_byte_index_iso_8859_2_spot_check(tmp_path: Path) -> None:
    """iso-8859-2: pointer 32 (byte 0xA0) → U+00A0 (NO-BREAK SPACE)."""
    path = Path("docs/whatwg/index-iso-8859-2.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_single_byte_index(path)
    assert table[32] == 0x00A0  # byte 0xA0 → NO-BREAK SPACE


def test_write_bin_length(tmp_path: Path) -> None:
    table = [0] * 128
    dest = tmp_path / "test.bin"
    write_bin(table, dest)
    assert dest.stat().st_size == 128 * 4


def test_write_bin_little_endian(tmp_path: Path) -> None:
    table = [0] * 128
    table[0] = 0x20AC
    dest = tmp_path / "test.bin"
    write_bin(table, dest)
    raw = dest.read_bytes()
    first_entry = struct.unpack_from("<I", raw, 0)[0]
    assert first_entry == 0x20AC


def test_write_bin_zero_unmapped(tmp_path: Path) -> None:
    table = [0] * 128
    dest = tmp_path / "test.bin"
    write_bin(table, dest)
    raw = dest.read_bytes()
    # All entries should be zero
    for i in range(128):
        val = struct.unpack_from("<I", raw, i * 4)[0]
        assert val == 0


def test_render_hpp_contains_guard(tmp_path: Path) -> None:
    table = [0] * 128
    hpp = render_hpp("iso-8859-2", table)
    assert "DATA_TABLES_ISO_8859_2_HPP" in hpp


def test_render_hpp_contains_identifier(tmp_path: Path) -> None:
    table = [0] * 128
    hpp = render_hpp("iso-8859-2", table)
    assert "iso_8859_2[128]" in hpp


def test_render_hpp_contains_namespace(tmp_path: Path) -> None:
    table = [0] * 128
    hpp = render_hpp("ibm866", table)
    assert "beman::transcoding::detail::tables" in hpp


def test_render_hpp_contains_spdx(tmp_path: Path) -> None:
    table = [0] * 128
    hpp = render_hpp("ibm866", table)
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_hpp_contains_generated_marker(tmp_path: Path) -> None:
    table = [0] * 128
    hpp = render_hpp("ibm866", table)
    assert "GENERATED" in hpp


def test_render_hpp_known_codepoint(tmp_path: Path) -> None:
    table = [0] * 128
    table[0] = 0x20AC  # EURO SIGN at pointer 0
    hpp = render_hpp("windows-1252", table)
    assert "0x20AC" in hpp


def test_render_hpp_zero_entry_is_bare_zero(tmp_path: Path) -> None:
    table = [0] * 128
    hpp = render_hpp("test-codec", table)
    # Unmapped entries should appear as bare 0, not 0x0000
    assert "0x0000" not in hpp


# ---------------------------------------------------------------------------
# GBK table tests
# ---------------------------------------------------------------------------


def _make_gbk_index_file(tmp_path: Path, entries: dict[int, int]) -> Path:
    """Write a minimal WHATWG index-gb18030-style file."""
    lines = [
        "# test gbk index file",
        "#",
    ]
    for ptr, cp in sorted(entries.items()):
        lines.append(f"  {ptr}\t0x{cp:04X}\t# comment")
    path = tmp_path / "index-gb18030.txt"
    path.write_text("\n".join(lines) + "\n")
    return path


def test_parse_gbk_index_length(tmp_path: Path) -> None:
    path = _make_gbk_index_file(tmp_path, {0: 0x4E02})
    table = parse_gbk_index(path)
    assert len(table) == GBK_POINTER_COUNT
    assert GBK_POINTER_COUNT == 23940


def test_parse_gbk_index_known_entry(tmp_path: Path) -> None:
    path = _make_gbk_index_file(tmp_path, {0: 0x4E02})
    table = parse_gbk_index(path)
    assert table[0] == 0x4E02


def test_parse_gbk_index_unmapped_is_zero(tmp_path: Path) -> None:
    path = _make_gbk_index_file(tmp_path, {0: 0x4E02})
    table = parse_gbk_index(path)
    assert table[1] == 0


def test_parse_gbk_index_ignores_out_of_range(tmp_path: Path) -> None:
    # Pointers >= 23940 should be ignored (gb18030 extended range)
    path = _make_gbk_index_file(tmp_path, {0: 0x4E02, 23940: 0x1234})
    table = parse_gbk_index(path)
    assert len(table) == GBK_POINTER_COUNT
    assert table[0] == 0x4E02


def test_parse_gbk_index_real_spot_check_index0(tmp_path: Path) -> None:
    """WHATWG spec: pointer 0 (lead 0x81, trail 0x40) -> U+4E02."""
    path = Path("docs/whatwg/index-gb18030.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_gbk_index(path)
    assert table[0] == 0x4E02


def test_parse_gbk_index_real_spot_check_index1(tmp_path: Path) -> None:
    """WHATWG spec: pointer 1 (lead 0x81, trail 0x41) -> U+4E04."""
    path = Path("docs/whatwg/index-gb18030.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_gbk_index(path)
    assert table[1] == 0x4E04


def test_render_gbk_hpp_contains_guard() -> None:
    table = [0] * GBK_POINTER_COUNT
    hpp = render_gbk_hpp(table)
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GBK_HPP" in hpp


def test_render_gbk_hpp_contains_array_size() -> None:
    table = [0] * GBK_POINTER_COUNT
    hpp = render_gbk_hpp(table)
    assert f"gbk[{GBK_POINTER_COUNT}]" in hpp


def test_render_gbk_hpp_contains_namespace() -> None:
    table = [0] * GBK_POINTER_COUNT
    hpp = render_gbk_hpp(table)
    assert "beman::transcoding::detail::tables" in hpp


def test_render_gbk_hpp_contains_spdx() -> None:
    table = [0] * GBK_POINTER_COUNT
    hpp = render_gbk_hpp(table)
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_gbk_hpp_known_codepoint() -> None:
    table = [0] * GBK_POINTER_COUNT
    table[0] = 0x4E02
    hpp = render_gbk_hpp(table)
    assert "0x4E02" in hpp


# ---------------------------------------------------------------------------
# GB18030 ranges tests
# ---------------------------------------------------------------------------


def _make_gb18030_ranges_file(tmp_path: Path, entries: list[tuple[int, int]]) -> Path:
    """Write a minimal index-gb18030-ranges.txt with given (pointer, codepoint) pairs.

    Returns the path to the written file.
    """
    lines = ["# test gb18030-ranges index", "#"]
    for ptr, cp in entries:
        lines.append(f"  {ptr}\t0x{cp:04X}")
    path = tmp_path / "index-gb18030-ranges.txt"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def test_parse_gb18030_ranges_empty(tmp_path: Path) -> None:
    path = _make_gb18030_ranges_file(tmp_path, [])
    result = parse_gb18030_ranges(path)
    assert result == []


def test_parse_gb18030_ranges_single_entry(tmp_path: Path) -> None:
    path = _make_gb18030_ranges_file(tmp_path, [(0, 0x0080)])
    result = parse_gb18030_ranges(path)
    assert len(result) == 1
    assert result[0] == (0, 0x0080)


def test_parse_gb18030_ranges_sorted(tmp_path: Path) -> None:
    path = _make_gb18030_ranges_file(tmp_path, [(36, 0x00A5), (0, 0x0080)])
    result = parse_gb18030_ranges(path)
    assert result[0] == (0, 0x0080)
    assert result[1] == (36, 0x00A5)


def test_parse_gb18030_ranges_ignores_comments(tmp_path: Path) -> None:
    lines = ["# comment line", "  0\t0x0080", "# another comment", "  36\t0x00A5"]
    path = tmp_path / "index-gb18030-ranges.txt"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    result = parse_gb18030_ranges(path)
    assert len(result) == 2


def test_parse_gb18030_ranges_real_first_entry() -> None:
    path = Path("docs/whatwg/index-gb18030-ranges.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    result = parse_gb18030_ranges(path)
    assert result[0] == (0, 0x0080)


def test_parse_gb18030_ranges_real_count() -> None:
    path = Path("docs/whatwg/index-gb18030-ranges.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    result = parse_gb18030_ranges(path)
    assert len(result) == 207


def test_parse_gb18030_ranges_real_last_entry() -> None:
    path = Path("docs/whatwg/index-gb18030-ranges.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    result = parse_gb18030_ranges(path)
    assert result[-1] == (189000, 0x10000)


def test_render_gb18030_ranges_hpp_contains_guard() -> None:
    hpp = render_gb18030_ranges_hpp([])
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GB18030_RANGES_HPP" in hpp


def test_render_gb18030_ranges_hpp_contains_spdx() -> None:
    hpp = render_gb18030_ranges_hpp([])
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_gb18030_ranges_hpp_contains_namespace() -> None:
    hpp = render_gb18030_ranges_hpp([])
    assert "beman::transcoding::detail::tables" in hpp


def test_render_gb18030_ranges_hpp_count() -> None:
    entries = [(0, 0x0080), (36, 0x00A5)]
    hpp = render_gb18030_ranges_hpp(entries)
    assert "gb18030_ranges[2]" in hpp
    assert "gb18030_ranges_count = 2" in hpp


def test_render_gb18030_ranges_hpp_known_entry() -> None:
    entries = [(0, 0x0080), (189000, 0x10000)]
    hpp = render_gb18030_ranges_hpp(entries)
    assert "0x0080" in hpp
    assert "189000" in hpp
    assert "0x10000" in hpp


# ---------------------------------------------------------------------------
# Big5 table tests
# ---------------------------------------------------------------------------


def _make_big5_index_file(tmp_path: Path, entries: dict[int, int]) -> Path:
    """Write a minimal WHATWG index-big5.txt with given pointer→codepoint map."""
    lines = [
        "# test big5 index file",
        "#",
    ]
    for ptr, cp in sorted(entries.items()):
        lines.append(f"  {ptr}\t0x{cp:04X}\t# comment")
    path = tmp_path / "index-big5.txt"
    path.write_text("\n".join(lines) + "\n")
    return path


def test_big5_pointer_count() -> None:
    assert BIG5_POINTER_COUNT == 19782


def test_parse_big5_index_length(tmp_path: Path) -> None:
    path = _make_big5_index_file(tmp_path, {5495: 0x4E00})
    table = parse_big5_index(path)
    assert len(table) == BIG5_POINTER_COUNT


def test_parse_big5_index_known_entry(tmp_path: Path) -> None:
    path = _make_big5_index_file(tmp_path, {5495: 0x4E00})
    table = parse_big5_index(path)
    assert table[5495] == 0x4E00


def test_parse_big5_index_unmapped_is_zero(tmp_path: Path) -> None:
    path = _make_big5_index_file(tmp_path, {5495: 0x4E00})
    table = parse_big5_index(path)
    assert table[0] == 0


def test_parse_big5_index_ignores_out_of_range(tmp_path: Path) -> None:
    path = _make_big5_index_file(tmp_path, {0: 0x4E02, 19782: 0x1234})
    table = parse_big5_index(path)
    assert len(table) == BIG5_POINTER_COUNT
    assert table[0] == 0x4E02


def test_parse_big5_index_real_spot_check_pointer5495() -> None:
    """WHATWG spec: pointer 5495 -> U+4E00 (一)."""
    path = Path("docs/whatwg/index-big5.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_big5_index(path)
    assert table[5495] == 0x4E00


def test_parse_big5_index_real_special_pointers_null() -> None:
    """Pointers 1133, 1135, 1164 are null in the table (special-cased in decoder)."""
    path = Path("docs/whatwg/index-big5.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_big5_index(path)
    assert table[1133] == 0
    assert table[1135] == 0
    assert table[1164] == 0


def test_render_big5_hpp_contains_guard() -> None:
    table = [0] * BIG5_POINTER_COUNT
    hpp = render_big5_hpp(table)
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_BIG5_HPP" in hpp


def test_render_big5_hpp_contains_array_size() -> None:
    table = [0] * BIG5_POINTER_COUNT
    hpp = render_big5_hpp(table)
    assert f"big5[{BIG5_POINTER_COUNT}]" in hpp


def test_render_big5_hpp_contains_namespace() -> None:
    table = [0] * BIG5_POINTER_COUNT
    hpp = render_big5_hpp(table)
    assert "beman::transcoding::detail::tables" in hpp


def test_render_big5_hpp_contains_spdx() -> None:
    table = [0] * BIG5_POINTER_COUNT
    hpp = render_big5_hpp(table)
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_big5_hpp_known_codepoint() -> None:
    table = [0] * BIG5_POINTER_COUNT
    table[5495] = 0x4E00
    hpp = render_big5_hpp(table)
    assert "0x4E00" in hpp


# ---------------------------------------------------------------------------
# Shift_JIS table tests
# ---------------------------------------------------------------------------


def _make_shift_jis_index_file(tmp_path: Path, entries: dict[int, int]) -> Path:
    """Write a minimal WHATWG index-jis0208.txt with given pointer→codepoint map."""
    lines = [
        "# test shift_jis index file",
        "#",
    ]
    for ptr, cp in sorted(entries.items()):
        lines.append(f"  {ptr}\t0x{cp:04X}\t# comment")
    path = tmp_path / "index-jis0208.txt"
    path.write_text("\n".join(lines) + "\n")
    return path


def test_shift_jis_pointer_count() -> None:
    assert SHIFTJIS_POINTER_COUNT == 11280


def test_parse_shift_jis_index_length(tmp_path: Path) -> None:
    path = _make_shift_jis_index_file(tmp_path, {0: 0x3000})
    table = parse_shift_jis_index(path)
    assert len(table) == SHIFTJIS_POINTER_COUNT


def test_parse_shift_jis_index_known_entry(tmp_path: Path) -> None:
    path = _make_shift_jis_index_file(tmp_path, {0: 0x3000})
    table = parse_shift_jis_index(path)
    assert table[0] == 0x3000


def test_parse_shift_jis_index_unmapped_is_zero(tmp_path: Path) -> None:
    path = _make_shift_jis_index_file(tmp_path, {0: 0x3000})
    table = parse_shift_jis_index(path)
    assert table[1] == 0


def test_parse_shift_jis_index_ignores_out_of_range(tmp_path: Path) -> None:
    path = _make_shift_jis_index_file(tmp_path, {0: 0x3000, 11280: 0x1234})
    table = parse_shift_jis_index(path)
    assert len(table) == SHIFTJIS_POINTER_COUNT
    assert table[0] == 0x3000


def test_parse_shift_jis_index_real_pointer0() -> None:
    """WHATWG spec: pointer 0 (lead 0x81, trail 0x40) -> U+3000 (IDEOGRAPHIC SPACE)."""
    path = Path("docs/whatwg/index-jis0208.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_shift_jis_index(path)
    assert table[0] == 0x3000


def test_parse_shift_jis_index_real_pointer1485() -> None:
    """WHATWG spec: pointer 1485 (lead 0x88, trail 0xEA) -> U+4E00 (一)."""
    path = Path("docs/whatwg/index-jis0208.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_shift_jis_index(path)
    assert table[1485] == 0x4E00


def test_render_shift_jis_hpp_contains_guard() -> None:
    table = [0] * SHIFTJIS_POINTER_COUNT
    hpp = render_shift_jis_hpp(table)
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_SHIFT_JIS_HPP" in hpp


def test_render_shift_jis_hpp_contains_array_size() -> None:
    table = [0] * SHIFTJIS_POINTER_COUNT
    hpp = render_shift_jis_hpp(table)
    assert f"shift_jis[{SHIFTJIS_POINTER_COUNT}]" in hpp


def test_render_shift_jis_hpp_contains_namespace() -> None:
    table = [0] * SHIFTJIS_POINTER_COUNT
    hpp = render_shift_jis_hpp(table)
    assert "beman::transcoding::detail::tables" in hpp


def test_render_shift_jis_hpp_contains_spdx() -> None:
    table = [0] * SHIFTJIS_POINTER_COUNT
    hpp = render_shift_jis_hpp(table)
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_shift_jis_hpp_known_codepoint() -> None:
    table = [0] * SHIFTJIS_POINTER_COUNT
    table[0] = 0x3000
    hpp = render_shift_jis_hpp(table)
    assert "0x3000" in hpp


# ---------------------------------------------------------------------------
# EUC-JP JIS X 0212 table
# ---------------------------------------------------------------------------


def _make_jis0212_index_file(tmp_path: Path, entries: dict[int, int]) -> Path:
    """Write a minimal index-jis0212.txt with given pointer→codepoint map."""
    lines = [
        "# test jis0212 index file",
        "#",
    ]
    for ptr, cp in sorted(entries.items()):
        lines.append(f"  {ptr}\t0x{cp:04X}\t# comment")
    path = tmp_path / "index-jis0212.txt"
    path.write_text("\n".join(lines) + "\n")
    return path


def test_euc_jp_jis0212_pointer_count() -> None:
    assert EUC_JP_JIS0212_POINTER_COUNT == 8836


def test_parse_euc_jp_jis0212_index_length(tmp_path: Path) -> None:
    path = _make_jis0212_index_file(tmp_path, {108: 0x02D8})
    table = parse_euc_jp_jis0212_index(path)
    assert len(table) == EUC_JP_JIS0212_POINTER_COUNT


def test_parse_euc_jp_jis0212_index_known_entry(tmp_path: Path) -> None:
    path = _make_jis0212_index_file(tmp_path, {108: 0x02D8})
    table = parse_euc_jp_jis0212_index(path)
    assert table[108] == 0x02D8


def test_parse_euc_jp_jis0212_index_unmapped_is_zero(tmp_path: Path) -> None:
    path = _make_jis0212_index_file(tmp_path, {108: 0x02D8})
    table = parse_euc_jp_jis0212_index(path)
    assert table[0] == 0


def test_parse_euc_jp_jis0212_index_ignores_out_of_range(tmp_path: Path) -> None:
    path = _make_jis0212_index_file(tmp_path, {108: 0x02D8, 8836: 0x1234})
    table = parse_euc_jp_jis0212_index(path)
    assert len(table) == EUC_JP_JIS0212_POINTER_COUNT
    assert table[108] == 0x02D8


def test_parse_euc_jp_jis0212_index_real_pointer108() -> None:
    """WHATWG spec: pointer 108 -> U+02D8 (BREVE). EUC-JP: 0x8F 0xA2 0xAF."""
    path = Path("docs/whatwg/index-jis0212.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_euc_jp_jis0212_index(path)
    assert table[108] == 0x02D8


def test_render_euc_jp_jis0212_hpp_contains_guard() -> None:
    table = [0] * EUC_JP_JIS0212_POINTER_COUNT
    hpp = render_euc_jp_jis0212_hpp(table)
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_JP_JIS0212_HPP" in hpp


def test_render_euc_jp_jis0212_hpp_contains_array_size() -> None:
    table = [0] * EUC_JP_JIS0212_POINTER_COUNT
    hpp = render_euc_jp_jis0212_hpp(table)
    assert f"euc_jp_jis0212[{EUC_JP_JIS0212_POINTER_COUNT}]" in hpp


def test_render_euc_jp_jis0212_hpp_contains_namespace() -> None:
    table = [0] * EUC_JP_JIS0212_POINTER_COUNT
    hpp = render_euc_jp_jis0212_hpp(table)
    assert "beman::transcoding::detail::tables" in hpp


def test_render_euc_jp_jis0212_hpp_contains_spdx() -> None:
    table = [0] * EUC_JP_JIS0212_POINTER_COUNT
    hpp = render_euc_jp_jis0212_hpp(table)
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_euc_jp_jis0212_hpp_known_codepoint() -> None:
    table = [0] * EUC_JP_JIS0212_POINTER_COUNT
    table[108] = 0x02D8
    hpp = render_euc_jp_jis0212_hpp(table)
    assert "0x02D8" in hpp


# ---------------------------------------------------------------------------
# EUC-KR table
# ---------------------------------------------------------------------------


def _make_euc_kr_index_file(tmp_path: Path, entries: dict[int, int]) -> Path:
    """Write a minimal index-euc-kr.txt with given pointer→codepoint map."""
    lines = [
        "# test euc-kr index file",
        "#",
    ]
    for ptr, cp in sorted(entries.items()):
        lines.append(f"  {ptr}\t0x{cp:04X}\t# comment")
    path = tmp_path / "index-euc-kr.txt"
    path.write_text("\n".join(lines) + "\n")
    return path


def test_euc_kr_pointer_count() -> None:
    assert EUC_KR_POINTER_COUNT == 23940


def test_parse_euc_kr_index_length(tmp_path: Path) -> None:
    path = _make_euc_kr_index_file(tmp_path, {0: 0xAC02})
    table = parse_euc_kr_index(path)
    assert len(table) == EUC_KR_POINTER_COUNT


def test_parse_euc_kr_index_known_entry(tmp_path: Path) -> None:
    path = _make_euc_kr_index_file(tmp_path, {0: 0xAC02})
    table = parse_euc_kr_index(path)
    assert table[0] == 0xAC02


def test_parse_euc_kr_index_unmapped_is_zero(tmp_path: Path) -> None:
    path = _make_euc_kr_index_file(tmp_path, {0: 0xAC02})
    table = parse_euc_kr_index(path)
    assert table[1] == 0


def test_parse_euc_kr_index_ignores_out_of_range(tmp_path: Path) -> None:
    path = _make_euc_kr_index_file(tmp_path, {0: 0xAC02, 23940: 0x1234})
    table = parse_euc_kr_index(path)
    assert len(table) == EUC_KR_POINTER_COUNT
    assert table[0] == 0xAC02


def test_parse_euc_kr_index_real_pointer9026() -> None:
    """WHATWG spec: pointer 9026 -> U+AC00 (가). EUC-KR: 0xB0 0xA2."""
    path = Path("docs/whatwg/index-euc-kr.txt")
    if not path.exists():
        import pytest

        pytest.skip("docs/whatwg not present")
    table = parse_euc_kr_index(path)
    assert table[9026] == 0xAC00


def test_render_euc_kr_hpp_contains_guard() -> None:
    table = [0] * EUC_KR_POINTER_COUNT
    hpp = render_euc_kr_hpp(table)
    assert "INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_EUC_KR_HPP" in hpp


def test_render_euc_kr_hpp_contains_array_size() -> None:
    table = [0] * EUC_KR_POINTER_COUNT
    hpp = render_euc_kr_hpp(table)
    assert f"euc_kr[{EUC_KR_POINTER_COUNT}]" in hpp


def test_render_euc_kr_hpp_contains_namespace() -> None:
    table = [0] * EUC_KR_POINTER_COUNT
    hpp = render_euc_kr_hpp(table)
    assert "beman::transcoding::detail::tables" in hpp


def test_render_euc_kr_hpp_contains_spdx() -> None:
    table = [0] * EUC_KR_POINTER_COUNT
    hpp = render_euc_kr_hpp(table)
    assert "SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception" in hpp


def test_render_euc_kr_hpp_known_codepoint() -> None:
    table = [0] * EUC_KR_POINTER_COUNT
    table[9026] = 0xAC00
    hpp = render_euc_kr_hpp(table)
    assert "0xAC00" in hpp
