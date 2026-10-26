# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Every header under include/beman/transcode/ is in the install list.

include/beman/transcode/CMakeLists.txt names the installed headers one by
one. A header added to the tree but not to that list builds and tests fine
from the source checkout and then fails `make testinstall`, because the
installed umbrella header includes a file that was never copied. This test
fails at `make pytest` instead, before the push.
"""

import re
from pathlib import Path

ROOT = Path(__file__).parent.parent.parent
INCLUDE_DIR = ROOT / "include" / "beman" / "transcode"

# Configured at build time from config_generated.hpp.in and installed from the
# build tree, so it is not listed by its source name.
GENERATED = {"config_generated.hpp.in"}


def headers_on_disk() -> set[str]:
    return {
        path.relative_to(INCLUDE_DIR).as_posix()
        for path in INCLUDE_DIR.rglob("*")
        if path.is_file() and path.suffix in {".hpp", ".in"}
    } - GENERATED


def headers_in_install_list() -> set[str]:
    text = (INCLUDE_DIR / "CMakeLists.txt").read_text()
    match = re.search(r"set\(BEMAN_TRANSCODE_HEADERS\s+(.*?)\)", text, re.DOTALL)
    assert match is not None, "BEMAN_TRANSCODE_HEADERS not found"
    # The configured header is listed by its build-tree path; skip it.
    return {entry for entry in match.group(1).split() if "${" not in entry}


def test_every_header_is_installed() -> None:
    missing = headers_on_disk() - headers_in_install_list()
    assert not missing, f"not in BEMAN_TRANSCODE_HEADERS: {sorted(missing)}"


def test_install_list_names_only_real_headers() -> None:
    stale = headers_in_install_list() - headers_on_disk()
    assert not stale, f"in BEMAN_TRANSCODE_HEADERS but not on disk: {sorted(stale)}"
