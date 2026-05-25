# P3-Step 4: UTF-Family and ASCII Fast-Path Benchmarks

**Branch:** `p3-step4-utf-benchmarks`
**Depends on:** [p3-step3-benchmark-fixtures.md](p3-step3-benchmark-fixtures.md)

---

## Goal

Measure the algorithmic UTF paths in `beman::transcode`, starting with the
highest-value core conversions and the ASCII-dominant best-case path discussed
in the benchmarking report.

## Cases To Cover

- UTF-8 decode on mostly ASCII corpora
- UTF-8 encode from scalar input where applicable
- UTF-8 <-> UTF-16LE
- UTF-8 <-> UTF-16BE
- eager/monolithic path first

## Deliverables

- benchmark cases for the UTF-family conversion paths
- corpus labels separating ASCII-heavy and multibyte-heavy inputs
- smoke filters that run quickly during step work
- metadata that clearly identifies the fast-path cases

## Constraints

- keep chunked/lazy stress out of this step
- use preallocated sinks where possible
- avoid adding external baselines yet; this step establishes the native numbers

## Procedure

1. Create branch `p3-step4-utf-benchmarks` from `main`
2. Add eager UTF benchmark kernels using the fixture library
3. Add ASCII-heavy and multibyte-heavy corpus mappings
4. Define smoke filters for a small UTF subset
5. Run `make test`
6. Run `make lint`
7. Run the UTF benchmark smoke command
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the UTF benchmark smoke command for the newly added cases
```

## Notes

This step should make it easy to answer the report's first practical question:
how close the scalar C++ pipeline gets to memory-bandwidth-limited behavior on
favorable UTF inputs.
