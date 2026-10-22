# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Each tool runs at one version, whichever of uv or pre-commit invokes it.

pyproject.toml is the source of truth (uv.lock is not committed). Dependabot
bumps pyproject.toml and the weekly autoupdate bumps
.pre-commit-config.yaml, independently. These tests fail as soon as one moves
without the other, so the bump PR is where the two get realigned.
"""

import re
import tomllib
from pathlib import Path

import pytest

ROOT = Path(__file__).parent.parent.parent

# pre-commit hook repository -> the uv package that provides the same tool.
PRE_COMMIT_REPOS = {
    "https://github.com/pre-commit/mirrors-clang-format": "clang-format",
    "https://github.com/astral-sh/ruff-pre-commit": "ruff",
    "https://github.com/EbodShojaei/bake": "mbake",
}

# Tools built from the same LLVM release.
LLVM_TOOLS = ("clang-format", "clang-tidy")


def pyproject_pins() -> dict[str, str]:
    with (ROOT / "pyproject.toml").open("rb") as f:
        project = tomllib.load(f)
    pins: dict[str, str] = {}
    for requirement in project["dependency-groups"]["dev"]:
        name, sep, version = requirement.partition("==")
        if sep:
            pins[name.strip()] = version.strip()
    return pins


def pre_commit_revs() -> dict[str, str]:
    config = (ROOT / ".pre-commit-config.yaml").read_text(encoding="utf-8")
    revs = re.findall(r"^  - repo: (\S+)\n\s+rev: (\S+)$", config, re.MULTILINE)
    return {repo: rev.removeprefix("v") for repo, rev in revs}


@pytest.mark.parametrize("repo,package", sorted(PRE_COMMIT_REPOS.items()))
def test_pre_commit_matches_pyproject(repo: str, package: str) -> None:
    pins = pyproject_pins()
    assert package in pins, f"pin {package} with == in pyproject.toml"
    assert pre_commit_revs()[repo] == pins[package]


def test_llvm_tools_share_a_release() -> None:
    pins = pyproject_pins()
    assert len({pins.get(tool) for tool in LLVM_TOOLS}) == 1, {
        tool: pins.get(tool) for tool in LLVM_TOOLS
    }
