# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> [!IMPORTANT]
> There is overlapping work in progress.
> Start every task on its own branch and separate git worktree rooted from `main`.
> Before starting work, fetch and integrate any new commits from `main` into that worktree's branch.
> Do not work in a shared or already-dirty worktree for a new task.

## Project

`beman.transcode` is a C++23 header-only library for Unicode transcoding using ranges and views. It is part of the Beman C++ Standard Library incubation project targeting standardization.

## Build and Test

The project uses CMake Ninja Multi-Config with `uv` managing Python-based tools (cmake, ninja, ctest, pre-commit).

```bash
# Build and run all tests — C++ (ctest) + Python (pytest)
make test

# Build only
make compile

# Run tests without rebuilding
make ctest

# Lint: clang-format, cmake formatting, spell check, ruff, mypy, etc.
make lint

# Coverage report
make coverage

# Run Python tool tests only
make pytest

# Run mypy type checker only
make mypy
```

The `make` commands invoke `uv run cmake`/`uv run ctest`, so `uv` must be installed. The build directory is `.build/build-system/` by default; build config defaults to `Asan`.

To build/test a specific configuration directly:
```bash
uv run cmake --build .build/build-system/ --config Debug
uv run ctest --test-dir .build/build-system/ -C Debug --output-on-failure
```

To run a single test binary:
```bash
.build/build-system/tests/beman/transcode/Debug/beman.transcode.tests.null_term
```

## Architecture

The library is header-only under `include/beman/transcode/`. `transcode.hpp` is the umbrella header; `transcode.cppm` is the optional C++23 module interface (built when `BEMAN_TRANSCODE_USE_MODULES=ON`). It provides **two transcoding backends behind a shared pipeline-oriented interface** (see `README.md` for the full motivation).

Headers directly under `include/beman/transcode/` are the specification surface:
what the paper proposes, and what `beman.specgen` generates wording from.
`detail/` is implementation, and is invisible to the wording because specgen
only reads declarations in the file it is given. Adding a name to a
specification header therefore proposes it; see `docs/wording-outline.md`.

### WHATWG backend (portable, constexpr, header-only)

