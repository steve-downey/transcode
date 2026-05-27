# P3-Step 6: `iconv` Baselines

**Branch:** `p3-step6-iconv-baselines`
**Depends on:** [p3-step5-legacy-whatwg-benchmarks.md](p3-step5-legacy-whatwg-benchmarks.md)
**Read first:** `docs/plans/phase3-handoff.md` and `docs/plans/handoff-next.md`

---

## Goal

Benchmark raw `iconv` and `iconv_transcode_view` on matching workloads so the
project can quantify whether the C++ wrapper adds measurable cost on top of the
system C API.

## Context for Executing Agent

The library provides two iconv-related views:
- `iconv_transcode_view<IconvFns, R>` — yields `char`
- `iconv_transcode_or_error_view<IconvFns, R>` — yields `expected<char, iconv_error>`

Real iconv function pointers:
```cpp
#include <beman/transcode/iconv_real.hpp>
// beman::transcoding::iconv_real_functions has .open, .convert, .close
```

View usage:
```cpp
char buffer[4096];
iconv_transcode_view view(range, iconv_real_functions{}, "UTF-8", "UTF-32LE",
                          std::span<char>(buffer));
for (char c : view) { /* ... */ }
```

For raw iconv comparison, write a direct C-style loop calling `iconv()`.

Corpora: use `corpus_span("en_mars_utf8.txt")` etc. from `benchmark_fixture.hpp`.
Link `Iconv::Iconv` (already found by top-level CMake via `find_package(Iconv REQUIRED)`).

## Deliverables

1. `benchmark/iconv_benchmarks.bench.cpp`
2. Registration in `benchmark/CMakeLists.txt` with `Iconv::Iconv` link

## Benchmark Cases

- "Raw iconv: UTF-8 to UTF-32 English"
- "iconv_transcode_view: UTF-8 to UTF-32 English"
- "Raw iconv: Shift-JIS to UTF-8 Japanese"
- "iconv_transcode_view: Shift-JIS to UTF-8 Japanese"

## Constraints

- Compare like with like: same corpus, same output shape
- Keep handle open/close and buffer allocation outside timed region
- Do NOT introduce synthetic EINVAL/E2BIG stress; that belongs in Step 10

## Procedure

1. Create worktree branch `p3-step6-iconv-baselines` from `main`
2. Create `benchmark/iconv_benchmarks.bench.cpp`
3. Register in `benchmark/CMakeLists.txt`, link `Iconv::Iconv`
4. Run `make compile`, `make test`, `make lint`
5. Run `make bench`
6. Commit, merge to main (non-ff), push both remotes
7. Update `docs/plans/handoff-next.md`

## Verification

```bash
make compile
make test
make lint
make bench
```

## Handoff to Step 7

Write to `docs/plans/handoff-next.md`:
- Step 6 is done
- Next: read `p3-step7-codecvt-baseline.md`
- Note whether iconv_transcode_view matches raw iconv performance
- Note any platform-specific iconv quirks observed
