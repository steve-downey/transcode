#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Generate C++ test vector headers from WPT encoding test JS files."""

import re
import subprocess
import sys
from pathlib import Path

WPT_DIR = Path("docs/wpt")
TEST_DIR = Path("tests/beman/transcode")

GB18030_JS = WPT_DIR / "gb18030-decoder.any.js"
MISTAKES_JS = WPT_DIR / "textdecoder-mistakes.any.js"
ISO2022JP_JS = WPT_DIR / "iso-2022-jp-decoder.any.js"
SINGLEBYTE_JS = WPT_DIR / "single-byte-decoder.window.js"
UTF16SURROGATES_JS = WPT_DIR / "textdecoder-utf16-surrogates.any.js"
FATAL_JS = WPT_DIR / "textdecoder-fatal.any.js"
BOM_JS = WPT_DIR / "textdecoder-byte-order-marks.any.js"
FATAL_SINGLE_BYTE_JS = WPT_DIR / "textdecoder-fatal-single-byte.any.js"
EOF_JS = WPT_DIR / "textdecoder-eof.any.js"


def parse_js_string(s: str) -> list[int]:
    """Parse a JavaScript string literal content to list of Unicode codepoints.

    Handles \\uXXXX (with surrogate-pair combining), \\u{XXXXX}, \\xNN,
    and literal characters.
    """
    result: list[int] = []
    i = 0
    while i < len(s):
        if s[i] == "\\" and i + 1 < len(s):
            if s[i + 1] == "u":
                if i + 2 < len(s) and s[i + 2] == "{":
                    j = s.index("}", i + 3)
                    cp = int(s[i + 3 : j], 16)
                    result.append(cp)
                    i = j + 1
                else:
                    cp = int(s[i + 2 : i + 6], 16)
                    i += 6
                    # Combine surrogate pairs (\uD800-\uDBFF followed by \uDC00-\uDFFF)
                    if (
                        0xD800 <= cp <= 0xDBFF
                        and i + 5 <= len(s)
                        and s[i] == "\\"
                        and s[i + 1] == "u"
                        and s[i + 2] != "{"
                    ):
                        low = int(s[i + 2 : i + 6], 16)
                        if 0xDC00 <= low <= 0xDFFF:
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00)
                            i += 6
                    result.append(cp)
            elif s[i + 1] == "\\":
                result.append(ord("\\"))
                i += 2
            elif s[i + 1] == "n":
                result.append(ord("\n"))
                i += 2
            elif s[i + 1] == "r":
                result.append(ord("\r"))
                i += 2
            elif s[i + 1] == "t":
                result.append(ord("\t"))
                i += 2
            elif s[i + 1] == "x":
                cp = int(s[i + 2 : i + 4], 16)
                result.append(cp)
                i += 4
            else:
                result.append(ord(s[i + 1]))
                i += 2
        else:
            result.append(ord(s[i]))
            i += 1
    return result


_DECODE_RE = re.compile(
    r"decode\(\s*\[([^\]]*)\]\s*,\s*\"((?:[^\"\\]|\\.)*)\""
    r"(?:\s*,\s*\"((?:[^\"\\]|\\.)*)\")?\s*\)",
    re.MULTILINE,
)


def parse_gb18030_decode_vectors(content: str) -> list[dict[str, object]]:
    """Parse decode([bytes], "string", "description") calls from WPT JS."""
    vectors: list[dict[str, object]] = []
    for m in _DECODE_RE.finditer(content):
        bytes_str, expected_str, desc_str = m.groups()
        input_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        expected_cps = parse_js_string(expected_str)
        desc = desc_str if desc_str else ""
        vectors.append(
            {"input": input_bytes, "expected": expected_cps, "description": desc}
        )
    return vectors


_INVALID8_RE = re.compile(
    r"\{\s*bytes:\s*\[([^\]]*)\]\s*,\s*charcodes:\s*\[([^\]]*)\]\s*\}"
)


