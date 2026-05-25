# Handoff: beman.transcode — Step 47

## Project

`beman.transcode` — C++20 header-only library for Unicode transcoding
using ranges and views. Beman Project incubation for a C++29 standard
proposal.

- GitHub: `https://github.com/steve-downey/transcode.git` (remote `origin`)
- Bloomberg: `bbgithub.dev.bloomberg.com:sdowney/transcode.git` (remote `bbgithub`)
- Working directory: `/home/sdowney/src/steve-downey/transcode/transcode`
- Two remotes; always push both: `git push origin <branch> && git push bbgithub <branch>`
- Merge to main: `git merge --no-ff`

## Current State

**Step 46 complete and merged to main.**

557 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures in `papers/wg21/` and `tools/tests/test_generate_labels.py`
(Python ruff/codespell issues unrelated to the transcode C++ library). The
C++ and CMake portions of lint pass cleanly.

Coverage (after step 46): lines 83.4%, functions 99.9% (2026/2027).

### What Step 46 Added

A `transcode_closure<From, To>` variable template in
`include/beman/transcode/detail/transcode_view.hpp`:

```cpp
// Usage:
auto v = bytes | transcode<codec::windows_1252, codec::utf_8>;
auto v = bytes | transcode<codec::utf_8, codec::gbk>;
```

The closure wraps `whatwg_decode<From> | whatwg_encode<To>` into a single
pipeable range adapter. Result type is
`whatwg_encode_view<To, whatwg_decode_view<From, all_t<R>>>` — zero
overhead, no intermediate allocation, lazily evaluated.

7 new tests in `tests/beman/transcode/transcode_view.test.cpp`.

The header is included in the umbrella `include/beman/transcode/transcode.hpp`.

## What To Do Next — Step 47

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–46 are complete. The checklist does not yet have a step 47 entry.

### Recommended step 47: `transcode_view` negative compile test + consteval test

**Part A — negative compile test** for raw arrays:

Add a `.cpp` file that tries to pipe a raw `char[]` array into `transcode<>`:
```cpp
// transcode_view_reject_array_fail.cpp
#include <beman/transcode/detail/transcode_view.hpp>
char arr[] = "hello";
auto v = arr | beman::transcoding::transcode<beman::transcoding::codec::utf_8,
                                             beman::transcoding::codec::utf_8>;
```
This should trigger the `static_assert` with message:
```
"transcode: raw arrays are not valid input to transcode"
```
Register in `tests/beman/transcode/CMakeLists.txt` as an `OBJECT` library
with `EXCLUDE_FROM_ALL` and `PASS_REGULAR_EXPRESSION` on the build command.
See existing patterns at the bottom of CMakeLists.txt (e.g.,
`concepts_reject_arrays_fail`).

**Part B — consteval test** that actually exercises the closure:

The current step 46 consteval test is a stub (`constify(true)`). Replace or
augment it with a real test. Note: `whatwg_decode`/`whatwg_encode` closures
are `constexpr` but the underlying iterators may not be fully consteval
because they call `static` lookup tables. Verify first:

```cpp
// In transcode_view.test.cpp — attempt a real consteval round-trip
// If the view is truly constexpr-usable, something like:
constexpr bool check_transcode_closure_constexpr() {
    // ... iterate over a span and check bytes
}
static_assert(check_transcode_closure_constexpr());
```

If the views are not consteval-compatible (likely due to `static` tables or
non-constexpr iconv internals), document it in a comment and keep `constify(true)`.

### Alternative step 47: `transcode_string` consteval tests

`get_encoding(string_view)` in `detail/labels.hpp` is `constexpr`. Add
`constify()` tests in `labels.test.cpp` that verify compile-time evaluation:
```cpp
CHECK(constify(get_encoding("utf-8") == codec::utf_8));
CHECK(constify(!get_encoding("bogus").has_value()));
```

`sniff_encoding()` in `detail/sniff.hpp` is also likely `constexpr`. Add
similar tests in `sniff.test.cpp`.

## TDD Process

Each step is a separate branch: `step<N>-<slug>`, branched from `main`.

1. Write failing tests (RED) → commit → push both remotes
2. Implement (GREEN) → `make test` (all pass) → `make lint` → commit → push both remotes
3. `git checkout main && git merge --no-ff step<N>-...` → push both remotes
4. Update `docs/plans/phase2-checklist.md` — mark completed items `[x]`

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
make mypy      # mypy type checker only
```

## Coding Rules (abbreviated)

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_VIEW_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files for context

- `include/beman/transcode/transcode.hpp` — umbrella header (includes transcode_view.hpp)
- `include/beman/transcode/detail/transcode_view.hpp` — step 46: `transcode_closure<From,To>` + `transcode<From,To>` variable template
- `include/beman/transcode/detail/transcode_string.hpp` — step 43+45 one-shot function (codec + label overloads)
- `include/beman/transcode/detail/labels.hpp` — `get_encoding(string_view)` constexpr lookup
- `include/beman/transcode/detail/sniff.hpp` — `sniff_encoding()` constexpr BOM detection
- `include/beman/transcode/detail/concepts.hpp` — `legacy_byte_range`, `unicode_scalar_range`
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view + `whatwg_decode` closure + `codec` enum
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view + `whatwg_encode` closure
- `tests/beman/transcode/transcode_view.test.cpp` — step 46 tests (style reference)
- `tests/beman/transcode/transcode_string_label.test.cpp` — step 45 tests (style reference)
- `tests/beman/transcode/CMakeLists.txt` — how to register test targets and negative compile tests
