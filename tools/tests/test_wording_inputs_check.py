# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for papers/wording/inputs-check.sh.

The script is the cheap half of the wording drift gate: it compares the hashes
of the headers the wording was generated from against the hashes of the tree as
it stands, and allows a difference only when the header is named in
papers/wording/PENDING.

Every case drives the script with explicit --committed / --current / --pending
files, so nothing here depends on the repository's real headers or on a specgen
being installed.
"""

import os
import subprocess
from pathlib import Path

import pytest

SCRIPT = Path(__file__).parent.parent.parent / "papers" / "wording" / "inputs-check.sh"

# Two plausible hash lines, in the `sha256sum` format the script parses.
HEADER_A = "include/beman/transcode/error.hpp"
HEADER_B = "include/beman/transcode/null_term.hpp"
HEADER_COLLATION_A = "include/beman/transcode/codec.hpp"
HEADER_COLLATION_B = "include/beman/transcode/codec_concepts.hpp"
HASH_1 = "1" * 64
HASH_2 = "2" * 64


def hashes(*pairs: tuple[str, str]) -> str:
    """Render (hash, path) pairs the way `sha256sum` does."""
    return "".join(f"{digest}  {path}\n" for digest, path in pairs)


def run_check(
    tmp_path: Path,
    committed: str,
    current: str,
    pending: str | None = None,
    env: dict[str, str] | None = None,
) -> subprocess.CompletedProcess[str]:
    """Drive the script against three literal files and return the result."""
    committed_file = tmp_path / "inputs.sha256"
    current_file = tmp_path / "current.sha256"
    committed_file.write_text(committed)
    current_file.write_text(current)
    argv = [
        str(SCRIPT),
        "--committed",
        str(committed_file),
        "--current",
        str(current_file),
    ]
    if pending is not None:
        pending_file = tmp_path / "PENDING"
        pending_file.write_text(pending)
        argv += ["--pending", str(pending_file)]
    return subprocess.run(argv, capture_output=True, text=True, check=False, env=env)


def require_locale(name: str) -> str:
    """Return the installed spelling of name, or skip on minimal CI images."""
    result = subprocess.run(
        ["locale", "-a"], capture_output=True, text=True, check=True
    )
    normalized_name = name.lower().replace("-", "")
    for installed in result.stdout.splitlines():
        if installed.lower().replace("-", "") == normalized_name:
            return installed
    pytest.skip(f"{name} is not installed")


def test_no_drift_passes(tmp_path: Path) -> None:
    """Matching hashes and no pending list: the pre-existing happy path."""
    same = hashes((HASH_1, HEADER_A), (HASH_2, HEADER_B))
    result = run_check(tmp_path, same, same, pending="")
    assert result.returncode == 0
    assert "unchanged since the fragments were generated" in result.stdout


def test_unlisted_change_fails(tmp_path: Path) -> None:
    """A header that moved without being listed: the drift this gate is for."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A)),
        hashes((HASH_2, HEADER_A)),
        pending="",
    )
    assert result.returncode == 1
    assert HEADER_A in result.stderr
    assert "Run 'make wording'" in result.stderr


def test_listed_change_reports_and_passes(tmp_path: Path) -> None:
    """A header that moved and is listed: reported, not fatal."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A)),
        hashes((HASH_2, HEADER_A)),
        pending=f"{HEADER_A}\n",
    )
    assert result.returncode == 0
    assert "regeneration pending" in result.stdout
    assert HEADER_A in result.stdout


def test_stale_entry_fails(tmp_path: Path) -> None:
    """A listed header that did not change: the exemption has outlived its use."""
    same = hashes((HASH_1, HEADER_A))
    result = run_check(tmp_path, same, same, pending=f"{HEADER_A}\n")
    assert result.returncode == 1
    assert "has not changed" in result.stderr
    assert HEADER_A in result.stderr


def test_listed_and_unlisted_together_fails(tmp_path: Path) -> None:
    """One allowed change does not excuse an unlisted one beside it."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A), (HASH_1, HEADER_B)),
        hashes((HASH_2, HEADER_A), (HASH_2, HEADER_B)),
        pending=f"{HEADER_A}\n",
    )
    assert result.returncode == 1
    assert HEADER_B in result.stderr
    assert "Run 'make wording'" in result.stderr


