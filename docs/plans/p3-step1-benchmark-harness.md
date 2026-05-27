# P3-Step 1: Benchmark Harness Scaffolding

**Branch:** `p3-step1-benchmark-harness`
**Depends on:** Phase 2 stability on `main`
**Read first:** `docs/plans/phase3-handoff.md` and `docs/plans/handoff-next.md`

---

## Goal

Create the dedicated top-level `benchmark/` build surface for the project:
CMake wiring, Catch2 benchmark executable pattern, Makefile target, and
one trivial smoke benchmark proving the harness works.

This step is intentionally about structure, not real benchmark coverage.

## Context for Executing Agent

The project is `beman.transcode`, a C++23 header-only Unicode transcoding
library. It uses CMake + Ninja Multi-Config. Tests use Catch2 3.x (already
a dependency via FetchContent). The Makefile exposes `make compile`,
`make test`, `make lint`.

No `benchmark/` directory exists yet. No `make bench` target exists.

The build uses Ninja Multi-Config, so executables land under a config
subdirectory (e.g., `.build/build-system/benchmark/Asan/`).

### Key files to reference

- `CMakeLists.txt` — top-level; add option + `add_subdirectory` here
- `Makefile` — add `bench` target here
- `examples/CMakeLists.txt` — pattern for adding executables
- `tests/beman/transcode/CMakeLists.txt` — pattern for Catch2 linking

### Build conventions

```cmake
# Examples pattern (follow this for benchmarks):
add_executable(beman.transcode.benchmarks.smoke)
target_sources(beman.transcode.benchmarks.smoke PRIVATE smoke.bench.cpp)
target_link_libraries(
    beman.transcode.benchmarks.smoke
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
```

### Library usage in benchmark code

```cpp
#include <beman/transcode/transcode.hpp>
#include <catch2/catch_all.hpp>

using namespace beman::transcoding;

// Decode bytes to Unicode:
std::span<const char>(input) | whatwg_decode<codec::utf_8>
```

## Deliverables

1. `benchmark/CMakeLists.txt` — registers smoke benchmark executable
2. `benchmark/smoke.bench.cpp` — one `BENCHMARK` case over a tiny UTF-8 kernel
3. Top-level `CMakeLists.txt` — option `BEMAN_TRANSCODE_BUILD_BENCHMARKS` +
   conditional `add_subdirectory(benchmark)`
4. `Makefile` — `make bench` target that runs the smoke executable

## Procedure

1. Create worktree branch `p3-step1-benchmark-harness` from `main`
2. Add `BEMAN_TRANSCODE_BUILD_BENCHMARKS` option to `CMakeLists.txt`
   (after `BEMAN_TRANSCODE_BUILD_EXAMPLES` option, default `${PROJECT_IS_TOP_LEVEL}`)
3. Add `add_subdirectory(benchmark)` guarded by that option (after the
   examples block, around line 115)
4. Create `benchmark/CMakeLists.txt` with smoke executable
5. Create `benchmark/smoke.bench.cpp` with one `BENCHMARK` macro:
   - Create a 4096-byte ASCII string
   - Benchmark iterating `whatwg_decode<codec::utf_8>` over it
   - Return `output.size()` to prevent dead-code elimination
6. Add `make bench` target to `Makefile`
7. Run `make compile` — verify benchmark compiles
8. Run `make test` — verify existing tests still pass
9. Run `make lint` — verify formatting
10. Run `make bench` — verify benchmark executes and prints timing
11. Commit, merge to main (non-ff), push both remotes
12. Update `docs/plans/handoff-next.md`

## Makefile Target Pattern

Look at existing targets in `Makefile` (e.g., `compile`, `ctest_`).
The benchmark target should:
- Depend on `compile` (ensures build is current)
- Execute the smoke binary directly
- Pass `"[smoke]"` tag filter if using Catch2 tags

Determine the actual executable path from the build directory layout.
For Ninja Multi-Config with Asan config, it will be something like:
`.build/build-system/benchmark/Asan/beman.transcode.benchmarks.smoke`

## Verification

```bash
make compile   # benchmark code compiles
make test      # existing 662+ tests still pass
make lint      # clean
make bench     # runs smoke benchmark, prints timing output
```

## Handoff to Step 2

After completing, write to `docs/plans/handoff-next.md`:
- Step 1 is done
- Next: read `p3-step2-benchmark-data.md`
- Files created: `benchmark/CMakeLists.txt`, `benchmark/smoke.bench.cpp`
- Make targets added: `make bench`
- Note the actual executable path discovered during the step
- Note any Makefile pattern decisions made
