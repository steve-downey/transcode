#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Benchmark matrix runner.
#
# Runs each supported compiler/optimization slice, prints environment metadata
# before results, and skips unavailable slices cleanly.
#
# Usage:
#   infra/scripts/bench-matrix.sh [TAG_FILTER]
#
# TAG_FILTER defaults to "[smoke]".  Pass "[benchmark]" for the full run.

set -euo pipefail

TAG_FILTER="${1:-[smoke]}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
TIMESTAMP="$(date -u '+%Y-%m-%dT%H:%M:%SZ')"

# ---------------------------------------------------------------------------
# Environment metadata
# ---------------------------------------------------------------------------

print_meta() {
    echo "# ===== BENCH-MATRIX METADATA ====="
    echo "# TIMESTAMP: ${TIMESTAMP}"
    echo "# HOSTNAME: $(hostname)"
    echo "# OS: $(uname -srm)"
    if [ -r /etc/os-release ]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        echo "# DISTRO: ${PRETTY_NAME:-unknown}"
    fi
    if [ -r /proc/cpuinfo ]; then
        CPU_MODEL="$(grep -m1 "model name" /proc/cpuinfo | cut -d: -f2 | xargs)"
        echo "# CPU: ${CPU_MODEL}"
        CPU_COUNT="$(nproc 2>/dev/null || grep -c "^processor" /proc/cpuinfo)"
        echo "# CPU_CORES: ${CPU_COUNT}"
    fi
    if command -v gcc >/dev/null 2>&1; then
        echo "# GCC: $(gcc --version | head -1)"
    else
        echo "# GCC: not found"
    fi
    if command -v clang >/dev/null 2>&1; then
        echo "# CLANG: $(clang --version | head -1)"
    else
        echo "# CLANG: not found"
    fi
    echo "# TAG_FILTER: ${TAG_FILTER}"
    echo "# ================================="
}

# ---------------------------------------------------------------------------
# Matrix slice runner
# ---------------------------------------------------------------------------

run_slice() {
    local preset="$1"
    local label="$2"
    local compiler_check="$3"

    echo ""
    echo ">>> Matrix slice: ${label}"

    if ! command -v "${compiler_check}" >/dev/null 2>&1; then
        echo "    SKIP: ${compiler_check} not found"
        return 0
    fi

    local build_dir="${ROOT}/build/${preset}"

    # Configure if build dir absent or cache missing
    if [ ! -f "${build_dir}/CMakeCache.txt" ]; then
        echo "    Configuring ${preset}..."
        if ! (cd "${ROOT}" && cmake --preset "${preset}"); then
            echo "    SKIP: cmake --preset ${preset} failed (unsupported on this machine)"
            return 0
        fi
    fi

    # Build benchmark executables only
    echo "    Building benchmarks for ${preset}..."
    if ! cmake --build "${build_dir}" \
            --target beman.transcode.benchmarks.smoke 2>/dev/null; then
        echo "    SKIP: build failed for ${preset}"
        return 0
    fi

    local bench="${build_dir}/benchmark/beman.transcode.benchmarks.smoke"
    if [ ! -x "${bench}" ]; then
        echo "    SKIP: benchmark binary not found at ${bench}"
        return 0
    fi

    echo "    Running ${TAG_FILTER} benchmarks..."
    "${bench}" "${TAG_FILTER}"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

print_meta

run_slice "gcc-release"     "GCC -O3"          "gcc"
run_slice "gcc-release-lto" "GCC -O3 -flto"    "gcc"
run_slice "llvm-release-lto" "Clang -O3 -flto" "clang"

echo ""
echo ">>> Matrix run complete: ${TIMESTAMP}"
