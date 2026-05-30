#!/usr/bin/env bash

set -euo pipefail

exec go run github.com/zricethezav/gitleaks/v8@v8.30.1 git --pre-commit --redact --staged --verbose
