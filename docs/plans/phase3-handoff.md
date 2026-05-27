# Phase 3 General Handoff — Read This First

This document provides the standing context every Phase 3 agent needs.
Read this, then read the specific `p3-step<N>` document, then read
`handoff-next.md` for step-specific notes from the previous agent.

---

## Project Identity

- **Library:** `beman.transcode` — C++23 header-only Unicode transcoding via ranges/views
- **Repo root:** the current working directory (use `pwd` to confirm)
- **Language:** C++23, minimum GCC 13, minimum Clang 19
- **Build system:** CMake + Ninja Multi-Config, managed via `uv` (Python venv)
- **Test framework:** Catch2 3.x (includes benchmark macros)
- **Python tooling:** `uv run` prefix for all Python commands

## Directory Layout

```
include/beman/transcode/       — public headers (the library)
include/beman/transcode/detail/ — implementation detail headers
tests/beman/transcode/          — Catch2 test sources
examples/                       — example programs
benchmark/                      — benchmark sources (created in P3-Step 1)
tools/                          — Python code-gen and tooling scripts
tools/tests/                    — pytest tests for Python tools
data/                           — generated data (tables, corpora)
docs/plans/                     — step plans (you are reading one)
```

## Key Make Targets

| Target | What it does |
|--------|-------------|
| `make compile` | Build all (Asan config by default) |
| `make test` | Build + run CTest + pytest |
| `make ctest` | Run CTest only (no rebuild) |
| `make pytest` | Run Python tests only |
| `make lint` | Pre-commit hooks: clang-format, gersemi, ruff, codespell |
| `make coverage` | Build Gcov config, run tests, report coverage |
| `make bench` | Run benchmark smoke (added in P3-Step 1) |

## Coding Rules (Enforced)

- Include guards (`#ifndef`/`#define`/`#endif`), never `#pragma once`
- Angle-bracket includes only (`<beman/transcode/...>`)
- Functions defined out-of-line in headers (body after class)
- No `using namespace` in headers
- No Co-Author trailer in commits
- Every constraint/static_assert needs a negative compile test
- `constexpr` everything that can be; test with `constify()` from test_utilities.hpp

## Library API Surface (Current)

### WhatWG codec views (enum-based, closed set)
```cpp
// Decode bytes to char32_t:
bytes | whatwg_decode<codec::utf_8>           // -> char32_t range
bytes | whatwg_decode_or_error<codec::utf_8>  // -> expected<char32_t, whatwg_error>

// Encode char32_t to bytes:
codepoints | whatwg_encode<codec::utf_8>      // -> char range

// All views support .base() on view and iterator
```

### Pluggable codec protocol (type-based, open for extension)
```cpp
// Concepts: decode_codec, encode_codec, random_access_decode_codec_type, flushable_decode_codec
// Templates: table_codec<std::array<char32_t,128>>, full_table_codec<std::array<char32_t,256>>
// Views:
bytes | decode(my_codec{})           // -> char32_t range
bytes | decode_or_error(my_codec{})  // -> expected<char32_t, decode_error>
```

### iconv views
```cpp
iconv_transcode_view<IconvFns, R>(range, fns, from, to, buffer)
iconv_transcode_or_error_view<IconvFns, R>(range, fns, from, to, buffer)
```

### Bulk operations
```cpp
decode_to<codec::utf_8>(range)       // -> vector<char32_t>
encode_to<codec::utf_8>(range)       // -> string
decode_into<codec::utf_8>(range, output_iter)
encode_into<codec::utf_8>(range, output_iter)
```

### Runtime dispatch
```cpp
transcode_string(src_span, codec::from, codec::to)  // -> string
get_encoding("utf-8")                                // -> optional<codec>
```

## Catch2 Benchmark Macros

```cpp
#include <catch2/catch_all.hpp>

// Simple:
BENCHMARK("name") { return do_work(); };

// Advanced (separate setup from measurement):
BENCHMARK_ADVANCED("name")(Catch::Benchmark::Chronometer meter) {
    // setup here (not timed)
    meter.measure([&] {
        // timed region
        return result;  // prevents DCE
    });
};
```

## iconv Mock Library

Located at `tests/beman/transcode/iconv_mock.hpp`. Provides:
- `mock_iconv` (identity), `mock_iconv_e2big`, `mock_iconv_eilseq`,
  `mock_iconv_partial_consume`, `mock_iconv_stateful`, etc.
- Injected via template parameter on iconv views.

## Branch and Commit Discipline

- Branch from `main` for each step: `p3-step<N>-<slug>`
- Run `make test` + `make lint` before committing
- Merge to main with `--no-ff`
- Push to both remotes: `git push origin main && git push bbgithub main`
- No Co-Author trailer

## What to Write in handoff-next.md

After completing your step, overwrite `docs/plans/handoff-next.md` with:
1. Which step you just completed
2. The next step file to read
3. Any files you created that the next step depends on
4. Any surprises, deviations, or decisions not in the step doc
5. The exact `make` targets that are now expected to work