def parse_utf8_mistake_vectors(content: str) -> list[dict[str, object]]:
    """Parse { bytes: [...], charcodes: [...] } from textdecoder-mistakes.any.js.

    Only extracts the invalid8 section (UTF-8 invalid byte sequences).
    """
    invalid8_start = content.find("const invalid8")
    if invalid8_start == -1:
        return []
    bracket_start = content.find("[", invalid8_start)
    depth = 0
    invalid8_end = bracket_start
    for i in range(bracket_start, len(content)):
        if content[i] == "[":
            depth += 1
        elif content[i] == "]":
            depth -= 1
            if depth == 0:
                invalid8_end = i
                break
    section = content[invalid8_start : invalid8_end + 1]

    vectors: list[dict[str, object]] = []
    for m in _INVALID8_RE.finditer(section):
        bytes_str, charcodes_str = m.groups()
        input_bytes = [
            int(x.strip().replace("_", ""), 0)
            for x in bytes_str.split(",")
            if x.strip()
        ]
        expected_cps = [
            int(x.strip().replace("_", ""), 0)
            for x in charcodes_str.split(",")
            if x.strip()
        ]
        desc = f"bytes {input_bytes!r}"
        vectors.append(
            {"input": input_bytes, "expected": expected_cps, "description": desc}
        )
    return vectors


def parse_iso2022jp_decode_vectors(content: str) -> list[dict[str, object]]:
    """Parse decode([bytes], "string", "description") from ISO-2022-JP WPT JS.

    Same format as gb18030-decoder.any.js.
    """
    return parse_gb18030_decode_vectors(content)


_SINGLEBYTE_RE = re.compile(r'"([^"]+)"\s*:\s*\[([^\]]+)\]', re.MULTILINE)


def parse_single_byte_indexes(content: str) -> list[dict[str, object]]:
    """Parse singleByteIndexes = { "name": [...], ... } from WPT JS.

    Returns list of {"name": str, "codepoints": list[int]} where null
    entries in the source become 0xFFFD.
    """
    start = content.find("singleByteIndexes")
    if start == -1:
        return []
    brace_start = content.find("{", start)
    depth = 0
    brace_end = brace_start
    for i in range(brace_start, len(content)):
        if content[i] == "{":
            depth += 1
        elif content[i] == "}":
            depth -= 1
            if depth == 0:
                brace_end = i
                break
    section = content[brace_start : brace_end + 1]

    indexes: list[dict[str, object]] = []
    for m in _SINGLEBYTE_RE.finditer(section):
        name = m.group(1)
        values_str = m.group(2)
        codepoints: list[int] = []
        for token in values_str.split(","):
            token = token.strip()
            if token == "null":
                codepoints.append(0xFFFD)
            else:
                codepoints.append(int(token))
        indexes.append({"name": name, "codepoints": codepoints})
    return indexes


def _format_bytes(values: list[int]) -> str:
    """Format a list of byte values as a C++ initializer list."""
    return ", ".join(f"0x{v:02X}" for v in values)


def _format_codepoints(values: list[int]) -> str:
    """Format a list of codepoints as char32_t hex values."""
    return ", ".join(f"0x{v:04X}" for v in values)


def _escape_c_string(s: str) -> str:
    """Escape a string for use in a C string literal."""
    return s.replace("\\", "\\\\").replace('"', '\\"')


