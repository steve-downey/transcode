# P3-Step 6: `iconv` Baselines

**Branch:** `p3-step6-iconv-baselines`
**Depends on:** [p3-step5-legacy-whatwg-benchmarks.md](p3-step5-legacy-whatwg-benchmarks.md)

---

## Goal

Benchmark raw `iconv` and `iconv_transcode_view` on matching workloads so the
project can quantify whether the C++ wrapper adds measurable cost on top of the
system C API.

## Deliverables

- raw `iconv` benchmark kernels using comparable buffers and output handling
- `iconv_transcode_view` benchmark kernels using the same corpora/codecs
- smoke comparisons for at least one UTF path and one legacy encoding path
- clear metadata separating `raw_iconv` from `iconv_view`

## Constraints

- compare like with like: same corpus, same output shape, same chunking mode
- keep setup outside the timed region where possible
- do not yet introduce synthetic `EINVAL`/`E2BIG` stress; that belongs in P3-Step 10

## Procedure

1. Create branch `p3-step6-iconv-baselines` from `main`
2. Add raw `iconv` kernels for matched cases
3. Add `iconv_transcode_view` kernels for the same cases
4. Add smoke filters for direct comparison
5. Run `make test`
6. Run `make lint`
7. Run the `iconv` benchmark smoke command
8. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the iconv benchmark smoke command
```

## Notes

If system `iconv` behavior is platform-variant, document the variant and keep
the benchmark surface honest rather than papering over it.
