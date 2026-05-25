# P3-Step 7: `std::codecvt` Negative Baseline

**Branch:** `p3-step7-codecvt-baseline`
**Depends on:** [p3-step6-iconv-baselines.md](p3-step6-iconv-baselines.md)

---

## Goal

Add the deprecated `std::codecvt` comparison path as a negative baseline while
keeping unsupported or broken library environments green.

## Deliverables

- conditional benchmark support for `std::codecvt`
- explicit skip behavior where the implementation is absent or unusable
- a short benchmark README note documenting known limitations by platform or STL

## Constraints

- portability matters more than broad `codecvt` coverage
- if the environment cannot support a meaningful benchmark, the step should
  produce a documented skip rather than fragile emulation
- avoid treating `codecvt` failures as project regressions

## Procedure

1. Create branch `p3-step7-codecvt-baseline` from `main`
2. Add conditional compilation/build logic for the `codecvt` baseline
3. Add one or two benchmark cases where the implementation is sane
4. Document the skip path and feature gating
5. Run `make test`
6. Run `make lint`
7. Run the `codecvt` smoke command or verify the documented skip behavior
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the codecvt smoke command, or an explicit documented skip on this platform
```

## Notes

The benchmark report treats `codecvt` as a historical negative baseline, not a
first-class portability requirement.