def test_path_collation_is_independent_of_callers_locale(tmp_path: Path) -> None:
    """C-sorted paths must also be compared under the C locale."""
    env = os.environ.copy()
    env["LC_ALL"] = require_locale("en_US.UTF-8")
    result = run_check(
        tmp_path,
        hashes(
            (HASH_1, HEADER_COLLATION_A),
            (HASH_1, HEADER_COLLATION_B),
        ),
        hashes(
            (HASH_2, HEADER_COLLATION_A),
            (HASH_2, HEADER_COLLATION_B),
        ),
        pending=f"{HEADER_COLLATION_A}\n{HEADER_COLLATION_B}\n",
        env=env,
    )
    assert result.returncode == 0
    assert "regeneration pending" in result.stdout
    assert result.stderr == ""


def test_comments_and_blank_lines_ignored(tmp_path: Path) -> None:
    """The shipped PENDING is all comments, and must behave as an empty list."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A)),
        hashes((HASH_2, HEADER_A)),
        pending="# a comment\n\n   \n",
    )
    assert result.returncode == 1
    assert HEADER_A in result.stderr


def test_trailing_comment_on_an_entry(tmp_path: Path) -> None:
    """An entry may carry a note saying which step will regenerate it."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A)),
        hashes((HASH_2, HEADER_A)),
        pending=f"{HEADER_A}  # regenerated by p6-step8\n",
    )
    assert result.returncode == 0
    assert "regeneration pending" in result.stdout


def test_added_header_is_drift(tmp_path: Path) -> None:
    """A header that joined the document extent also makes the fragments stale."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A)),
        hashes((HASH_1, HEADER_A), (HASH_2, HEADER_B)),
        pending="",
    )
    assert result.returncode == 1
    assert HEADER_B in result.stderr


def test_removed_header_is_drift(tmp_path: Path) -> None:
    """So does one that left it."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A), (HASH_2, HEADER_B)),
        hashes((HASH_1, HEADER_A)),
        pending="",
    )
    assert result.returncode == 1
    assert HEADER_B in result.stderr


def test_missing_pending_file_behaves_as_empty(tmp_path: Path) -> None:
    """The list is optional; without it the gate is exactly what it was."""
    result = run_check(
        tmp_path,
        hashes((HASH_1, HEADER_A)),
        hashes((HASH_2, HEADER_A)),
        pending=None,
    )
    assert result.returncode == 1
    assert HEADER_A in result.stderr


def test_missing_committed_file_is_a_usage_error(tmp_path: Path) -> None:
    """Distinguish "cannot check" from "check failed"."""
    current_file = tmp_path / "current.sha256"
    current_file.write_text(hashes((HASH_1, HEADER_A)))
    result = subprocess.run(
        [
            str(SCRIPT),
            "--committed",
            str(tmp_path / "absent.sha256"),
            "--current",
            str(current_file),
        ],
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 2
    assert "no such file" in result.stderr


def test_unknown_argument_is_a_usage_error(tmp_path: Path) -> None:
    result = subprocess.run(
        [str(SCRIPT), "--nonsense"], capture_output=True, text=True, check=False
    )
    assert result.returncode == 2
    assert "unknown argument" in result.stderr


def test_shipped_pending_file_is_empty_of_entries() -> None:
    """`main` must never carry a live exemption."""
    pending = SCRIPT.parent / "PENDING"
    entries = [
        line.split("#", 1)[0].strip()
        for line in pending.read_text().splitlines()
        if line.split("#", 1)[0].strip()
    ]
    assert entries == []
