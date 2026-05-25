# Handoff: beman.transcode — Step 49

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

**Step 48 complete and merged to main.**

559 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in `papers/wg21/` (Python ruff/codespell
issues unrelated to the transcode C++ library). All C++, CMake, and
`tools/` Python lint passes cleanly.

### What Step 48 Added

1. **Negative compile test** for `sniff_encoding` rejecting `char32_t` ranges:
   - `tests/beman/transcode/sniff_reject_char32_range_fail.cpp` — tries
     to call `sniff_encoding(vector<char32_t>)`, expects
     `PASS_REGULAR_EXPRESSION "legacy_byte_range"`.
   - Registered in `tests/beman/transcode/CMakeLists.txt`.

2. **Python lint fixes** in `tools/generate_labels.py`:
   - Shortened module docstring (E501).
   - Split `parse_encodings_json` docstring across two lines (E501).
   - Replaced `try/except FileNotFoundError/pass` with
     `contextlib.suppress(FileNotFoundError)` (SIM105).

3. **Python lint fix** in `tools/tests/test_generate_labels.py`:
   - Wrapped long `ENCODINGS_JSON` path assignment (E501).

### Note on the previous handoff

The previous handoff-next.md recommended "step 48: `labels.hpp` and
`sniff.hpp` consteval tests", but those consteval tests were already
on main — they were added as part of steps 40 and 41. Step 48 was
therefore redefined as the above negative compile test + lint fixes.

## What To Do Next — Step 49

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–48 are complete. The checklist does not yet have a step 49 entry.

### Coverage status

```
lines:     83.4% (7705/9238)
functions: 99.9% (2026/2027)
```

Lowest-coverage files (library headers only):

| File | Coverage | Uncovered |
|------|----------|-----------|
| `whatwg_decode_view.hpp` | 79.7% (518/650) | 132 lines |
| `iconv_transcode_or_error_view.hpp` | 83.7% (82/98) | 16 lines |
| `iconv_transcode_view.hpp` | 84.8% (67/79) | 12 lines |
| `whatwg_encode_view.hpp` | 92.0% (332/361) | 29 lines |
| `gb18030.hpp` | 89.1% (106/119) | 13 lines |
| `transcode_string.hpp` | 99.2% (260/262) | 2 lines |

### Recommended step 49: `whatwg_decode_view` coverage improvement

`whatwg_decode_view.hpp` at 79.7% has the biggest coverage gap. The
uncovered lines are likely in codec-specific `load()` branches that
aren't exercised by existing tests (e.g., specific error paths in
multi-byte decoders, or rarely-triggered state transitions).

Steps:
1. Run `make coverage` and inspect the HTML report at
   `.build/build-system/coverage/` to identify which specific lines
   in `whatwg_decode_view.hpp` are uncovered.
2. Write targeted test cases in existing test files (e.g.,
   `whatwg_decode.test.cpp`, `big5_decode.test.cpp`, etc.) that
   exercise those code paths.
3. Focus on meaningful coverage — don't fight the compiler for
   template instantiation artifacts or platform-specific branches.

### Alternative step 49 options

- **`whatwg_encode_view.hpp` coverage** (92.0%) — similar analysis,
  identify and test uncovered encode paths.
- **`iconv_transcode_view.hpp` + `iconv_transcode_or_error_view.hpp`
  coverage** — these use the system iconv library, so coverage depends
  on platform iconv availability.
- **Module support** — `transcode.cppm` is mentioned in CLAUDE.md but
  may not be fully wired up with all the new headers.

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

- `include/beman/transcode/whatwg_decode_view.hpp` — decode view (biggest coverage gap)
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view
- `include/beman/transcode/detail/transcode_view.hpp` — `transcode_closure<From,To>` pipe helper
- `include/beman/transcode/detail/transcode_string.hpp` — one-shot function
- `include/beman/transcode/detail/sniff.hpp` — BOM detection (constexpr)
- `include/beman/transcode/detail/labels.hpp` — label lookup (generated, constexpr)
- `tests/beman/transcode/CMakeLists.txt` — test registration + negative compile tests
- `tests/beman/transcode/sniff_reject_char32_range_fail.cpp` — step 48 negative compile test (pattern reference)
