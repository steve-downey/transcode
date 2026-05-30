# P3-Step 3: Benchmark Fixture Library and Result Schema

**Branch:** `p3-step3-benchmark-fixtures`
**Depends on:** [p3-step2-benchmark-data.md](p3-step2-benchmark-data.md)
**Read first:** `docs/plans/phase3-handoff.md` and `docs/plans/handoff-next.md`

---

## Goal

Build the reusable support layer that later benchmarks share: corpus loading,
chunking helpers, sink utilities that prevent dead-code elimination, and
metadata capture. Unit-test all reusable helpers.

## Context for Executing Agent

The project has `benchmark/smoke.bench.cpp` and corpus data:
- Checked-in fallback: `benchmark/corpus/*.txt` (small, ~100 bytes each)
- Generated (optional): `data/benchmarks/*.txt` and `*.bin` (larger)

Catch2 3.x `BENCHMARK_ADVANCED` separates setup from measurement:
```cpp
BENCHMARK_ADVANCED("name")(Catch::Benchmark::Chronometer meter) {
    auto data = load_data();  // not timed
    meter.measure([&] {
        return process(data);  // timed, return prevents DCE
    });
};
```

Project convention: reusable helpers get unit tests in `tests/beman/transcode/`
and are registered in `tests/beman/transcode/CMakeLists.txt`.

## Deliverables

1. `benchmark/benchmark_fixture.hpp` — corpus loading and chunking
2. `benchmark/benchmark_sink.hpp` — anti-DCE sink utilities
3. `tests/beman/transcode/benchmark_fixture.test.cpp` — unit tests
4. Updated `benchmark/smoke.bench.cpp` — uses fixtures
5. Registration in `tests/beman/transcode/CMakeLists.txt`

## API Design

### benchmark_fixture.hpp

```cpp
#ifndef BENCHMARK_BENCHMARK_FIXTURE_HPP
#define BENCHMARK_BENCHMARK_FIXTURE_HPP

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace beman::transcoding::bench {

// Load a corpus file. Tries data/benchmarks/<name> first, falls back to
// benchmark/corpus/<name>. Caches in static storage (load once per process).
std::string_view load_corpus(const char* filename);

// Same as load_corpus but returns span<const char> for direct piping.
std::span<const char> corpus_span(const char* filename);

// Split data into chunks. Last chunk may be smaller.
std::vector<std::string_view> chunk_corpus(std::string_view data, std::size_t chunk_size);

} // namespace beman::transcoding::bench

#endif
```

### benchmark_sink.hpp

```cpp
#ifndef BENCHMARK_BENCHMARK_SINK_HPP
#define BENCHMARK_BENCHMARK_SINK_HPP

#include <cstddef>

namespace beman::transcoding::bench {

// Store a value where the compiler cannot prove it's unused.
template <typename T>
void volatile_sink(const T& value) {
    static volatile const T* p;
    p = &value;
}

// Count elements without storing them. Returns the count.
template <typename Range>
std::size_t count_elements(Range&& r) {
    std::size_t n = 0;
    for (auto&& x : r)
        ++n;
    return n;
}

} // namespace beman::transcoding::bench

#endif
```

## Procedure

1. Create worktree branch `p3-step3-benchmark-fixtures` from `main`
2. Create `benchmark/benchmark_fixture.hpp` and `.cpp` (if file I/O needed)
3. Create `benchmark/benchmark_sink.hpp`
4. Create `tests/beman/transcode/benchmark_fixture.test.cpp`
5. Register test in `tests/beman/transcode/CMakeLists.txt`
6. Update `benchmark/smoke.bench.cpp` to use `corpus_span()` and `count_elements()`
7. If the fixture has a `.cpp` file, update `benchmark/CMakeLists.txt`
   to link it (or make it a small static library)
8. Run `make test` — fixture tests pass
9. Run `make lint`
10. Run `make bench` — smoke uses real corpus
11. Commit, merge to main (non-ff), push both remotes
12. Update `docs/plans/handoff-next.md`

## Verification

```bash
make test      # fixture unit tests pass (among all others)
make lint
make bench     # smoke benchmark uses fixtures
```

## Handoff to Step 4

Write to `docs/plans/handoff-next.md`:
- Step 3 is done
- Next: read `p3-step4-utf-benchmarks.md`
- Fixture API: `corpus_span("en_mars_utf8.txt")`, `count_elements(range)`
- Include paths: `"benchmark_fixture.hpp"`, `"benchmark_sink.hpp"` (from benchmark/)
- The `benchmark/CMakeLists.txt` may need include_directories for benchmark/