def render_gb18030_vectors_hpp(
    vectors: list[dict[str, object]], out_path: Path
) -> None:
    """Generate the C++ header for GB18030 WPT decode vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/gb18030-decoder.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/"
        "encoding/legacy-mb-schinese/gb18030"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_GB18030_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_GB18030_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptDecodeVector {")
    lines.append("    std::vector<uint8_t>  input;")
    lines.append("    std::vector<char32_t> expected;")
    lines.append("    const char*           description;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append("inline const WptDecodeVector gb18030_wpt_decode_vectors[] = {")

    for v in vectors:
        input_bytes: list[int] = v["input"]  # type: ignore[assignment]
        expected: list[int] = v["expected"]  # type: ignore[assignment]
        desc: str = v["description"]  # type: ignore[assignment]
        in_str = _format_bytes(input_bytes)
        exp_str = _format_codepoints(expected)
        desc_escaped = _escape_c_string(desc)
        lines.append(f'    {{{{{in_str}}}, {{{exp_str}}}, "{desc_escaped}"}},')

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_GB18030_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


def render_utf8_vectors_hpp(vectors: list[dict[str, object]], out_path: Path) -> None:
    """Generate the C++ header for UTF-8 WPT decode vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/textdecoder-mistakes.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptDecodeVector {")
    lines.append("    std::vector<uint8_t>  input;")
    lines.append("    std::vector<char32_t> expected;")
    lines.append("    const char*           description;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append("inline const WptDecodeVector utf8_wpt_decode_vectors[] = {")

    for v in vectors:
        input_bytes: list[int] = v["input"]  # type: ignore[assignment]
        expected: list[int] = v["expected"]  # type: ignore[assignment]
        desc: str = v["description"]  # type: ignore[assignment]
        in_str = _format_bytes(input_bytes)
        exp_str = _format_codepoints(expected)
        desc_escaped = _escape_c_string(desc)
        lines.append(f'    {{{{{in_str}}}, {{{exp_str}}}, "{desc_escaped}"}},')

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_UTF8_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


def render_iso2022jp_vectors_hpp(
    vectors: list[dict[str, object]], out_path: Path
) -> None:
    """Generate the C++ header for ISO-2022-JP WPT decode vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/iso-2022-jp-decoder.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_ISO2022JP_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_ISO2022JP_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptDecodeVector {")
    lines.append("    std::vector<uint8_t>  input;")
    lines.append("    std::vector<char32_t> expected;")
    lines.append("    const char*           description;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append("inline const WptDecodeVector iso2022jp_wpt_decode_vectors[] = {")

    for v in vectors:
        input_bytes: list[int] = v["input"]  # type: ignore[assignment]
        expected: list[int] = v["expected"]  # type: ignore[assignment]
        desc: str = v["description"]  # type: ignore[assignment]
        in_str = _format_bytes(input_bytes)
        exp_str = _format_codepoints(expected)
        desc_escaped = _escape_c_string(desc)
        lines.append(f'    {{{{{in_str}}}, {{{exp_str}}}, "{desc_escaped}"}},')

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_ISO2022JP_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


def render_single_byte_vectors_hpp(
    indexes: list[dict[str, object]], out_path: Path
) -> None:
    """Generate the C++ header for single-byte WPT index data."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/single-byte-decoder.window.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_SINGLE_BYTE_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_SINGLE_BYTE_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptSingleByteIndex {")
    lines.append("    const char* name;")
    lines.append("    char32_t    codepoints[128];")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr WptSingleByteIndex wpt_single_byte_indexes[] = {")

    for idx in indexes:
        name: str = idx["name"]  # type: ignore[assignment]
        cps: list[int] = idx["codepoints"]  # type: ignore[assignment]
        cp_strs = [f"0x{cp:04X}" for cp in cps]
        lines.append(f'    {{"{name}",')
        # Format codepoints in rows of 16
        lines.append("     {")
        for row_start in range(0, len(cp_strs), 16):
            row = cp_strs[row_start : row_start + 16]
            comma = "," if row_start + 16 < len(cp_strs) else ""
            lines.append("         " + ", ".join(row) + comma)
        lines.append("     }},")

    lines.append("};")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_SINGLE_BYTE_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


_UTF16_BAD_RE = re.compile(
    r"\{\s*encoding:\s*'([^']+)'\s*,\s*input:\s*\[([^\]]*)\]\s*,"
    r"\s*expected:\s*'((?:[^'\\]|\\.)*)'\s*,\s*name:\s*'([^']*)'\s*\}",
    re.MULTILINE | re.DOTALL,
)


def parse_utf16_surrogate_vectors(content: str) -> list[dict[str, object]]:
    """Parse the bad[] array from textdecoder-utf16-surrogates.any.js."""
    vectors: list[dict[str, object]] = []
    for m in _UTF16_BAD_RE.finditer(content):
        encoding, bytes_str, expected_str, name = m.groups()
        input_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        expected_cps = parse_js_string(expected_str)
        vectors.append(
            {
                "encoding": encoding,
                "input": input_bytes,
                "expected": expected_cps,
                "description": name,
            }
        )
    return vectors


def render_utf16_surrogates_vectors_hpp(
    vectors: list[dict[str, object]], out_path: Path
) -> None:
    """Generate the C++ header for UTF-16 surrogate WPT decode vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/textdecoder-utf16-surrogates.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptDecodeVector {")
    lines.append("    std::vector<uint8_t>  input;")
    lines.append("    std::vector<char32_t> expected;")
    lines.append("    const char*           description;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append(
        "inline const WptDecodeVector utf16le_surrogate_wpt_decode_vectors[] = {"
    )

    for v in vectors:
        input_bytes: list[int] = v["input"]  # type: ignore[assignment]
        expected: list[int] = v["expected"]  # type: ignore[assignment]
        desc: str = v["description"]  # type: ignore[assignment]
        in_str = _format_bytes(input_bytes)
        exp_str = _format_codepoints(expected)
        desc_escaped = _escape_c_string(desc)
        lines.append(f'    {{{{{in_str}}}, {{{exp_str}}}, "{desc_escaped}"}},')

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_UTF16_SURROGATES_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


_FATAL_RE = re.compile(
    r"\{\s*encoding:\s*'([^']+)'\s*,\s*input:\s*\[([^\]]*)\]\s*,"
    r"\s*name:\s*'([^']*)'\s*\}",
    re.MULTILINE | re.DOTALL,
)


def parse_fatal_vectors(content: str) -> list[dict[str, object]]:
    """Parse the bad[] array from textdecoder-fatal.any.js."""
    vectors: list[dict[str, object]] = []
    for m in _FATAL_RE.finditer(content):
        encoding, bytes_str, name = m.groups()
        input_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        vectors.append(
            {
                "encoding": encoding,
                "input": input_bytes,
                "description": name,
            }
        )
    return vectors


def render_fatal_vectors_hpp(vectors: list[dict[str, object]], out_path: Path) -> None:
    """Generate the C++ header for WPT fatal-mode decode vectors."""
    utf8_vectors = [v for v in vectors if v["encoding"] == "utf-8"]
    utf16le_vectors = [v for v in vectors if v["encoding"] == "utf-16le"]

    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/textdecoder-fatal.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptFatalVector {")
    lines.append("    std::vector<uint8_t> input;")
    lines.append("    const char*          description;")
    lines.append("};")
    lines.append("")

    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append("inline const WptFatalVector utf8_fatal_wpt_vectors[] = {")
    for v in utf8_vectors:
        input_bytes: list[int] = v["input"]  # type: ignore[assignment]
        desc: str = v["description"]  # type: ignore[assignment]
        in_str = _format_bytes(input_bytes)
        desc_escaped = _escape_c_string(desc)
        lines.append(f'    {{{{{in_str}}}, "{desc_escaped}"}},')
    lines.append("};")
    lines.append("")

    lines.append("inline const WptFatalVector utf16le_fatal_wpt_vectors[] = {")
    for v in utf16le_vectors:
        input_bytes2: list[int] = v["input"]  # type: ignore[assignment]
        desc2: str = v["description"]  # type: ignore[assignment]
        in_str2 = _format_bytes(input_bytes2)
        desc2_escaped = _escape_c_string(desc2)
        lines.append(f'    {{{{{in_str2}}}, "{desc2_escaped}"}},')
    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_FATAL_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


_BOM_CASE_RE = re.compile(
    r"\{\s*encoding:\s*'([^']+)'\s*,\s*bom:\s*\[([^\]]*)\]\s*,"
    r"\s*bytes:\s*\[([^\]]*)\]\s*\}",
    re.MULTILINE | re.DOTALL,
)

_BOM_STRING_RE = re.compile(r"var string = '([^']*(?:\\.[^']*)*)'")


def parse_bom_vectors(content: str) -> list[dict[str, object]]:
    """Parse testCases from textdecoder-byte-order-marks.any.js.

    Returns list of {"encoding": str, "bom": list[int], "bytes": list[int],
    "expected": list[int]}.
    """
    string_match = _BOM_STRING_RE.search(content)
    if not string_match:
        return []
    expected_cps = parse_js_string(string_match.group(1))

    vectors: list[dict[str, object]] = []
    for m in _BOM_CASE_RE.finditer(content):
        encoding, bom_str, bytes_str = m.groups()
        bom_bytes = [int(x.strip(), 0) for x in bom_str.split(",") if x.strip()]
        input_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        vectors.append(
            {
                "encoding": encoding,
                "bom": bom_bytes,
                "bytes": input_bytes,
                "expected": expected_cps,
            }
        )
    return vectors


def render_bom_vectors_hpp(vectors: list[dict[str, object]], out_path: Path) -> None:
    """Generate the C++ header for WPT BOM decode vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/textdecoder-byte-order-marks.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptBomCase {")
    lines.append("    std::vector<uint8_t>  bytes;")
    lines.append("    std::vector<uint8_t>  bom;")
    lines.append("    std::vector<char32_t> expected;")
    lines.append("    const char*           encoding;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append("inline const WptBomCase wpt_bom_cases[] = {")

    for v in vectors:
        bom: list[int] = v["bom"]  # type: ignore[assignment]
        input_bytes: list[int] = v["bytes"]  # type: ignore[assignment]
        expected: list[int] = v["expected"]  # type: ignore[assignment]
        encoding: str = v["encoding"]  # type: ignore[assignment]
        bom_str = _format_bytes(bom)
        in_str = _format_bytes(input_bytes)
        exp_str = _format_codepoints(expected)
        lines.append(
            f'    {{{{{in_str}}}, {{{bom_str}}}, {{{exp_str}}}, "{encoding}"}},'
        )

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_BOM_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


_SBFATAL_RE = re.compile(
    r"\{encoding:\s*'([^']+)'\s*,\s*bad:\s*\[([^\]]*)\]\s*\}",
    re.MULTILINE,
)


def parse_fatal_single_byte_cases(content: str) -> list[dict[str, object]]:
    """Parse singleByteEncodings from textdecoder-fatal-single-byte.any.js.

    Returns list of {"encoding": str, "bad": list[int]}.
    """
    cases: list[dict[str, object]] = []
    for m in _SBFATAL_RE.finditer(content):
        encoding, bad_str = m.groups()
        bad = [int(x.strip(), 0) for x in bad_str.split(",") if x.strip()]
        cases.append({"encoding": encoding, "bad": bad})
    return cases


def render_fatal_single_byte_vectors_hpp(
    cases: list[dict[str, object]], out_path: Path
) -> None:
    """Generate the C++ header for WPT fatal single-byte vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/textdecoder-fatal-single-byte.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_FATAL_SINGLE_BYTE_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_FATAL_SINGLE_BYTE_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptFatalSingleByteCase {")
    lines.append("    const char*          encoding;")
    lines.append("    std::vector<uint8_t> bad;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append(
        "inline const WptFatalSingleByteCase wpt_fatal_single_byte_cases[] = {"
    )

    for c in cases:
        encoding: str = c["encoding"]  # type: ignore[assignment]
        bad: list[int] = c["bad"]  # type: ignore[assignment]
        bad_str = _format_bytes(bad)
        lines.append(f'    {{"{encoding}", {{{bad_str}}}}},')

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_FATAL_SINGLE_BYTE_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


_EOF_RE = re.compile(
    r"assert_equals\("
    r'new TextDecoder\("?([^")\s]*)"?\)'
    r"\.decode\(new Uint8Array\(\[([^\]]*)\]\)\)"
    r',\s*"((?:[^"\\]|\\.)*)"\)',
    re.MULTILINE,
)

_EOF_END_MARKER = '}, "TextDecoder end-of-queue handling");'


def parse_eof_vectors(content: str) -> list[dict[str, object]]:
    """Parse non-streaming assert_equals from textdecoder-eof.any.js.

    Returns list of {"encoding": str, "input": list[int], "expected": list[int]}.
    Only parses the first test() block (before the streaming block).
    """
    end = content.find(_EOF_END_MARKER)
    if end == -1:
        return []
    section = content[:end]
    vectors: list[dict[str, object]] = []
    for m in _EOF_RE.finditer(section):
        encoding, bytes_str, expected_str = m.groups()
        if not encoding:
            encoding = "utf-8"
        input_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        expected_cps = parse_js_string(expected_str)
        vectors.append(
            {"encoding": encoding, "input": input_bytes, "expected": expected_cps}
        )
    return vectors


def render_eof_vectors_hpp(vectors: list[dict[str, object]], out_path: Path) -> None:
    """Generate the C++ header for WPT EOF decode vectors."""
    lines: list[str] = []
    lines.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    lines.append(
        "// GENERATED — do not edit. Regenerate: uv run tools/generate_wpt_vectors.py"
    )
    lines.append("//")
    lines.append("// Source: docs/wpt/textdecoder-eof.any.js")
    lines.append(
        "// WPT: https://github.com/web-platform-tests/wpt/tree/master/encoding"
    )
    lines.append("// License: W3C 3-Clause BSD License")
    lines.append("// Note: only non-streaming cases are included; streaming")
    lines.append("//       (stream:true) cases require stateful API not provided")
    lines.append("//       by this library's range-view interface.")
    lines.append("")
    lines.append("#ifndef TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP")
    lines.append("#define TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include <vector>")
    lines.append("")
    lines.append("namespace beman::transcoding::tests::wpt {")
    lines.append("")
    lines.append("struct WptEofVector {")
    lines.append("    const char*           encoding;")
    lines.append("    std::vector<uint8_t>  input;")
    lines.append("    std::vector<char32_t> expected;")
    lines.append("};")
    lines.append("")
    lines.append("// NOLINTBEGIN(cert-err58-cpp)")
    lines.append("inline const WptEofVector wpt_eof_vectors[] = {")

    for v in vectors:
        encoding: str = v["encoding"]  # type: ignore[assignment]
        input_bytes: list[int] = v["input"]  # type: ignore[assignment]
        expected: list[int] = v["expected"]  # type: ignore[assignment]
        in_str = _format_bytes(input_bytes)
        exp_str = _format_codepoints(expected)
        lines.append(f'    {{"{encoding}", {{{in_str}}}, {{{exp_str}}}}},')

    lines.append("};")
    lines.append("// NOLINTEND(cert-err58-cpp)")
    lines.append("")
    lines.append("} // namespace beman::transcoding::tests::wpt")
    lines.append("")
    lines.append("#endif // TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))


def main() -> int:
    gb18030_content = GB18030_JS.read_text()
    gb18030_vectors = parse_gb18030_decode_vectors(gb18030_content)
    print(f"Parsed {len(gb18030_vectors)} GB18030 decode vectors")
    render_gb18030_vectors_hpp(gb18030_vectors, TEST_DIR / "wpt_gb18030_vectors.hpp")
    print(f"Generated {TEST_DIR / 'wpt_gb18030_vectors.hpp'}")

    mistakes_content = MISTAKES_JS.read_text()
    utf8_vectors = parse_utf8_mistake_vectors(mistakes_content)
    print(f"Parsed {len(utf8_vectors)} UTF-8 mistake vectors")
    render_utf8_vectors_hpp(utf8_vectors, TEST_DIR / "wpt_utf8_vectors.hpp")
    print(f"Generated {TEST_DIR / 'wpt_utf8_vectors.hpp'}")

    iso2022jp_content = ISO2022JP_JS.read_text()
    iso2022jp_vectors = parse_iso2022jp_decode_vectors(iso2022jp_content)
    print(f"Parsed {len(iso2022jp_vectors)} ISO-2022-JP decode vectors")
    render_iso2022jp_vectors_hpp(
        iso2022jp_vectors, TEST_DIR / "wpt_iso2022jp_vectors.hpp"
    )
    print(f"Generated {TEST_DIR / 'wpt_iso2022jp_vectors.hpp'}")

    singlebyte_content = SINGLEBYTE_JS.read_text()
    singlebyte_indexes = parse_single_byte_indexes(singlebyte_content)
    print(f"Parsed {len(singlebyte_indexes)} single-byte indexes")
    render_single_byte_vectors_hpp(
        singlebyte_indexes, TEST_DIR / "wpt_single_byte_vectors.hpp"
    )
    print(f"Generated {TEST_DIR / 'wpt_single_byte_vectors.hpp'}")

    utf16surr_content = UTF16SURROGATES_JS.read_text()
    utf16surr_vectors = parse_utf16_surrogate_vectors(utf16surr_content)
    print(f"Parsed {len(utf16surr_vectors)} UTF-16 surrogate vectors")
    render_utf16_surrogates_vectors_hpp(
        utf16surr_vectors, TEST_DIR / "wpt_utf16_surrogates_vectors.hpp"
    )
    print(f"Generated {TEST_DIR / 'wpt_utf16_surrogates_vectors.hpp'}")

    fatal_content = FATAL_JS.read_text()
    fatal_vectors = parse_fatal_vectors(fatal_content)
    print(f"Parsed {len(fatal_vectors)} fatal mode vectors")
    render_fatal_vectors_hpp(fatal_vectors, TEST_DIR / "wpt_fatal_vectors.hpp")
    print(f"Generated {TEST_DIR / 'wpt_fatal_vectors.hpp'}")

    bom_content = BOM_JS.read_text()
    bom_vectors = parse_bom_vectors(bom_content)
    print(f"Parsed {len(bom_vectors)} BOM cases")
    render_bom_vectors_hpp(bom_vectors, TEST_DIR / "wpt_bom_vectors.hpp")
    print(f"Generated {TEST_DIR / 'wpt_bom_vectors.hpp'}")

    fatal_sb_content = FATAL_SINGLE_BYTE_JS.read_text()
    fatal_sb_cases = parse_fatal_single_byte_cases(fatal_sb_content)
    print(f"Parsed {len(fatal_sb_cases)} fatal single-byte cases")
    render_fatal_single_byte_vectors_hpp(
        fatal_sb_cases, TEST_DIR / "wpt_fatal_single_byte_vectors.hpp"
    )
    print(f"Generated {TEST_DIR / 'wpt_fatal_single_byte_vectors.hpp'}")

    eof_content = EOF_JS.read_text()
    eof_vectors = parse_eof_vectors(eof_content)
    print(f"Parsed {len(eof_vectors)} EOF vectors")
    render_eof_vectors_hpp(eof_vectors, TEST_DIR / "wpt_eof_vectors.hpp")
    print(f"Generated {TEST_DIR / 'wpt_eof_vectors.hpp'}")

    run_cf = True
    try:
        subprocess.run(["clang-format", "--version"], capture_output=True, check=True)
    except (FileNotFoundError, subprocess.CalledProcessError):
        run_cf = False

    if run_cf:
        for hpp in [
            TEST_DIR / "wpt_gb18030_vectors.hpp",
            TEST_DIR / "wpt_utf8_vectors.hpp",
            TEST_DIR / "wpt_iso2022jp_vectors.hpp",
            TEST_DIR / "wpt_single_byte_vectors.hpp",
            TEST_DIR / "wpt_utf16_surrogates_vectors.hpp",
            TEST_DIR / "wpt_fatal_vectors.hpp",
            TEST_DIR / "wpt_bom_vectors.hpp",
            TEST_DIR / "wpt_fatal_single_byte_vectors.hpp",
            TEST_DIR / "wpt_eof_vectors.hpp",
        ]:
            subprocess.run(["clang-format", "-i", str(hpp)], check=True)
            print(f"Formatted {hpp}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
