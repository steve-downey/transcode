# P3-Step 8: `encoding_rs` Baseline

**Branch:** `p3-step8-encoding-rs-baseline`
**Depends on:** [p3-step7-codecvt-baseline.md](p3-step7-codecvt-baseline.md)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Integrate an `encoding_rs` baseline that can be compared against matched
`beman::transcode` cases without contaminating the core C++ build surface.

## Context for Executing Agent

encoding_rs is a Rust library. Integration options: Rust FFI via cbindgen, or a
separate helper binary that reads stdin and writes stdout. The project's `docs/`
directory has oracle direction for encoding_rs integration. This step should keep
the Rust dependency completely optional and isolated. Corpora: same files from
fixture layer.

## Deliverables

- optional build or helper-binary path for `encoding_rs`
- isolated interface boundary for benchmark invocation
- matched benchmark cases against at least UTF and one legacy WHATWG encoding
- documentation for how to enable and run the baseline locally

## Constraints

- keep the Rust/FFI dependency optional
- isolate the dependency from the public library build
- benchmark the real implementation, not a character-by-character FFI anti-pattern

## Procedure

1. Create branch `p3-step8-encoding-rs-baseline` from `main`
2. Add the optional dependency/build surface
3. Implement a benchmark invocation strategy that works on contiguous inputs
4. Add matched benchmark cases and smoke filters
5. Document enablement and known limitations
6. Run `make test`
7. Run `make lint`
8. Run the `encoding_rs` smoke command
9. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the encoding_rs smoke command on an enabled environment
```

## Notes

This step should align with the repository's existing oracle direction rather
than inventing a new integration story.

## Handoff to Step 9

Step 8 done, next read p3-step9-simdutf-baseline.md.
