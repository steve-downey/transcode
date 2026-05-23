# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

`beman.transcode` is a C++20 header-only library for Unicode transcoding using ranges and views. It is part of the Beman C++ Standard Library incubation project targeting standardization.

## Build and Test

The project uses CMake Ninja Multi-Config with `uv` managing Python-based tools (cmake, ninja, ctest, pre-commit).

```bash
# Build and run all tests (default target)
make test

# Build only
make compile

# Run tests without rebuilding
make ctest

# Lint (clang-format, cmake formatting, spell check, etc.)
make lint

# Coverage report
make coverage
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

The library is entirely in `include/beman/transcode/`:

- **`whatwg_decode_view.hpp`** — Core range view that decodes a byte range (legacy chars/`std::byte`) to `char32_t` Unicode codepoints following the WHATWG UTF-8 codec. Invalid sequences yield `U'�'`.
- **`detail/concepts.hpp`** — The `legacy_byte_range` concept: accepts ranges of `char`/`signed char`/`unsigned char`/`std::byte`, explicitly rejects raw arrays (to avoid silent null-terminator inclusion) and char8_t/wide char types.
- **`detail/null_term.hpp`** — `views::null_term`: adapts a pointer to a null-terminated C string into a range, usable with `whatwg_decode_view`. Requires a pointer type; rejects non-pointer ranges.
- **`detail/error.hpp`** — `whatwg_error` and `iconv_error` enums for future use.
- **`transcode.cppm`** — Optional C++23 module interface (when `BEMAN_TRANSCODE_USE_MODULES=ON`).

Typical usage: `views::null_term(ptr) | whatwg_decode_view` to get a `char32_t` range from a C string.

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

## Code Style

Formatting is enforced by `.clang-format` (clang-format 22). Run `make lint` to apply. CMake files are formatted by `gersemi`.

License header on every file: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
