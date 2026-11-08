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

Documentation site (what GitHub Pages publishes, via `.github/workflows/docs-pages.yml`):
```bash
make docs        # Doxygen -> .build/docs/doxygen/html
make mrdocs      # MrDocs  -> .build/docs/mrdocs (needs mrdocs on PATH)
make paper       # the paper -> papers/generated/transcode-view.{html,pdf}
make site        # all of the above, then the index and the rendered Markdown -> .build/docs/site
make site-pages  # just the index and the rendered Markdown, with whatever is already built
```
`tools/build_site.py` assembles the site; `docs/site/index.md` is the authored part of its landing page.
Every Markdown file under `docs/` and `papers/` (except the paper's own sources and the vendored frameworks) is rendered and listed on the index.

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

Headers directly under `include/beman/transcode/` and `include/beman/transcode/whatwg/` are the specification surface:
what the paper proposes, and what `beman.specgen` generates wording from.
`detail/` is implementation, and is invisible to the wording because specgen
only reads declarations in the file it is given. Adding a name to a
specification header therefore proposes it; see `docs/wording-outline.md`.

### WHATWG backend (portable, constexpr, header-only)

A compile-time implementation of the [WHATWG Encoding Standard](https://encoding.spec.whatwg.org/), in namespace `beman::transcoding::whatwg` under `include/beman/transcode/whatwg/`. The encoding is a compile-time template parameter, selected from `enum class whatwg::encoding` in `whatwg/encoding.hpp`.

- **`whatwg/decoder.hpp` / `whatwg/encoder.hpp`** — `whatwg::decoder<E>` and `whatwg::encoder<E>`, the WHATWG decoders and encoders as codec objects. They model the generic codec concepts below, so the library's own codecs and a program's are the same kind of thing.
- **`whatwg/decode_view.hpp` / `whatwg/encode_view.hpp`** — the variable templates `whatwg::decode<E>` / `whatwg::decode_or_error<E>` and `whatwg::encode<E>` / `whatwg::encode_or_error<E>`. Each is the generic codec-object adaptor over `whatwg::decoder<E>` or `whatwg::encoder<E>`; there are no WHATWG-specific view classes. The lossy forms substitute (U+FFFD on decode, `'?'` on encode); the `_or_error` forms yield `std::expected<…, whatwg::error>`. `detail/whatwg_decode_select.hpp` / `detail/whatwg_encode_select.hpp` decide which encodings are single-byte and so get the random-access path.
- **`decode_view.hpp` / `encode_view.hpp`** — the generic views and adaptors parameterized on a codec object: `decode(codec)`, `encode(codec)`, and their `_or_error` forms. A codec modeling `single_byte_text_decoder` or `single_byte_text_encoder` gets `random_access_decode_view` / `random_access_encode_view`, which are implementation and `\omit`ted from the wording.
- **`codec_concepts.hpp`** — `text_decoder`, `text_encoder`, `flushable_text_decoder`, `flushable_text_encoder`, `bom_aware_text_decoder`, `single_byte_text_decoder`, `single_byte_text_encoder`. The contract: `decode_one(iter, sentinel) → decode_result`, `encode_one(cp) → encode_result`, and optionally `flush()`, `decode_first(iter, sentinel)`, `decode_byte(byte)` and `encode_byte(scalar)`. `codec_result.hpp` defines the result structs.
- **`detail/table_codec.hpp`** + **`detail/single_byte*.hpp`** — table-driven single-byte codec engine.
- **Per-codec implementations** in `detail/`: `utf8.hpp`, `utf8_encode.hpp`, `utf16.hpp`, `utf32.hpp`, `big5.hpp`, `gb18030.hpp`, `gbk.hpp`, `shift_jis.hpp`, `euc_jp.hpp`, `euc_kr.hpp`, `iso2022jp.hpp`, `x_user_defined.hpp`, plus the single-byte tables and `reverse_index.hpp`.
- **`whatwg/label.hpp`** — `whatwg::get_encoding(label)`: WHATWG label → `whatwg::encoding` lookup (e.g. `"latin1"` → `windows_1252`). The table behind it, `detail/labels.hpp`, is **generated** by `tools/generate_labels.py`; do not hand-edit. **`whatwg/text_encoding.hpp`** bridges to `std::text_encoding::id` both ways (`get_encoding(id)`, `to_text_encoding_id(e)`).
- **`whatwg/bom_sniff.hpp`** — `whatwg::bom_sniff`, the Standard's "BOM sniff". Requires a `forward_range`, since it peeks without consuming.
- **`whatwg/transcode_string.hpp`** — eager `whatwg::transcode_string(src, from, to)` over the views, with a label-string overload; returns `optional<string>`, empty when `to` has no encoder.
- **`whatwg/transcode_view.hpp`** / **`transcode_view.hpp`** — the composed decode→encode pipeline: `whatwg::transcode<From, To>`, and the generic `transcode(from, to)` over codec objects.
- **`whatwg/bulk.hpp`** / **`bulk_transcode.hpp`** — the bulk helpers `decode_to`, `encode_to`, `decode_into`, `encode_into`, in WHATWG and generic forms.

### iconv backend (system encodings)

A C++ ranges wrapper around the POSIX `iconv` API: `iconv_transcode_view.hpp`, `iconv_transcode_or_error_view.hpp`, `iconv_real.hpp`, `iconv_bulk.hpp`. Gives the existing iconv ecosystem a leak-free, composable ranges interface. Tests use `tests/beman/transcode/iconv_mock.hpp` to avoid depending on the platform's iconv tables.

### Shared plumbing

- **`concepts.hpp`** — the `legacy_byte_range` concept: accepts ranges of `char`/`signed char`/`unsigned char`/`std::byte`; explicitly rejects raw arrays (avoids silent null-terminator inclusion) and `char8_t`/wide char types.
- **`null_term.hpp`** — `views::null_term`: adapts a pointer to a null-terminated C string into a range. Requires a pointer; rejects non-pointer ranges.
- **`error.hpp`** / **`whatwg/error.hpp`** — the `iconv_error` and `whatwg::error` enums, and `error_mode` (replacement vs. `std::expected`), which selects between the lossy and `_or_error` behavior of every view.
- **`config.hpp`** — build configuration; `BEMAN_TRANSCODE_USE_MODULES()` switches the headers between including and importing.

Typical usage: `bytes | beman::transcoding::whatwg::decode<whatwg::encoding::utf_8>` → a `char32_t` range; `views::null_term(ptr) | whatwg::decode<whatwg::encoding::utf_8>` for a C string.

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

1. A **negative compile test** — a `.cpp` file that should fail to compile, registered with `beman_add_xfail_compile_test()` from `cmake/beman-xfail-compile.cmake`. The source carries one or more `// EXPECT: "<regex>"` lines, and the test passes only when the build fails *and* its output matches one of them. Each regex must match the *specific* compiler diagnostic proving the right constraint fired, not just any error. GCC and Clang word constraint failures differently (`required for the satisfaction of 'C<T>' [with T = X]` versus `'X' does not satisfy 'C'`), so a constraint test usually carries one EXPECT line per compiler. The wording also drifts between Clang versions, and CI runs several (19 through 22 at the time of writing): Clang 19 and 20 print a non-type template argument unquoted (`(encoding)36 does not satisfy 'C'`) where 21 quotes it, and how far a type is namespace-qualified varies, so write `'?` for a quote that moves and prefer `.*` to `.+` between tokens that may be adjacent. When a test fails only in CI, read the quoted compiler line from the log, not its line breaks: CMake reflows the driver's error report at 80 columns, but the match was made against the raw output. Keep each line within the clang-format column limit; `ReflowComments` would otherwise split it.

2. A **consteval test** — use `constify()` from `test_utilities.hpp` to verify the expression is valid in a constant evaluation context:
   ```cpp
   #include "tests/beman/transcode/test_utilities.hpp"
   using beman::transcoding::tests::constify;
   // ...
   CHECK(constify(some_expr) == expected);
   ```

**TDD order**: write the runtime test first, then the negative compile test, then the consteval test.

Negative compile tests are registered in `tests/beman/transcode/CMakeLists.txt` as `beman_add_xfail_compile_test(<name> SOURCE <file> LINK_LIBRARIES beman::transcode)`. The function makes the source an `OBJECT` library excluded from the default build and adds a ctest that runs a nested `cmake --build --target` on it and matches the output against the source's `EXPECT` lines, reporting "compiled", "failed for another reason", and "failed as expected" separately. Every such test carries the CTest label `xfail-compile`, so `ctest -L xfail-compile` runs just that suite and `-LE xfail-compile` skips it. Run a new one under both GCC and Clang (`make TOOLCHAIN=gcc-16 ...` and `make TOOLCHAIN=clang-23 ...`) before pushing it. The driver reads the `EXPECT` lines when the test runs, so editing one takes effect on the next `ctest` with no reconfigure or rebuild.

### Coverage

Run `make coverage` to generate a coverage report. New code should have coverage. Don't fight the compiler to reach 100% — template instantiations and platform-specific branches may not all be reachable. But any *surprising* uncovered code (logic you expected tests to hit) should either get a test added or be noted for follow-up. Branch coverage is not a priority; line/function coverage is what matters.

## Code Style

Formatting is enforced by `.clang-format`, with the clang-format version pinned in `pyproject.toml`.
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

**Functions out-of-line** — define function bodies outside the class body, inside the header, with full `ClassName::method_name` qualification (see `iconv_transcode_view.hpp` and `decode_view.hpp` for the pattern).
Exception: hidden `friend` functions used as customization points (e.g., `operator==` against `default_sentinel_t`, `operator|` for pipe adapters) may be defined inline inside the class body.

**No `using namespace` in headers.**
Namespace nesting mirrors directory depth.

**`constexpr`** — make every API that can be `constexpr` so, and add a compile-time test using `constify()` from `tests/beman/transcode/test_utilities.hpp`.

**Directory layout** — split `include/`/`src/`/`tests/` trees; no co-located component trios.
