# P3-Step 11: Toolchain Matrix Automation

**Branch:** `p3-step11-toolchain-matrix`
**Depends on:** [p3-step10-boundary-stress.md](p3-step10-boundary-stress.md)

---

## Goal

Automate the compiler and optimization matrix from the benchmarking report so
full runs can be reproduced without manual shell choreography and are
accessible from the Makefile workflow.

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
