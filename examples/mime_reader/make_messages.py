# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Writes the test messages for the MIME reader into messages/.

Run it from anywhere: ``python3 make_messages.py``.  The messages are
committed, so running it is only needed to change them; it overwrites them.

A MIME message is bytes, and several of these are not valid UTF-8 as a whole
(an ISO-2022-JP part, a Shift_JIS part, a KOI8-R body), so they are built as
bytes and written as bytes.  Python's codecs do the encoding, which is the
point: the reader's expected output (make_expected.py) is likewise made by
Python, independently of the C++.
"""

import base64
import binascii
from pathlib import Path

CRLF = b"\r\n"
LF = b"\n"


def header(name: str, value: str) -> bytes:
    return f"{name}: {value}".encode("ascii")


def base64_lines(data: bytes, eol: bytes) -> bytes:
    """RFC 2045 base64: lines of at most 76 characters."""
    encoded = base64.b64encode(data)
    return eol.join(encoded[i : i + 76] for i in range(0, len(encoded), 76)) + eol


def quoted_printable(data: bytes, eol: bytes) -> bytes:
    """RFC 2045 quoted-printable, with soft line breaks, and `eol` line ends."""
    encoded = binascii.b2a_qp(data, istext=True)
    return encoded.replace(LF, eol)


def with_eol(text: str, eol: bytes, encoding: str) -> bytes:
    """`text`, written with '\\n' line breaks, as bytes with `eol` line breaks."""
    return text.replace("\n", eol.decode("ascii")).encode(encoding)


def entity(headers: list[bytes], body: bytes, eol: bytes = CRLF) -> bytes:
    """Header fields, the blank line, and a body."""
    return eol.join(headers) + eol + eol + body


def multipart(
    boundary: str,
    parts: list[bytes],
    preamble: bytes = b"",
    epilogue: bytes = b"",
    eol: bytes = CRLF,
) -> bytes:
    """RFC 2046: a delimiter before each part, and a close delimiter."""
    dash = b"--" + boundary.encode("ascii")
    out = preamble + eol if preamble else b""
    for part in parts:
        out += dash + eol + part + eol
    out += dash + b"--" + eol
    return out + epilogue


def iso_2022_jp_alternative() -> bytes:
    japanese = "こんにちは、世界。\nこれは日本語のテストです。\n"
    html = "<p>こんにちは、世界。</p>\n<p>これは HTML の部分です。</p>\n"
    plain_part = entity(
        [
            header("Content-Type", "text/plain; charset=iso-2022-jp"),
            header("Content-Transfer-Encoding", "7bit"),
        ],
        with_eol(japanese, CRLF, "iso2022_jp"),
    )
    html_part = entity(
        [
            header("Content-Type", 'text/html; charset="utf-8"'),
            header("Content-Transfer-Encoding", "quoted-printable"),
        ],
        quoted_printable(html.encode("utf-8"), CRLF),
    )
    top = [
        header("From", "Taro Yamada <taro@example.jp>"),
        header("To", "reader@example.org"),
        header("Subject", "Greetings from Tokyo"),
        header("MIME-Version", "1.0"),
        header("Content-Type", 'multipart/alternative; boundary="alt-boundary-1"'),
    ]
    body = multipart(
        "alt-boundary-1",
        [plain_part, html_part],
        preamble=b"This is a multi-part message in MIME format.",
        epilogue=b"This epilogue is skipped.\r\n",
    )
    return entity(top, body)


def shift_jis_base64() -> bytes:
    intro = entity(
        [
            header("Content-Type", "text/plain; charset=us-ascii"),
            header("Content-Transfer-Encoding", "7bit"),
        ],
        b"The attachment is a Shift_JIS text file.\r\n",
    )
    sjis_text = "吾輩は猫である。名前はまだ無い。\nどこで生れたかとんと見当がつかぬ。\n"
    attachment = entity(
        [
            header("Content-Type", 'text/plain; charset="Shift_JIS"; name="note.txt"'),
            header("Content-Transfer-Encoding", "base64"),
            header("Content-Disposition", 'attachment; filename="note.txt"'),
        ],
        base64_lines(with_eol(sjis_text, CRLF, "shift_jis"), CRLF),
    )
    blob = entity(
        [
            header("Content-Type", "Application/Octet-Stream"),
            header("Content-Transfer-Encoding", "base64"),
        ],
        base64_lines(bytes(range(256)), CRLF),
    )
    top = [
        header("From", "archive@example.org"),
        header("Subject", "Attachments"),
        header("MIME-Version", "1.0"),
        header("Content-Type", 'multipart/mixed; boundary="=_mixed_B"'),
    ]
    return entity(top, multipart("=_mixed_B", [intro, attachment, blob]))


def windows_1252_quoted_printable() -> bytes:
    # A message with bare LF line ends throughout, as a file saved on Unix has.
    text = (
        "Café crème costs €4,50 – “quoted” and ‘single’.\n"
        "The next line is long enough that quoted-printable must break it with a "
        "soft line break, and it has an = sign and an é in it.\n"
        "100% sure, half-width: ½ ¾ ©\n"
    )
    top = [
        header("From", "cafe@example.org"),
        header("Subject", "Menu"),
        header("MIME-Version", "1.0"),
        header("Content-Type", 'text/plain; charset="windows-1252"'),
        header("Content-Transfer-Encoding", "quoted-printable"),
    ]
    return entity(top, quoted_printable(text.encode("cp1252"), LF), eol=LF)


def utf_8_base64() -> bytes:
    text = (
        "Ünïcödé: ñ, ß, œ.\n"
        "日本語, 한국어, Русский, ελληνικά\n"
        "Beyond the BMP: \U0001f600 \U0001d11e \U00020bb7\n"
    )
    top = [
        header("From", "unicode@example.org"),
        header("Subject", "Base64 UTF-8"),
        header("MIME-Version", "1.0"),
        header("Content-Type", "TEXT/Plain; CHARSET=UTF-8; format=flowed"),
        header("Content-Transfer-Encoding", "Base64"),
    ]
    return entity(top, base64_lines(with_eol(text, CRLF, "utf-8"), CRLF))


def koi8_r_subject() -> bytes:
    def word(text: str) -> bytes:
        return b"=?KOI8-R?B?" + base64.b64encode(text.encode("koi8_r")) + b"?="

    subject = (
        b"Subject: Re: "
        + word("Тестовое сообщение")
        + CRLF
        + b" "
        + word(" для проверки кодировки")
    )
    body = "Привет, мир!\nЭто сообщение в кодировке KOI8-R.\n"
    top = [
        header("From", "=?iso-8859-1?Q?Andr=E9_Dupont?= <andre@example.org>"),
        subject,
        header("MIME-Version", "1.0"),
        header("Content-Type", "text/plain; charset=koi8-r"),
        header("Content-Transfer-Encoding", "8bit"),
    ]
    return entity(top, with_eol(body, CRLF, "koi8_r"))


def nested_multipart() -> bytes:
    # The inner boundary begins with the whole of the outer one, which is
    # allowed, and a reader must not take "--outer boundary:1x" for a delimiter
    # of the outer message.
    outer = "outer boundary:1"
    inner = "outer boundary:1x"
    plain = entity(
        [
            header("Content-Type", "text/plain; charset=utf-8"),
            header("Content-Transfer-Encoding", "7bit"),
        ],
        b"Plain text, the first alternative.\r\n\r\nWith a blank line.\r\n",
    )
    html = entity(
        [
            header("Content-Type", "text/html; charset=utf-8"),
            header("Content-Transfer-Encoding", "base64"),
        ],
        base64_lines("<p>HTML, with é, the second alternative.</p>\n".encode(), CRLF),
    )
    alternative = entity(
        [header("Content-Type", f'multipart/alternative; boundary="{inner}"')],
        multipart(inner, [plain, html]),
    )
    pdf = entity(
        [
            header("Content-Type", "application/pdf; name=report.pdf"),
            header("Content-Transfer-Encoding", "base64"),
            header("Content-Disposition", "attachment; filename=report.pdf"),
        ],
        base64_lines(b"%PDF-1.4\n%\xe2\xe3\xcf\xd3\n1 0 obj\n<<>>\nendobj\n", CRLF),
    )
    top = [
        header("From", "nested@example.org"),
        header("Subject", "Nested multipart"),
        header("MIME-Version", "1.0"),
        header("Content-Type", f'multipart/mixed; boundary="{outer}"'),
    ]
    body = multipart(
        outer,
        [alternative, pdf],
        preamble=b"Preamble: not part of any part.",
        epilogue=b"Epilogue: nor is this.\r\n",
    )
    return entity(top, body)


def unknown_charset() -> bytes:
    fine = entity(
        [
            header("Content-Type", "text/plain; charset=utf-8"),
            header("Content-Transfer-Encoding", "7bit"),
        ],
        b"This part is fine.\r\n",
    )
    made_up = entity(
        [
            header("Content-Type", "text/plain; charset=x-made-up"),
            header("Content-Transfer-Encoding", "7bit"),
        ],
        b"This part declares a charset that nobody has heard of.\r\n",
    )
    # No Content-Type: text/plain, us-ascii (RFC 2045 section 5.2).
    default = entity(
        [header("Content-Transfer-Encoding", "7bit")],
        b"This part declares nothing at all.\r\n",
    )
    top = [
        header("From", "mystery@example.org"),
        header("Subject", "An unknown charset"),
        header("MIME-Version", "1.0"),
        header("Content-Type", 'multipart/mixed; boundary="unknown-1"'),
    ]
    return entity(top, multipart("unknown-1", [fine, made_up, default]))


MESSAGES = {
    "iso-2022-jp-alternative": iso_2022_jp_alternative,
    "shift-jis-base64": shift_jis_base64,
    "windows-1252-quoted-printable": windows_1252_quoted_printable,
    "utf-8-base64": utf_8_base64,
    "koi8-r-subject": koi8_r_subject,
    "nested-multipart": nested_multipart,
    "unknown-charset": unknown_charset,
}


def main() -> None:
    out = Path(__file__).resolve().parent / "messages"
    out.mkdir(exist_ok=True)
    for name, make in MESSAGES.items():
        (out / f"{name}.eml").write_bytes(make())


if __name__ == "__main__":
    main()
