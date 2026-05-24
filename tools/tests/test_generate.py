# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/generate_tables.py."""

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from generate_tables import (
    GBK_POINTER_COUNT,
    codec_to_guard,
    codec_to_identifier,
    parse_gbk_index,
    parse_single_byte_index,
    render_gbk_hpp,
    render_hpp,
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
