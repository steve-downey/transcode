# P3-Step 5: Legacy WHATWG Codec Benchmarks

**Branch:** `p3-step5-legacy-whatwg-benchmarks`
**Depends on:** [p3-step4-utf-benchmarks.md](p3-step4-utf-benchmarks.md)

---

## Goal

Extend native `beman::transcode` measurement beyond UTF into representative
single-byte and multi-byte WHATWG codecs so the benchmark suite covers the
architectural extremes described in the report.

## Representative Coverage

- single-byte table path: `windows-1251`, `iso-8859-2`, or equivalent
- mixed branch UTF text like Arabic for predictor pressure
- CJK/stateful path: `shift_jis`, `gb18030`, `iso_2022_jp`, or equivalent

## Deliverables

- benchmark cases for representative single-byte codecs
- benchmark cases for representative multi-byte codecs
- corpus-to-codec mapping encoded in case metadata
- smoke subset for one single-byte and one multi-byte path

## Constraints

- benchmark representative codecs, not every codec in the standard yet
- keep the suite small enough that smoke runs remain practical
- record whether each case is expected best-case or pathological

## Procedure

1. Create branch `p3-step5-legacy-whatwg-benchmarks` from `main`
2. Add single-byte representative kernels
3. Add multi-byte/stateful representative kernels
4. Tag the cases with category metadata
5. Run `make test`
6. Run `make lint`
7. Run the legacy benchmark smoke command
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the legacy benchmark smoke command for representative codecs
```

## Notes

The goal here is not exhaustive per-codec reporting. It is to establish the
native performance envelope across the key algorithm families.
