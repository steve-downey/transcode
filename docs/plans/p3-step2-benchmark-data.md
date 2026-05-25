# P3-Step 2: Benchmark Corpus Acquisition and Provenance

**Branch:** `p3-step2-benchmark-data`
**Depends on:** [p3-step1-benchmark-harness.md](p3-step1-benchmark-harness.md)

---

## Goal

Acquire and document the benchmark corpora called for in the benchmarking
report, with the same provenance discipline already used for WHATWG and WPT
inputs.

## Data To Cover

- Wikipedia Mars extracts for English/French, Arabic, Czech/Russian, and
  Japanese/Chinese targets
- WPT-derived synthetic vectors relevant to fallback, BOM, and error behavior
- small fuzz/error corpora for malformed input paths

## Deliverables

- typed tooling to fetch or materialize benchmark corpora
- pristine upstream/source artifacts under `docs/`
- provenance docs (`SOURCE.md` and `source.bib`)
- derived corpus manifest under `data/` with size, language, encoding, and
  benchmark tags
- Python tests for the tooling

## Files Expected

- `tools/download_benchmark_corpora.py`
- `tools/tests/test_download_benchmark_corpora.py`
- `docs/benchmarks/` or equivalent pristine source directory
- `docs/benchmarks/SOURCE.md`
- `docs/benchmarks/source.bib`
- `data/benchmarks/corpus_manifest.json`

## Constraints

- Keep raw upstream/source files distinct from derived benchmark fixtures
- Record exact retrieval date and checksums
- Avoid overfitting to synthetic ASCII-only data

## Procedure

1. Create branch `p3-step2-benchmark-data` from `main`
2. Add the download/materialization tool with full typing
3. Materialize the first corpus set plus provenance docs
4. Generate the manifest consumed by later benchmark fixtures
5. Add Python tests for parsing, checksums, and manifest generation
6. Run `make test`
7. Run `make lint`
8. Run the tooling smoke command and confirm the manifest is reproducible
9. Update `handoff-next.md`

## Verification

```bash
make test
make lint
# plus the corpus-tool smoke command introduced in this step
```

## Notes

Do not add benchmark kernels here. The outcome should be trustworthy benchmark
inputs, not measurement logic.
