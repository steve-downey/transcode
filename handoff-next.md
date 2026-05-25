# Handoff: beman.transcode — Step 43

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

**Step 42 complete and merged to main.**

521 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures in `papers/wg21/` and `tools/tests/test_generate_labels.py`
(Python ruff/codespell issues unrelated to the transcode C++ library). The
C++ and CMake portions of lint pass cleanly.

### What Step 42 Added

Umbrella header — `include/beman/transcode/transcode.hpp` now exposes all
public APIs through a single include.

Key changes:
- `include/beman/transcode/transcode.hpp` — updated to include:
  - `<beman/transcode/detail/null_term.hpp>`  (`views::null_term`, `null_term_view`)
  - `<beman/transcode/whatwg_decode_view.hpp>` (`whatwg_decode_view`, `whatwg_decode`, `codec`)
  - `<beman/transcode/whatwg_encode_view.hpp>` (`whatwg_encode_view`, `whatwg_encode`)
  - `<beman/transcode/detail/labels.hpp>`     (`get_encoding()`)
  - `<beman/transcode/detail/sniff.hpp>`      (`sniff_encoding()`)
- `tests/beman/transcode/transcode.test.cpp` — 7 runtime tests exercising
  all public APIs via the umbrella header (including null_term+decode pipe
  composition and consteval sniff_encoding)

Users can now do `#include <beman/transcode/transcode.hpp>` to get:
```cpp
using namespace beman::transcoding;
auto decoded = bytes | whatwg_decode<codec::utf_8>;       // char32_t range
auto encoded = codepoints | whatwg_encode<codec::utf_8>;  // char range
auto c = get_encoding("shift_jis");                       // codec lookup
auto s = sniff_encoding(data);                            // BOM detection
auto r = views::null_term(ptr);                           // C string range
```

## What To Do Next — Step 43

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–42 are complete. The checklist does not yet have a step 43 entry —
look at `docs/plans/phase2-index.md` for the phase overview to choose the
next logical step.

### Likely candidates for step 43

1. **`transcode_string` one-shot function** — a high-level convenience
   function that transcodes a byte string from one encoding to another,
   returning a `std::string`. Signature sketch:
   ```cpp
   // In namespace beman::transcoding
   // Returns the transcoded bytes, or std::nullopt on error
   std::string transcode_to_utf8(std::span<const char> src, codec from);
   std::string transcode_from_utf8(std::u32string_view src, codec to);
   // Or a combined version:
   std::string transcode(std::span<const char> src, codec from, codec to);
   ```
   This would compose `whatwg_decode_view` + `whatwg_encode_view` under
   the hood. The tricky part is defining the error-handling strategy
   (replacement character vs. throwing vs. returning expected).

2. **`transcode_view` pipe composition helper** — a combined decode+encode
   view that pipes `whatwg_decode` → `whatwg_encode`:
   ```cpp
   auto v = bytes | transcode<codec::utf_8, codec::shift_jis>;
   ```
   This would be a range adapter closure that composes the two views.

3. **More WPT conformance tests** — check if any WPT test files in
   `docs/wpt/` haven't been imported yet (e.g. encoder tests for
   non-UTF-8 codecs).

4. **`iconv_transcode_view` WPT integration** — use WPT vectors to
   validate the iconv-based transcoder against the same conformance
   data as the WHATWG native implementation.

### Checking for more WPT files

The WPT files in `docs/wpt/` are:
```
textdecoder-eof.any.js
textdecoder-fatal-single-byte.any.js
textdecoder-fatal-streaming.any.js
textdecoder-fatal.any.js
textdecoder-byte-order-marks.any.js
textdecoder-utf16-surrogates.any.js
api-surrogates-utf8.any.js
textencoder-utf16-surrogates.any.js
iso-2022-jp-decoder.any.js
single-byte-decoder.window.js
```
You can check the WPT encoding test suite at
`https://github.com/web-platform-tests/wpt/tree/master/encoding`
for any additional test files not yet imported.

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

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_SNIFF_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files for context

- `include/beman/transcode/transcode.hpp` — umbrella header (step 42)
- `include/beman/transcode/detail/concepts.hpp` — `legacy_byte_range`, `unicode_scalar_range`
- `include/beman/transcode/detail/sniff.hpp` — BOM detection (step 41)
- `include/beman/transcode/detail/labels.hpp` — label→codec lookup (step 40)
- `include/beman/transcode/whatwg_decode_view.hpp` — main decode view + `codec` enum + `whatwg_decode` pipe adapter
- `include/beman/transcode/whatwg_encode_view.hpp` — main encode view + `whatwg_encode` pipe adapter
- `tests/beman/transcode/transcode.test.cpp` — step 42 test style reference (umbrella header usage)
- `tests/beman/transcode/sniff.test.cpp` — step 41 test style reference
- `tests/beman/transcode/labels.test.cpp` — step 40 test style reference
