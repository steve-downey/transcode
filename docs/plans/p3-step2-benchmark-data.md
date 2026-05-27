# P3-Step 2: Benchmark Corpus Acquisition and Provenance

**Branch:** `p3-step2-benchmark-data`
**Depends on:** [p3-step1-benchmark-harness.md](p3-step1-benchmark-harness.md)
**Read first:** `docs/plans/phase3-handoff.md` and `docs/plans/handoff-next.md`

---

## Goal

Acquire and document the benchmark corpora called for in the benchmarking
report, with the same provenance discipline used for WHATWG and WPT inputs.

## Context for Executing Agent

The project has a `benchmark/` directory with a smoke benchmark (from Step 1).
Python tooling uses `uv run` and lives in `tools/`. Python tests are in
`tools/tests/` and run via `make pytest`. The `data/` directory contains
generated codec lookup tables under `data/tables/`.

The benchmarking methodology requires Wikipedia "Mars" article extracts
as real-world corpora. Each language targets a specific encoding stress path:

| Language | Encoding Stress | File to produce |
|----------|----------------|-----------------|
| English | ASCII-heavy UTF-8 | `en_mars_utf8.txt` |
| Arabic | Mixed 1/2-byte UTF-8 | `ar_mars_utf8.txt` |
| Russian | Cyrillic / windows-1251 | `ru_mars_utf8.txt`, `ru_mars_windows1251.bin` |
| Japanese | CJK / Shift-JIS | `ja_mars_utf8.txt`, `ja_mars_shiftjis.bin` |

### MediaWiki API for extraction

```
https://en.wikipedia.org/w/api.php?action=query&titles=Mars&prop=extracts&explaintext=true&format=json
```
Replace `en.wikipedia.org` with `ar.`, `ru.`, `ja.` for other languages.

### Python encoding conversion

```python
text.encode('windows-1251', errors='replace')  # UTF-8 -> windows-1251 bytes
text.encode('shift_jis', errors='replace')      # UTF-8 -> Shift-JIS bytes
```

## Deliverables

1. `tools/download_benchmark_corpora.py` — download + convert + manifest
2. `tools/tests/test_download_benchmark_corpora.py` — unit tests
3. `docs/benchmarks/SOURCE.md` — provenance documentation
4. `data/benchmarks/corpus_manifest.json` — generated manifest (gitignored)
5. `benchmark/corpus/` — small checked-in fallback files (~100-200 bytes each)
6. Update `.gitignore` to exclude `data/benchmarks/`

## Corpus Manifest Schema

```json
[
  {
    "file": "en_mars_utf8.txt",
    "lang": "en",
    "encoding": "utf-8",
    "bytes": 123456,
    "sha256": "abc123..."
  }
]
```

## Fallback Corpus (checked in)

Create `benchmark/corpus/` with small representative samples:
- `en_mars_utf8.txt` — "Mars is the fourth planet from the Sun..."
- `ar_mars_utf8.txt` — Arabic equivalent (~100 bytes)
- `ru_mars_utf8.txt` — Russian equivalent
- `ja_mars_utf8.txt` — Japanese equivalent

These enable `make bench` to work without network access.

## Procedure

1. Create worktree branch `p3-step2-benchmark-data` from `main`
2. Create `tools/download_benchmark_corpora.py`
3. Create fallback corpus files in `benchmark/corpus/`
4. Create `docs/benchmarks/SOURCE.md`
5. Add `data/benchmarks/` to `.gitignore`
6. Create `tools/tests/test_download_benchmark_corpora.py`
7. Run `make pytest` — Python tests pass
8. Run `make test` — existing tests pass
9. Run `make lint`
10. Optionally run the download tool to verify it works
11. Commit, merge to main (non-ff), push both remotes
12. Update `docs/plans/handoff-next.md`

## Verification

```bash
make test
make lint
make pytest
# Optionally: uv run python tools/download_benchmark_corpora.py
```

## Handoff to Step 3

Write to `docs/plans/handoff-next.md`:
- Step 2 is done
- Next: read `p3-step3-benchmark-fixtures.md`
- Fallback corpus files: `benchmark/corpus/*.txt`
- Generated corpus: `data/benchmarks/` (run download tool to populate)
- Manifest: `data/benchmarks/corpus_manifest.json`
