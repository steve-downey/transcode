# Handoff: Next Step

## Completed

- **P3-Step 3: Benchmark Fixture Library** — done on `worktree-pluggable-codec-protocol` branch

## What was done

- Created `benchmark/benchmark_fixture.hpp` — `load_corpus()`, `corpus_span()`,
  `chunk_corpus()` API; corpus loaded from `BENCHMARK_DATA_DIR` or fallback
  `BENCHMARK_CORPUS_DIR` (both set as CMake compile-time defines)
- Created `benchmark/benchmark_fixture.cpp` — implementation with static cache
  (mutex-protected `std::map<string, string>`)
- Created `benchmark/benchmark_sink.hpp` — `volatile_sink<T>()` (DCE barrier)
  and `constexpr count_elements()` (range counter that returns a value)
- Created `tests/beman/transcode/benchmark_fixture.test.cpp` — 9 unit tests
  covering chunk_corpus, load_corpus (fallback corpus), caching, corpus_span,
  count_elements (including consteval test with constify()), volatile_sink
- Updated `benchmark/CMakeLists.txt` — `beman.transcode.benchmark_fixture`
  static library with `PUBLIC "${PROJECT_SOURCE_DIR}"` include dir
- Updated `tests/beman/transcode/CMakeLists.txt` — registered fixture test
  with `catch_discover_tests`
- Updated `benchmark/smoke.bench.cpp` — uses `BENCHMARK_ADVANCED` with
  `corpus_span("en_mars_utf8.txt")` and `count_elements(...)`
- Also fixed pre-existing ruff lint issues from p3-step2 (E501, F841,
  unused imports, trailing whitespace in docs/benchmarks/SOURCE.md)
- 671 C++ + 189 Python tests pass; mypy + ruff + clang-format all clean

## Files created

- `benchmark/benchmark_fixture.hpp`
- `benchmark/benchmark_fixture.cpp`
- `benchmark/benchmark_sink.hpp`
- `tests/beman/transcode/benchmark_fixture.test.cpp`

## Files modified

- `benchmark/CMakeLists.txt` — added fixture static library
- `benchmark/smoke.bench.cpp` — uses fixtures
- `tests/beman/transcode/CMakeLists.txt` — registered fixture test
- `tools/download_benchmark_corpora.py` — fixed ruff lint (E501, F841)
- `tools/tests/test_download_benchmark_corpora.py` — ruff removed unused imports
- `docs/benchmarks/SOURCE.md` — pre-commit removed trailing whitespace

## Next Step

Read `docs/plans/p3-step4-utf-benchmarks.md`

Also read `docs/plans/phase3-handoff.md` for project conventions.

## Fixture API (for the next step to use)

```cpp
#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>

// Load corpus (cached, falls back to benchmark/corpus/ if data/benchmarks/ absent)
std::string_view sv  = beman::transcoding::bench::load_corpus("en_mars_utf8.txt");
std::span<const char> sp = beman::transcoding::bench::corpus_span("en_mars_utf8.txt");

// Chunk into fixed-size pieces
auto chunks = beman::transcoding::bench::chunk_corpus(sv, 1024);

// Count elements (prevents DCE, constexpr)
std::size_t n = beman::transcoding::bench::count_elements(range);

// Volatile sink (prevents DCE for non-returning computations)
beman::transcoding::bench::volatile_sink(value);
```

## Include paths and linking

- Headers reachable via `<benchmark/benchmark_fixture.hpp>` and
  `<benchmark/benchmark_sink.hpp>` (project root is the include dir)
- Any target that links `beman.transcode.benchmark_fixture` automatically
  gets `${PROJECT_SOURCE_DIR}` in its include path (PUBLIC property)
- In `benchmark/CMakeLists.txt`: link against `beman.transcode.benchmark_fixture`
- In `tests/.../CMakeLists.txt`: link against `beman.transcode.benchmark_fixture`
  (the `beman.transcode.benchmark_fixture` target is defined in `benchmark/`)

## Current State

- `make test` passes (671 C++ + 189 Python tests)
- `make lint` passes (mypy, ruff, clang-format, gersemi all clean)
- `make bench` works: smoke benchmark uses real corpus data

## Branch Discipline

This step was done on `worktree-pluggable-codec-protocol`. After merging to main,
the next step should follow the same pattern (work in this worktree, rebase
from origin/main first).
