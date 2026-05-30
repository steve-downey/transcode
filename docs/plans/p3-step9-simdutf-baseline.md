# P3-Step 9: `simdutf` Ceiling Baseline

**Branch:** `p3-step9-simdutf-baseline`
**Depends on:** [p3-step8-encoding-rs-baseline.md](p3-step8-encoding-rs-baseline.md)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Add the `simdutf` performance ceiling baseline so the project can quantify the
gap between composable standard C++ ranges and hardware-specific SIMD code.

## Context for Executing Agent

simdutf is a C++ library available via FetchContent or vcpkg. API:
```cpp
#include <simdutf.h>
simdutf::convert_utf8_to_utf32(data, length, output);
simdutf::validate_utf8(data, length);
```
Guard with CMake option `BEMAN_TRANSCODE_BENCHMARK_SIMDUTF`. Use `#ifdef` in
source. Match workloads against Step 4 UTF cases for fair comparison.

## Deliverables

- optional `simdutf` dependency wiring
- matched UTF benchmark kernels aligned with P3-Step 4 cases
- smoke benchmark filters for at least UTF-8 decode and UTF-8 <-> UTF-16 paths
- clear metadata marking `simdutf` as the ceiling baseline

## Constraints

- keep the dependency optional and off by default
- align workloads with existing UTF benchmark cases for fair comparison
- do not mix `simdutf`-specific data preparation into the timed region

## Procedure

1. Create branch `p3-step9-simdutf-baseline` from `main`
2. Add optional `simdutf` dependency wiring
3. Add matched UTF benchmark cases
4. Add smoke filters and feature gating docs
5. Run `make test`
6. Run `make lint`
7. Run the `simdutf` smoke command
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the simdutf smoke command on an enabled environment
```

## Notes

This step should make the mechanical-sympathy gap measurable, not erase it.

## Handoff to Step 10

Step 9 done, next read p3-step10-boundary-stress.md.
