# Handoff: beman.transcode — Step 45

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

**Step 44 complete and merged to main.**

541 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures in `papers/wg21/` and `tools/tests/test_generate_labels.py`
(Python ruff/codespell issues unrelated to the transcode C++ library). The
C++ and CMake portions of lint pass cleanly.

Coverage (after step 44): lines 83.2%, functions 99.9% (1961/1962).

### What Step 44 Added

Coverage audit of `transcode_string.hpp`: 10 new test cases that exercise
every previously uncovered `switch` arm in `transcode_string()`:

- ASCII identity through all 27 single-byte decode/encode codec arms not
  previously exercised (ibm866, all iso_8859_*, koi8_r/u, macintosh,
  windows_874–1258, x_mac_cyrillic, x_user_defined, shift_jis decode)
- UTF-16LE and UTF-16BE decode and encode (byte-order verification)
- All 5 CJK codec decode arms (gb18030, big5, euc_jp, iso_2022_jp, euc_kr)
- All 5 CJK codec encode arms
- Default encode arm: `to=codec::replacement` and `to=codec::x_user_defined`
  both yield empty string (no encoder defined for those codecs)

Coverage went from 58.9% lines / 67.2% functions → 83.2% lines / 99.9%
functions. Only 1 function remains uncovered (likely an unreachable
template instantiation branch inside Catch2 or a constexpr-only path).

## What To Do Next — Step 45

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–44 are complete. The checklist does not yet have a step 45 entry.

### Recommended step 45: `transcode_view` pipe composition helper

A combined decode+encode range adapter closure that wraps
`whatwg_decode<From> | whatwg_encode<To>` into a single pipeable object:

```cpp
// Usage:
auto v = bytes | transcode<codec::shift_jis, codec::utf_8>;
// or:
auto v = bytes | transcode<codec::utf_8, codec::gbk>;
```

**Design sketch** (from handoff-next of step 43):

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

**TDD checklist for step 45:**
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

**Design note — naming conflict:** The variable template `transcode` may
conflict with the function `transcode_string` or existing names in the
namespace. Using `transcode` as a variable template in namespace
`beman::transcoding` should be fine since there's no free function with that
name. The umbrella header brings `transcode_string` into scope; verify there
is no ambiguity.

### Alternative step 45: `transcode_string` label overload

A thin overload accepting string labels instead of `codec` enum values:

```cpp
std::optional<std::string> transcode_string(
    std::span<const char> src,
    std::string_view from_label,
    std::string_view to_label);
```

Implementation:
```cpp
inline std::optional<std::string> transcode_string(
        std::span<const char> src,
        std::string_view from_label, std::string_view to_label) {
    auto from = get_encoding(from_label);
    auto to   = get_encoding(to_label);
    if (!from || !to) return std::nullopt;
    return transcode_string(src, *from, *to);
}
```

This is simpler but less architecturally interesting than `transcode_view`.

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
- `include/beman/transcode/detail/transcode_string.hpp` — step 43 one-shot function
- `include/beman/transcode/detail/concepts.hpp` — `legacy_byte_range`, `unicode_scalar_range`
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view + `whatwg_decode` closure + `codec` enum
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view + `whatwg_encode` closure
- `tests/beman/transcode/transcode_string.test.cpp` — test style reference (step 43–44)
- `tests/beman/transcode/CMakeLists.txt` — how to register a new test target
