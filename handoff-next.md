# Handoff: beman.transcode — Step 46

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

**Step 45 complete and merged to main.**

550 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures in `papers/wg21/` and `tools/tests/test_generate_labels.py`
(Python ruff/codespell issues unrelated to the transcode C++ library). The
C++ and CMake portions of lint pass cleanly.

Coverage (after step 45): lines 83.2%, functions 99.9% (1962/1963 — the
new label overload is covered by the 9 new tests).

### What Step 45 Added

A label-based overload of `transcode_string()` in
`include/beman/transcode/detail/transcode_string.hpp`:

```cpp
std::optional<std::string> transcode_string(
    std::span<const char> src,
    std::string_view from_label,
    std::string_view to_label);
```

Implementation delegates to `get_encoding()` (from `detail/labels.hpp`) for
WHATWG label lookup, then calls the existing `codec`-enum overload. Returns
`std::nullopt` if either label is unknown. Case-insensitive, whitespace-stripped
per WHATWG §4.2.

9 new tests in `tests/beman/transcode/transcode_string_label.test.cpp`.

## What To Do Next — Step 46

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–45 are complete. The checklist does not yet have a step 46 entry.

### Recommended step 46: `transcode_view` pipe composition helper

A combined decode+encode range adapter closure that wraps
`whatwg_decode<From> | whatwg_encode<To>` into a single pipeable object:

```cpp
// Usage:
auto v = bytes | transcode<codec::shift_jis, codec::utf_8>;
auto v = bytes | transcode<codec::utf_8, codec::gbk>;
```

**Design sketch:**

```cpp
// include/beman/transcode/detail/transcode_view.hpp
template <codec From, codec To>
struct transcode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const {
        return std::forward<R>(r) | whatwg_decode<From> | whatwg_encode<To>;
    }
    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const transcode_closure& self) {
        return self(std::forward<R>(r));
    }
};

template <codec From, codec To>
inline constexpr transcode_closure<From, To> transcode{};
```

Result type is `whatwg_encode_view<To, whatwg_decode_view<From, all_t<R>>>` —
zero overhead, no intermediate allocation, lazily evaluated.

**Files to create/edit:**
- `include/beman/transcode/detail/transcode_view.hpp` (new header)
- `include/beman/transcode/transcode.hpp` (add `#include` for new header)
- `tests/beman/transcode/transcode_view.test.cpp` (new test file)
- `tests/beman/transcode/CMakeLists.txt` (register new test target)

**TDD checklist for step 46:**
1. Write failing tests in `transcode_view.test.cpp` (RED: `transcode_view.hpp`
   doesn't exist yet)
2. Register in CMakeLists.txt
3. Commit RED + push both remotes
4. Implement `transcode_view.hpp` + add to umbrella header
5. `make test` (all pass) + `make lint` + `make coverage`
6. Commit GREEN + push both remotes
7. Merge to main + push both remotes
8. Update checklist + write handoff-next.md

**Tests to write (RED phase):**

```cpp
// transcode_view.test.cpp
#include <beman/transcode/detail/transcode_view.hpp>
#include <beman/transcode/detail/transcode_view.hpp>  // idempotent

// Basic pipe syntax
TEST_CASE("transcode: windows-1252 bytes → UTF-8 via pipe") {
    std::string src{'\x80'};  // 0x80 in windows-1252 → U+20AC EURO SIGN
    std::string expected{'\xE2', '\x82', '\xAC'};
    std::string result;
    for (char b : std::span<const char>(src) | transcode<codec::windows_1252, codec::utf_8>)
        result.push_back(b);
    CHECK(result == expected);
}

TEST_CASE("transcode: UTF-8 → windows-1252 via pipe") { ... }
TEST_CASE("transcode: UTF-8 → GBK via pipe (中)") { ... }
TEST_CASE("transcode: consteval — transcode_closure is trivially constructible") { ... }
```

**Design note — naming:** The variable template `transcode` in namespace
`beman::transcoding` should not conflict with anything — there is no free
function named `transcode` in that namespace. Verify with `grep -r "^transcode\b"` 
if in doubt.

### Alternative step 46: `transcode_string` consteval tests

Add `constify()` consteval tests for both `transcode_string` overloads and
`get_encoding`. Most of these functions are already `constexpr`/`inline`; verify
they work at compile time.

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

- `include/beman/transcode/transcode.hpp` — umbrella header
- `include/beman/transcode/detail/transcode_string.hpp` — step 43+45 one-shot function (codec + label overloads)
- `include/beman/transcode/detail/labels.hpp` — `get_encoding(string_view)` constexpr lookup
- `include/beman/transcode/detail/concepts.hpp` — `legacy_byte_range`, `unicode_scalar_range`
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view + `whatwg_decode` closure + `codec` enum
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view + `whatwg_encode` closure
- `tests/beman/transcode/transcode_string.test.cpp` — test style reference (steps 43–44)
- `tests/beman/transcode/transcode_string_label.test.cpp` — test style reference (step 45)
- `tests/beman/transcode/CMakeLists.txt` — how to register a new test target
