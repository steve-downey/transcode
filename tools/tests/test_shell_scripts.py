# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Behaviour tests for the repository's shell guard scripts."""

import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).parent.parent.parent
CHECK_FILENAMES = ROOT / "tools" / "check-filenames.sh"
RUN_GITLEAKS = ROOT / "tools" / "run-gitleaks.sh"
GENERATE = ROOT / "papers" / "wording" / "generate.sh"


def run_script(
    script: Path,
    *args: str,
    env: dict[str, str] | None = None,
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["bash", str(script), *args],
        capture_output=True,
        text=True,
        check=False,
        env=env,
    )


def test_check_filenames_accepts_clean_names() -> None:
    result = run_script(CHECK_FILENAMES, "src/good.cpp", "papers/wording.md")
    assert result.returncode == 0
    assert result.stderr == ""


def test_check_filenames_rejects_option_like_basename() -> None:
    result = run_script(CHECK_FILENAMES, "papers/-output.md")
    assert result.returncode == 1
    assert "papers/-output.md" in result.stderr
    assert "begins with a dash" in result.stderr


def write_executable(path: Path, contents: str) -> None:
    path.write_text(contents)
    path.chmod(0o755)


def gitleaks_environment(tmp_path: Path, *, git: int, protect: int) -> dict[str, str]:
    call_log = tmp_path / "call.log"
    write_executable(
        tmp_path / "gitleaks",
        """#!/bin/sh
case "$*" in
    "git --help") exit "$GITLEAKS_GIT_STATUS" ;;
    "protect --help") exit "$GITLEAKS_PROTECT_STATUS" ;;
esac
printf '%s\n' "$*" >"$CALL_LOG"
""",
    )
    env = os.environ.copy()
    env.update(
        {
            "PATH": f"{tmp_path}:/usr/bin:/bin",
            "CALL_LOG": str(call_log),
            "GITLEAKS_GIT_STATUS": str(git),
            "GITLEAKS_PROTECT_STATUS": str(protect),
        }
    )
    return env


def test_run_gitleaks_selects_modern_git_command(tmp_path: Path) -> None:
    env = gitleaks_environment(tmp_path, git=0, protect=1)
    result = run_script(RUN_GITLEAKS, env=env)
    assert result.returncode == 0
    assert (tmp_path / "call.log").read_text() == (
        "git --pre-commit --redact --staged --verbose\n"
    )


def test_run_gitleaks_selects_legacy_protect_command(tmp_path: Path) -> None:
    env = gitleaks_environment(tmp_path, git=1, protect=0)
    result = run_script(RUN_GITLEAKS, env=env)
    assert result.returncode == 0
    assert (tmp_path / "call.log").read_text() == (
        "protect --redact --staged --verbose\n"
    )


def test_run_gitleaks_rejects_unknown_installed_cli(tmp_path: Path) -> None:
    env = gitleaks_environment(tmp_path, git=1, protect=1)
    result = run_script(RUN_GITLEAKS, env=env)
    assert result.returncode == 1
    assert "neither a 'git' nor" in result.stderr
    assert not (tmp_path / "call.log").exists()


def test_run_gitleaks_falls_back_to_pinned_go_module(tmp_path: Path) -> None:
    call_log = tmp_path / "call.log"
    write_executable(
        tmp_path / "go",
        '#!/bin/sh\nprintf \'%s\\n\' "$*" >"$CALL_LOG"\n',
    )
    env = os.environ.copy()
    env.update({"PATH": f"{tmp_path}:/usr/bin:/bin", "CALL_LOG": str(call_log)})
    result = run_script(RUN_GITLEAKS, env=env)
    assert result.returncode == 0
    assert call_log.read_text() == (
        "run github.com/zricethezav/gitleaks/v8@v8.30.1 git --pre-commit "
        "--redact --staged --verbose\n"
    )


def wording_repository(tmp_path: Path) -> tuple[Path, Path, Path]:
    script = tmp_path / "repo" / "papers" / "wording" / "generate.sh"
    script.parent.mkdir(parents=True)
    shutil.copy2(GENERATE, script)
    include = tmp_path / "repo" / "include" / "beman" / "transcode"
    include.mkdir(parents=True)
    (include / "transcode.hpp").write_text("// test input\n")
    (include / "null_term.hpp").write_text("// test input\n")
    pending = script.parent / "PENDING"
    pending.write_text("keep this marker\n")

    specgen = tmp_path / "specgen"
    write_executable(specgen, "#!/bin/sh\nexit 0\n")
    return script, pending, specgen


def test_generate_scratch_run_preserves_pending(tmp_path: Path) -> None:
    script, pending, specgen = wording_repository(tmp_path)
    scratch = tmp_path / "scratch"
    env = os.environ.copy()
    env["SPECGEN"] = str(specgen)

    result = run_script(script, "--out", str(scratch), env=env)
    assert result.returncode == 0, result.stderr
    assert pending.read_text() == "keep this marker\n"


def test_generate_in_place_run_clears_pending_entries(tmp_path: Path) -> None:
    script, pending, specgen = wording_repository(tmp_path)
    env = os.environ.copy()
    env["SPECGEN"] = str(specgen)

    result = run_script(script, env=env)
    assert result.returncode == 0, result.stderr
    contents = pending.read_text()
    assert contents.startswith("# Headers that are knowingly ahead")
    assert "keep this marker" not in contents
