# P3-Step 11: Toolchain Matrix Automation

**Branch:** `p3-step11-toolchain-matrix`
**Depends on:** [p3-step10-boundary-stress.md](p3-step10-boundary-stress.md)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Automate the compiler and optimization matrix from the benchmarking report so
full runs can be reproduced without manual shell choreography and are
accessible from the Makefile workflow.

## Context for Executing Agent

The project uses CMakePresets.json for build configurations. Current presets
include `gcc-debug`, `gcc-release`, `llvm-debug`, `llvm-release`. The toolchain
files are in `infra/cmake/gnu-toolchain.cmake` and
`infra/cmake/llvm-toolchain.cmake`.

Add a `gcc-release-lto` preset inheriting from `gcc-release` with `-flto` added.
Create `docs/benchmarks/RUNNING.md` with instructions. Add `make bench-lto`
target.

## Matrix To Support

- GCC latest available: `-O3`
- GCC latest available: `-O3 -flto`
- Clang latest available: `-O3 -flto`

## Deliverables

- scripts, presets, and Makefile targets for matrix execution
- captured environment metadata: compiler version, CPU model, OS, build flags,
  dependency toggles
- documentation describing smoke versus full-matrix usage

## Constraints

- do not require every machine to support every compiler
- unsupported slices should skip cleanly and visibly
- matrix execution should write machine-readable metadata beside results
- the common entry points for smoke and full matrix runs should be discoverable
    as `make` targets

## Procedure

1. Create branch `p3-step11-toolchain-matrix` from `main`
2. Add matrix runner scripts or presets
3. Add Makefile targets that front the supported matrix slices
4. Capture environment metadata alongside benchmark results
5. Document smoke and full-matrix `make` targets
6. Run `make test`
7. Run `make lint`
8. Run at least one automated matrix smoke slice via the Makefile workflow
9. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus one automated matrix smoke run via a documented Makefile target
```

## Notes

This step turns benchmarking from an experiment into a repeatable process.

## Handoff to Step 12

Step 11 done, next read p3-step12-reporting-and-repro.md.
