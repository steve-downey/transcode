# P3-Step 12: Reporting, Dashboard, and Reproducibility Pack

**Branch:** `p3-step12-reporting-and-repro`
**Depends on:** [p3-step11-toolchain-matrix.md](p3-step11-toolchain-matrix.md)

---

## Goal

Convert raw benchmark timings and metadata into proposal-ready comparative
tables, GiB/s summaries, and reproducible reporting documentation.

## Deliverables

- post-processing scripts that translate Catch2 timing to throughput metrics
- comparison tables by implementation, corpus, topology, and toolchain
- chart generation (Vega-Lite or equivalent)
- documentation describing exactly how to regenerate the published outputs

## Files Expected

- `tools/process_benchmark_results.py`
- `tools/tests/test_process_benchmark_results.py`
- `docs/benchmarks/README.md` or equivalent report guide
- generated chart/table directory under `data/benchmarks/results/` or equivalent

## Constraints

- keep raw results separate from derived charts/tables
- document formulas for throughput conversion explicitly
- report both smoke-scale and full-matrix usage clearly

## Procedure

1. Create branch `p3-step12-reporting-and-repro` from `main`
2. Add post-processing tooling and tests
3. Generate tables and charts from a smoke dataset first
4. Write reproducibility/reporting documentation
5. Run `make test`
6. Run `make lint`
7. Run the reporting smoke command
8. Update `handoff-next.md` with future rerun guidance rather than another planned P3 step

## Verification

```bash
make test
make lint
# plus the reporting smoke command on a small benchmark result set
```

## Notes

This step closes the phase by making the benchmark program communicable and
repeatable for future proposal work, regressions, and follow-up analysis.
