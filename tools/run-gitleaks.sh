#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Scan the staged changes for secrets, with whatever gitleaks is available.
#
# gitleaks renamed the staged-scan command: it was `protect --staged` through
# v8.18, and became `git --staged` in v8.19.  CI installs a pinned 8.30.1 and
# gets the new spelling; a developer machine has whatever the distribution
# ships, and on an older one the hook failed with
#
#     Error: unknown command "git" for "gitleaks"
#
# on every file, staged or not.  A hook that always fails is worse than no
# hook, because it teaches everyone to read past a red result -- and it failed
# locally while passing in CI, which is the combination least likely to get
# noticed.
#
# So pick the spelling the installed binary actually has.

set -euo pipefail

common_args=(--redact --staged --verbose)

if command -v gitleaks &>/dev/null; then
    if gitleaks git --help &>/dev/null; then
        exec gitleaks git --pre-commit "${common_args[@]}"
    elif gitleaks protect --help &>/dev/null; then
        # v8.18 and earlier.  `protect` implied a pre-commit scan, so there is
        # no --pre-commit to pass.
        exec gitleaks protect "${common_args[@]}"
    else
        echo "run-gitleaks.sh: the installed gitleaks has neither a 'git' nor" >&2
        echo "a 'protect' command.  Install v8.19 or later, or remove it from" >&2
        echo "PATH to fall back to 'go run'." >&2
        exit 1
    fi
else
    exec go run github.com/zricethezav/gitleaks/v8@v8.30.1 git --pre-commit "${common_args[@]}"
fi