A compile-time implementation of the [WHATWG Encoding Standard](https://encoding.spec.whatwg.org/) covering ~39 codecs. The codec is a compile-time template parameter, selected from the `enum class codec` in `codec.hpp`.

- **`whatwg_decode_view.hpp` / `whatwg_encode_view.hpp`** — the public range adaptors. Each view is parameterized on a `transcode_error_kind` (from `error.hpp`): the lossy `whatwg_decode<codec>` / `whatwg_encode<codec>` closures (errors → `U+FFFD` on decode, `'?'` on encode) and the `..._or_error<codec>` closures yielding `std::expected<…, whatwg_error>` are the same view with a different error kind. Each header also has a `random_access_*` view used for O(1)-per-element single-byte codecs; `detail/whatwg_decode_select.hpp` / `detail/whatwg_encode_select.hpp` decide which codecs get it.
- **`decode_view.hpp` / `encode_view.hpp`** — the generic *pluggable* views parameterized on a codec object (not the `codec` enum). These drive any type satisfying the codec concepts.
- **`codec_concepts.hpp`** — `decode_codec` / `encode_codec` / `flushable_decode_codec` / `random_access_decode_codec_type`. The contract every codec implements: `decode_one(iter, sentinel) → decode_result`, `encode_one(cp) → encode_result`, optionally `flush()` and `decode_byte(byte)`. `codec_result.hpp` defines the result structs.
- **`detail/table_codec.hpp`** + **`detail/single_byte_*.hpp`** — table-driven single-byte codec engine.
- **Per-codec implementations** in `detail/`: `utf8.hpp`, `utf8_encode.hpp`, `utf16.hpp`, `utf32.hpp`, `big5.hpp`, `gb18030.hpp`, `gbk.hpp`, `shift_jis.hpp`, `euc_jp.hpp`, `euc_kr.hpp`, `iso2022jp.hpp`, `x_user_defined.hpp`, plus the single-byte tables.
- **`label.hpp`** — `get_encoding(label)`: WHATWG label → `codec` lookup (e.g. `"latin1"` → `windows_1252`). The table behind it, `detail/labels.hpp`, is **generated** by `tools/generate_labels.py`; do not hand-edit.
- **`sniff.hpp`** — BOM sniffing (`sniff_encoding`) per WHATWG §8.2.
- **`transcode_string.hpp`** — eager `transcode_string(src, from, to)` convenience over the views, with a label-string overload.
- **`transcode_view.hpp`** / **`bulk_transcode.hpp`** — the composed decode→encode pipeline (`transcode<From, To>`, `pluggable_transcode(from, to)`) and the bulk helpers (`decode_to`, `encode_to`, `decode_into`, `encode_into`).

### iconv backend (system encodings)

A C++ ranges wrapper around the POSIX `iconv` API: `iconv_transcode_view.hpp`, `iconv_transcode_or_error_view.hpp`, `iconv_real.hpp`, `iconv_bulk.hpp`. Gives the existing iconv ecosystem a leak-free, composable ranges interface. Tests use `tests/beman/transcode/iconv_mock.hpp` to avoid depending on the platform's iconv tables.

### Shared plumbing

- **`concepts.hpp`** — the `legacy_byte_range` concept: accepts ranges of `char`/`signed char`/`unsigned char`/`std::byte`; explicitly rejects raw arrays (avoids silent null-terminator inclusion) and `char8_t`/wide char types.
- **`null_term.hpp`** — `views::null_term`: adapts a pointer to a null-terminated C string into a range. Requires a pointer; rejects non-pointer ranges.
- **`error.hpp`** — the `whatwg_error` and `iconv_error` enums, and `transcode_error_kind` (replacement vs. `std::expected`), which selects between the lossy and `_or_error` behavior of every view.
- **`config.hpp`** — build configuration; `BEMAN_TRANSCODE_USE_MODULES()` switches the headers between including and importing.

Typical usage: `bytes | beman::transcoding::whatwg_decode<codec::utf_8>` → a `char32_t` range; `views::null_term(ptr) | whatwg_decode<codec::utf_8>` for a C string.

### Generated data and tooling (`tools/`, `data/`, `docs/whatwg/`)

The codec tables are generated, not authored. Source-of-truth WHATWG index files live in `docs/whatwg/`; Python tools turn them into committed C++ headers:

- `tools/download_indexes.py` — fetches the WHATWG index files.
- `tools/generate_tables.py` — emits `include/beman/transcode/detail/tables/*.hpp` and the binary `data/tables/*.bin`.
- `tools/generate_labels.py` — emits `detail/labels.hpp`.
- `tools/generate_wpt_vectors.py` — emits the `tests/beman/transcode/wpt_*_vectors.hpp` test vectors from Web Platform Tests.

Python tools have their own pytest suite under `tools/tests/` (run via `make pytest`). When changing a codec table or label set, regenerate via the tool and commit the output — don't edit generated headers by hand.

### Benchmarks

`benchmark/` holds Google Benchmark suites and baseline comparisons against `iconv`, `std::codecvt`, `simdutf`, and Rust `encoding_rs` (FFI in `benchmark/encoding_rs_bench/`), with corpora in `benchmark/corpus/`. See `docs/benchmarks/RUNNING.md`.

## Testing Conventions

Tests use **Catch2 3.x** and live in `tests/beman/transcode/`.

### Negative compile tests

Every concept constraint or `static_assert` must have:

1. A **negative compile test** — a `.cpp` file that should fail to compile, registered as a ctest with `WILL_FAIL` implicit (via `PASS_REGULAR_EXPRESSION` on the build command). The `PASS_REGULAR_EXPRESSION` must match the *specific* compiler diagnostic proving the right constraint fired, not just any error.

2. A **consteval test** — use `constify()` from `test_utilities.hpp` to verify the expression is valid in a constant evaluation context:
   ```cpp
   #include "tests/beman/transcode/test_utilities.hpp"
   using beman::transcoding::tests::constify;
   // ...
   CHECK(constify(some_expr) == expected);
   ```

**TDD order**: write the runtime test first, then the negative compile test, then the consteval test.

Negative compile tests are registered in `tests/beman/transcode/CMakeLists.txt` as `OBJECT` libraries with `EXCLUDE_FROM_ALL` plus an `add_test` that invokes `cmake --build --target <name>` and checks `PASS_REGULAR_EXPRESSION` against the compiler output.

### Coverage

Run `make coverage` to generate a coverage report. New code should have coverage. Don't fight the compiler to reach 100% — template instantiations and platform-specific branches may not all be reachable. But any *surprising* uncovered code (logic you expected tests to hit) should either get a test added or be noted for follow-up. Branch coverage is not a priority; line/function coverage is what matters.

## Code Style

Formatting is enforced by `.clang-format` (clang-format 22).
Run `make lint` to apply.
CMake files are formatted by `gersemi`.

License header on every file: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`

## Coding Rules

**Include guards** — use `#ifndef`/`#define`/`#endif`, never `#pragma once`.
Guard name mirrors the repo-relative path with `/` and `.` replaced by `_`, uppercased.
Examples: `include/beman/transcode/detail/range_traits.hpp` → `INCLUDE_BEMAN_TRANSCODE_DETAIL_RANGE_TRAITS_HPP`; `tests/beman/transcode/iconv_mock.hpp` → `TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP`.

**Includes** — always angle brackets (`<...>`), never quotes (`"..."`).
Use the full path from the include root: `<beman/transcode/concepts.hpp>`, `<tests/beman/transcode/test_utilities.hpp>`.
Never use relative `.`/`..` paths.
Never rely on transitive includes — include every header you directly use.

**Include order in test files** — the component header under test goes first (verifies it is self-contained).
Include it twice consecutively to verify idempotent inclusion (the include guard must prevent duplicate definitions).

**Functions out-of-line** — define function bodies outside the class body, inside the header, with full `ClassName::method_name` qualification (see `iconv_transcode_view.hpp` and `whatwg_decode_view.hpp` for the pattern).
Exception: hidden `friend` functions used as customization points (e.g., `operator==` against `default_sentinel_t`, `operator|` for pipe adapters) may be defined inline inside the class body.

**No `using namespace` in headers.**
Namespace nesting mirrors directory depth.

**`constexpr`** — make every API that can be `constexpr` so, and add a compile-time test using `constify()` from `tests/beman/transcode/test_utilities.hpp`.

**Directory layout** — split `include/`/`src/`/`tests/` trees; no co-located component trios.
