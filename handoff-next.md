# Handoff: beman.transcode — Step 48

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

**Step 47 complete and merged to main.**

558 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures in `papers/wg21/` and `tools/tests/test_generate_labels.py`
(Python ruff/codespell issues unrelated to the transcode C++ library). The
C++ and CMake portions of lint pass cleanly.

### What Step 47 Added

1. **Negative compile test** for `transcode_closure` raw array rejection:
   - `tests/beman/transcode/transcode_view_reject_array_fail.cpp` — tries
     to pipe a `char[]` into `transcode<>`, expects the `static_assert`
     diagnostic `"transcode: raw arrays are not valid input to transcode"`.
   - Registered in `tests/beman/transcode/CMakeLists.txt` as
     `transcode_view_reject_array_fail` (EXCLUDE_FROM_ALL + PASS_REGULAR_EXPRESSION).

2. **Consteval test** replacing the stub `constify(true)` in
   `tests/beman/transcode/transcode_view.test.cpp`:
   - A real round-trip check: ASCII `'A'` through
     `transcode<codec::utf_8, codec::utf_8>` returns `'\x41'` in consteval
     context.

3. **Bug fix** — added `constexpr` to four `operator==(iterator, default_sentinel_t)`
   hidden friend functions:
   - `whatwg_decode_view::iterator::operator==` (line ~138)
   - `whatwg_decode_or_error_view::iterator::operator==` (line ~219)
   - `whatwg_encode_view::iterator::operator==` (line ~88)
   - `whatwg_encode_or_error_view::iterator::operator==` (line ~157)
   These were the only non-`constexpr` members in both views.

## What To Do Next — Step 48

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–47 are complete. The checklist does not yet have a step 48 entry.

### Recommended step 48: `labels.hpp` and `sniff.hpp` consteval tests

**Part A — `get_encoding()` consteval tests** in
`tests/beman/transcode/labels.test.cpp` (file already exists):

`get_encoding(string_view)` in `include/beman/transcode/detail/labels.hpp`
is declared `constexpr`. Add a new `TEST_CASE` with `constify()` checks:

```cpp
#include <tests/beman/transcode/test_utilities.hpp>
using beman::transcoding::tests::constify;

TEST_CASE("labels: get_encoding consteval", "[labels]") {
    CHECK(constify(get_encoding("utf-8") == codec::utf_8));
    CHECK(constify(!get_encoding("bogus").has_value()));
    CHECK(constify(get_encoding("  UTF-8  ") == codec::utf_8));  // whitespace strip
    CHECK(constify(get_encoding("shift_jis") == codec::shift_jis));
    CHECK(constify(get_encoding("UTF-8") == codec::utf_8));      // case-insensitive
}
```

**Part B — `sniff_encoding()` consteval tests** in
`tests/beman/transcode/sniff.test.cpp` (file already exists):

`sniff_encoding()` in `include/beman/transcode/detail/sniff.hpp` is
`constexpr`. Add constify() checks for BOM detection at compile time.

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

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_LABELS_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files for context

- `include/beman/transcode/detail/labels.hpp` — `get_encoding(string_view)` constexpr lookup
- `include/beman/transcode/detail/sniff.hpp` — `sniff_encoding()` constexpr BOM detection
- `include/beman/transcode/detail/transcode_view.hpp` — step 46: `transcode_closure<From,To>` + `transcode<From,To>` variable template
- `include/beman/transcode/detail/transcode_string.hpp` — one-shot function (codec + label overloads)
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view (all `constexpr` including `operator==`)
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view (all `constexpr` including `operator==`)
- `tests/beman/transcode/labels.test.cpp` — existing labels tests (add constify tests here)
- `tests/beman/transcode/sniff.test.cpp` — existing sniff tests (add constify tests here)
- `tests/beman/transcode/transcode_view.test.cpp` — step 47 consteval test (style reference)
- `tests/beman/transcode/CMakeLists.txt` — how to register test targets and negative compile tests
