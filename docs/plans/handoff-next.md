# Handoff: Next Step

## Completed

- **P3-Step 2: Benchmark corpus acquisition and provenance** — done on `worktree-pluggable-codec-protocol` branch

## What was done

- Created `tools/download_benchmark_corpora.py` — downloads Mars Wikipedia article
  in en/ar/ru/ja via MediaWiki API, converts ru→windows-1251 and ja→shift_jis,
  writes `data/benchmarks/corpus_manifest.json` and `data/benchmarks/SOURCE.md`
- Created `benchmark/corpus/` with four small (~500 byte) checked-in fallback files:
  `en_mars_utf8.txt`, `ar_mars_utf8.txt`, `ru_mars_utf8.txt`, `ja_mars_utf8.txt`
- Created `docs/benchmarks/SOURCE.md` — provenance documentation (CC BY-SA 4.0
  license, attribution, encoding conversion notes, manifest description)
- Added `data/benchmarks/` to `.gitignore` (generated large files, not committed)
- Created `tools/tests/test_download_benchmark_corpora.py` — 18 Python tests
  for pure helper functions (no network calls)
- 662 C++ + 189 Python tests pass; mypy + ruff + clang-format all clean

## Files created

- `tools/download_benchmark_corpora.py`
- `tools/tests/test_download_benchmark_corpora.py`
- `benchmark/corpus/en_mars_utf8.txt` (checked in, ~500 bytes)
- `benchmark/corpus/ar_mars_utf8.txt` (checked in, ~360 bytes)
- `benchmark/corpus/ru_mars_utf8.txt` (checked in, ~490 bytes)
- `benchmark/corpus/ja_mars_utf8.txt` (checked in, ~380 bytes)
- `docs/benchmarks/SOURCE.md`

## Files modified

- `.gitignore` — added `data/benchmarks/` exclusion

## Next Step

Read `docs/plans/p3-step3-benchmark-fixtures.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Current State

- `make test` passes (662 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi all clean)
- `make bench` works: smoke benchmark runs fine

## Corpus Details

- **Fallback corpus** (checked in): `benchmark/corpus/*.txt` — small representative
  samples that enable `make bench` without network access
- **Full corpus** (gitignored): `data/benchmarks/` — populated by:
  ```bash
  uv run python tools/download_benchmark_corpora.py
  ```
- **Manifest**: `data/benchmarks/corpus_manifest.json` — JSON with file, lang,
  encoding, bytes, sha256 for each corpus file
- **Encoding conversions**: `data/benchmarks/ru_mars_windows1251.bin` and
  `data/benchmarks/ja_mars_shiftjis.bin` are derived from the UTF-8 sources

## Branch Discipline

This step was done on `worktree-pluggable-codec-protocol`. After merging to main,
the next step should follow the same pattern (work in this worktree, rebase
from origin/main first).
