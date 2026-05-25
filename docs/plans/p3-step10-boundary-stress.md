# P3-Step 10: Chunked/Lazy Boundary Stress Benchmarks

**Branch:** `p3-step10-boundary-stress`
**Depends on:** [p3-step9-simdutf-baseline.md](p3-step9-simdutf-baseline.md)

---

## Goal

Measure the real lazy-pipeline penalty of chunked text processing and boundary
recovery using deterministic stress cases, including mock-iconv `EINVAL` and
`E2BIG` conditions.

## Deliverables

- chunked benchmark cases for native `beman::transcode`
- deterministic mock-iconv stress kernels
- synthetic corpora chosen to split multi-byte sequences at hostile boundaries
- smoke filters for one `EINVAL` and one `E2BIG` scenario

## Constraints

- this step is about worst-case control-flow stress, not peak throughput
- keep the stress cases deterministic and reproducible
- reuse existing iconv test scaffolding where practical rather than inventing
  a second mocking model

## Procedure

1. Create branch `p3-step10-boundary-stress` from `main`
2. Add chunked input benchmark adapters and hostile chunk sizes
3. Add mock-iconv stress kernels for incomplete sequence and buffer exhaustion
4. Add smoke filters for deterministic reruns
5. Run `make test`
6. Run `make lint`
7. Run the boundary benchmark smoke command
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the boundary benchmark smoke command
```

## Notes

If this step reveals correctness defects rather than performance cliffs, stop
and write a P2 follow-up instead of folding the fix invisibly into P3.
