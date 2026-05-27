# Handoff: Next Step

## Completed

- **P3-Step 4: UTF-family and ASCII fast-path benchmarks** — done on `worktree-pluggable-codec-protocol` branch

## What was done

- Created `benchmark/utf_benchmarks.bench.cpp` — three benchmark cases:
  - `UTF-8 decode: English (ASCII-heavy)` — measures `corpus_span("en_mars_utf8.txt") | whatwg_decode<codec::utf_8>`
  - `UTF-8 decode: Arabic (multibyte-heavy)` — measures `corpus_span("ar_mars_utf8.txt") | whatwg_decode<codec::utf_8>`
  - `UTF-8 encode round-trip: English` — measures decode then re-encode via `| whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>`
- Updated `benchmark/CMakeLists.txt` — added `beman.transcode.benchmarks.utf` executable
- Updated `Makefile` — added `bench-utf` target running `[benchmark][utf]` tag filter
- Applied clang-format alignment fixes to fixture files that pre-commit caught
- 671 C++ + 189 Python tests pass; mypy + ruff + clang-format + gersemi all clean

## Files created

- `benchmark/utf_benchmarks.bench.cpp`

## Files modified

- `benchmark/CMakeLists.txt` — added utf benchmark executable
- `Makefile` — added `bench-utf` target
- `benchmark/benchmark_fixture.cpp` — clang-format alignment (no logic change)
- `benchmark/benchmark_fixture.hpp` — clang-format alignment (no logic change)
- `tests/beman/transcode/benchmark_fixture.test.cpp` — clang-format alignment (no logic change)

## Next Step

Read `docs/plans/p3-step5-legacy-whatwg-benchmarks.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Benchmark Results (fallback corpus — sub-KB files)

With the checked-in fallback corpus (each file ~600 bytes), timings are tiny
and variable. Run `make bench-utf` to see them. For meaningful throughput
numbers, download the full corpus first:

```bash
uv run python tools/download_benchmark_corpora.py
make bench-utf
```

Observed on fallback corpus (indicative only):
- English decode: ~68 µs (high variance due to small data)
- Arabic decode: ~14 µs (Arabic file is 640 bytes, also small)
- Round-trip encode: ~24 µs

The `whatwg_encode<codec::utf_8>` composes cleanly in pipe with
`whatwg_decode<codec::utf_8>` — the pipe expression compiles and runs
without issues.

## Fixture API (unchanged from step 3)

```cpp
#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>

std::span<const char>          sp     = beman::transcoding::bench::corpus_span("en_mars_utf8.txt");
std::vector<std::string_view>  chunks = beman::transcoding::bench::chunk_corpus(sv, 1024);
std::size_t                    n      = beman::transcoding::bench::count_elements(range);
beman::transcoding::bench::volatile_sink(value);
```

## Include paths and linking

- Link `beman.transcode.benchmark_fixture` to get `<benchmark/benchmark_fixture.hpp>` and `<benchmark/benchmark_sink.hpp>`
- In `benchmark/CMakeLists.txt`: link against `beman::transcode`, `beman.transcode.benchmark_fixture`, `Catch2::Catch2WithMain`

## Current State

- `make test` passes (671 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi all clean)
- `make bench` runs smoke (ASCII corpus, `[smoke]` tag)
- `make bench-utf` runs UTF benchmarks (`[benchmark][utf]` tag)

## Branch Discipline

This step was done on `worktree-pluggable-codec-protocol`. After merging to main,
the next step should follow the same pattern (work in this worktree, rebase
from origin/main first).
