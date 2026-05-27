# P3-Step 10: Chunked/Lazy Boundary Stress Benchmarks

**Branch:** `p3-step10-boundary-stress`
**Depends on:** [p3-step9-simdutf-baseline.md](p3-step9-simdutf-baseline.md)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Measure the real lazy-pipeline penalty of chunked text processing and boundary
recovery using deterministic stress cases, including mock-iconv `EINVAL` and
`E2BIG` conditions.

## Context for Executing Agent

The iconv mock library at `tests/beman/transcode/iconv_mock.hpp` provides
deterministic mocks. Key mocks for benchmarking:
- `mock_iconv_e2big` -- returns E2BIG after writing 1 byte
- `mock_iconv_eilseq` -- always returns EILSEQ
- `mock_iconv_partial_consume` -- partial progress then EINVAL

The fixture library's `chunk_corpus(data, chunk_size)` splits data for chunked
benchmarks. The iconv views accept mock function structs via template parameter.

Also benchmark the native whatwg_decode views with chunked input (iterating each
chunk separately to measure restart cost).

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

## Handoff to Step 11

Step 10 done, next read p3-step11-toolchain-matrix.md.
