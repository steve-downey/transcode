#!/usr/bin/env bash

set -euo pipefail

if command -v gitleaks &>/dev/null; then
    exec gitleaks git --pre-commit --redact --staged --verbose
else
    exec go run github.com/zricethezav/gitleaks/v8@v8.30.1 git --pre-commit --redact --staged --verbose
fi
