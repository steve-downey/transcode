# P3-Step 4: UTF-Family and ASCII Fast-Path Benchmarks

**Branch:** `p3-step4-utf-benchmarks`
**Depends on:** [p3-step3-benchmark-fixtures.md](p3-step3-benchmark-fixtures.md)
**Read first:** `docs/plans/phase3-handoff.md` and `docs/plans/handoff-next.md`

---

## Goal

Measure the algorithmic UTF paths in `beman::transcode`, focusing on the
highest-value core conversions and the ASCII-dominant best-case path.

## Context for Executing Agent

Benchmark fixtures are available (from Step 3):
```cpp
#include "benchmark_fixture.hpp"
#include "benchmark_sink.hpp"
using namespace beman::transcoding::bench;

auto data = corpus_span("en_mars_utf8.txt");
std::size_t n = count_elements(data | whatwg_decode<codec::utf_8>);
```

Library API for UTF paths:
```cpp
// Decode:
bytes | whatwg_decode<codec::utf_8>      // -> char32_t
bytes | whatwg_decode<codec::utf_16le>   // -> char32_t
bytes | whatwg_decode<codec::utf_16be>   // -> char32_t

// Encode:
char32_t_range | whatwg_encode<codec::utf_8>   // -> char
```

Corpora (fallback checked-in, or full from download tool):
- `en_mars_utf8.txt` — English, ASCII-heavy
- `ar_mars_utf8.txt` — Arabic, mixed 1/2-byte UTF-8 sequences

## Deliverables

1. `benchmark/utf_benchmarks.bench.cpp` — UTF-8 decode and encode cases
2. Registration in `benchmark/CMakeLists.txt`
3. Updated `make bench` or tag-based execution

## Benchmark Cases

```cpp
TEST_CASE("UTF benchmarks", "[benchmark][utf]") {
    BENCHMARK_ADVANCED("UTF-8 decode: English (ASCII-heavy)")
        (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    BENCHMARK_ADVANCED("UTF-8 decode: Arabic (multibyte-heavy)")
        (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ar_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    BENCHMARK_ADVANCED("UTF-8 encode round-trip: English")
        (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        // Decode then re-encode
        meter.measure([&] {
            std::size_t n = 0;
            for (char c : data | whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>)
                ++n;
            return n;
        });
    };
}
```

## Procedure

1. Create worktree branch `p3-step4-utf-benchmarks` from `main`
2. Create `benchmark/utf_benchmarks.bench.cpp` with the cases above
3. Add executable to `benchmark/CMakeLists.txt`
4. Run `make compile`
5. Run `make test`
6. Run `make lint`
7. Run `make bench` or the utf executable directly
8. Commit, merge to main (non-ff), push both remotes
9. Update `docs/plans/handoff-next.md`

## Verification

```bash
make compile
make test
make lint
make bench     # shows UTF timing numbers
```

## Handoff to Step 5

Write to `docs/plans/handoff-next.md`:
- Step 4 is done
- Next: read `p3-step5-legacy-whatwg-benchmarks.md`
- New file: `benchmark/utf_benchmarks.bench.cpp`
- Benchmark results show ASCII-heavy vs multibyte-heavy decode delta
- Note whether `whatwg_encode` compose with `whatwg_decode` smoothly in pipe
