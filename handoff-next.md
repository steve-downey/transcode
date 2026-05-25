# Handoff: beman.transcode — Step 44

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

**Step 43 complete and merged to main.**

531 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures in `papers/wg21/` and `tools/tests/test_generate_labels.py`
(Python ruff/codespell issues unrelated to the transcode C++ library). The
C++ and CMake portions of lint pass cleanly.

### What Step 43 Added

`transcode_string` — a one-shot runtime-dispatch transcoding function that
decodes bytes from one codec and re-encodes them to another codec, returning
a `std::string`.

Key changes:
- `include/beman/transcode/detail/transcode_string.hpp` — the new header:
  - `detail::transcode_decode_all<C>(std::span<const char>)` → `std::u32string`
  - `detail::transcode_encode_all<C>(std::u32string_view)` → `std::string`
  - `transcode_string(std::span<const char> src, codec from, codec to)` → `std::string`
    - Runtime switch dispatch for `from` (all 40 codecs)
    - Runtime switch dispatch for `to` (all encodeable codecs; `replacement`
      and `x_user_defined` fall through to empty-string default)
    - Decode errors produce U+FFFD; unmapped encode codepoints produce `?`
    - Intermediate representation: `std::u32string` (O(N) template
      instantiations instead of O(N²))
- `include/beman/transcode/transcode.hpp` — umbrella header now also includes
  `<beman/transcode/detail/transcode_string.hpp>`
- `tests/beman/transcode/transcode_string.test.cpp` — 10 runtime tests:
  empty input, UTF-8 identity, UTF-8 multibyte identity, windows-1252↔UTF-8
  (euro sign), invalid UTF-8 → replacement, ASCII through shift_jis,
  UTF-8↔GBK round-trip, unmapped codepoint → `?`, replacement-codec decode

Users can now:
```cpp
#include <beman/transcode/transcode.hpp>
using namespace beman::transcoding;

// Convert a legacy-encoded string to UTF-8:
std::string utf8 = transcode_string(span_of_win1252_bytes, codec::windows_1252, codec::utf_8);

// Or with runtime codec lookup:
auto from = get_encoding("shift_jis");
if (from) utf8 = transcode_string(src, *from, codec::utf_8);
```

## What To Do Next — Step 44

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–43 are complete. The checklist does not yet have a step 44 entry —
look at `docs/plans/phase2-index.md` for the phase overview.

### Likely candidates for step 44

1. **`transcode_view` pipe composition helper** — a combined decode+encode
   range adapter closure that composes `whatwg_decode` → `whatwg_encode`:
   ```cpp
   auto v = bytes | transcode<codec::shift_jis, codec::utf_8>;
   // or with compile-time codecs:
   auto v = bytes | transcode<codec::utf_8, codec::gbk>;
   ```
   This would be a range adapter closure wrapping two closures. Since both
   `from` and `to` are compile-time NTTPs, the result type would be:
   `whatwg_encode_view<To, whatwg_decode_view<From, std::views::all_t<R>>>`.
   The design challenge: how does the combined closure handle the two NTTP
   parameters while remaining pipeable?

2. **`transcode_string` with runtime codec via `get_encoding`** — an
   overload or variant that takes string label names directly:
   ```cpp
   std::optional<std::string> transcode_string(std::span<const char> src,
       std::string_view from_label, std::string_view to_label);
   ```
   This is a thin wrapper around `get_encoding()` + `transcode_string()`.

3. **Coverage audit** — run `make coverage` on the new `transcode_string`
   header and check that the decode/encode dispatch is adequately covered.
   The template instantiations for less-common codecs may not be covered
   by the 10 existing tests. Add targeted tests if surprising gaps appear.

4. **More WPT conformance tests** — check if any WPT test files in
   `docs/wpt/` haven't been imported yet.

### Architecture notes for step 44

For the `transcode_view` closure (option 1), the cleanest design is:

```cpp
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
inline constexpr transcode_closure<From, To> transcode_view{};
```

This yields `whatwg_encode_view<To, whatwg_decode_view<From, all_t<R>>>` as
the composed view — zero overhead, no intermediate allocation, lazily
evaluated.

The header would go in `include/beman/transcode/detail/transcode_view.hpp`
and be added to the umbrella header.

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

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_STRING_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files for context

- `include/beman/transcode/transcode.hpp` — umbrella header (step 42+43)
- `include/beman/transcode/detail/transcode_string.hpp` — step 43 (new)
- `include/beman/transcode/detail/concepts.hpp` — `legacy_byte_range`, `unicode_scalar_range`
- `include/beman/transcode/detail/sniff.hpp` — BOM detection (step 41)
- `include/beman/transcode/detail/labels.hpp` — label→codec lookup (step 40)
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view + `codec` enum + `whatwg_decode` pipe adapter
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view + `whatwg_encode` pipe adapter
- `tests/beman/transcode/transcode_string.test.cpp` — step 43 test style reference
- `tests/beman/transcode/transcode.test.cpp` — step 42 test style reference (umbrella header usage)
