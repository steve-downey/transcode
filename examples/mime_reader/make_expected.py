# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Writes the expected output of the MIME reader into expected/.

The summary is made with Python's standard ``email`` package, not with the C++
reader, so that the expected files are an independent opinion about what the
messages say.  ``python3 make_expected.py`` regenerates them.

The format is the one main.cpp prints:

    Subject: <decoded>
    From: <decoded>
    part <path>
      content-type: <type/subtype>
      charset: <charset, or - for a part that is not text>
      transfer-encoding: <mechanism>
      characters: <code points>        (text parts)
      text:
        > <line>
      bytes: <count>                   (other parts)

A text part whose charset Python does not know prints ``error: unknown charset``
in place of the characters.
"""

import codecs
import email.policy
from email.message import EmailMessage, MIMEPart
from email.parser import BytesParser
from pathlib import Path


def leaf_summary(part: MIMEPart, path: str) -> list[str]:
    content_type = part.get_content_type()
    is_text = part.get_content_maintype() == "text"
    charset = (part.get_content_charset() or "us-ascii") if is_text else "-"
    encoding = str(part.get("Content-Transfer-Encoding", "7bit")).strip().lower()
    lines = [
        f"part {path}",
        f"  content-type: {content_type}",
        f"  charset: {charset}",
        f"  transfer-encoding: {encoding}",
    ]
    payload = part.get_payload(decode=True)
    assert isinstance(payload, bytes)
    if not is_text:
        return [*lines, f"  bytes: {len(payload)}"]
    try:
        codecs.lookup(charset)
    except LookupError:
        return [*lines, f"  error: unknown charset '{charset}'"]
    text = payload.decode(charset, errors="replace").replace("\r\n", "\n")
    text_lines = text.split("\n")
    if text_lines[-1] == "":
        text_lines.pop()
    lines += [f"  characters: {len(text)}", "  text:"]
    lines += ["  > " + line if line else "  >" for line in text_lines]
    return lines


def parts_summary(node: MIMEPart, path: str) -> list[str]:
    if not node.is_multipart():
        return leaf_summary(node, path or "1")
    lines: list[str] = []
    for i, child in enumerate(node.iter_parts(), start=1):
        lines += parts_summary(child, f"{path}.{i}" if path else str(i))
    return lines


def summarize(raw: bytes) -> str:
    parser = BytesParser(_class=EmailMessage, policy=email.policy.default)
    message = parser.parsebytes(raw)
    lines: list[str] = []
    for name in ("Subject", "From"):
        value = message[name]
        if value is not None:
            lines.append(f"{name}: {value}")
    lines += parts_summary(message, "")
    return "\n".join(lines) + "\n"


def main() -> None:
    here = Path(__file__).resolve().parent
    expected = here / "expected"
    expected.mkdir(exist_ok=True)
    for message in sorted((here / "messages").glob("*.eml")):
        summary = summarize(message.read_bytes())
        (expected / f"{message.stem}.txt").write_text(
            summary, encoding="utf-8", newline="\n"
        )


if __name__ == "__main__":
    main()
