# Handoff: beman.transcode — Step 51

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

**Step 50 complete and merged to main.**

589 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in `papers/wg21/` (Python ruff/codespell
issues unrelated to the transcode C++ library). All C++, CMake, and
`tools/` Python lint passes cleanly.

### What Step 50 Added

Iconv view boundary-condition tests derived from `docs/iconv-testing.md`.

1. **New mocks** in `tests/beman/transcode/iconv_mock.hpp`:
   - `mock_iconv_partial_consume` — consumes 1 byte, writes 1, returns
     EINVAL. Exercises the staging-shift loop when iconv partially
     consumes the staging buffer.
   - `mock_iconv_e2big_zero_output` — always returns E2BIG without
     writing output. Exercises the skip-byte + output_full error path
     in `_or_error`.

2. **Mock-based tests** in `iconv_transcode.test.cpp` and
   `iconv_transcode_or_error.test.cpp`:
   - EILSEQ with non-`_or_error` view (skip bad bytes + terminate)
   - Partial staging consume (staging shift after partial iconv progress)
   - E2BIG with zero output → `output_full` error in `_or_error`
   - Partial consume with `_or_error` (yields output then continues)

3. **Real-iconv tests** in `iconv_real.test.cpp`:
   - Scenario A: 100 'A' chars → UTF-32LE with 4-byte buffer (forces
     repeated E2BIG, verifies seamless resumption)
   - Scenario B: U+1D11E (𝄞, 4-byte UTF-8) → UTF-32LE with 4-byte
     buffer (split multi-byte, forces EINVAL accumulation)

Coverage: `iconv_transcode_view.hpp` 84.8% → 94.9%;
`iconv_transcode_or_error_view.hpp` 83.7% → 92.9%.

### Feature gap noted

Scenario D from `iconv-testing.md` (end-of-range stateful flush) is a
**feature gap** — the current views do NOT call `iconv(cd, nullptr, ...)`
at end-of-input to flush shift-state bytes. This is needed for stateful
encodings like ISO-2022-JP through real iconv. A future step should:
1. Add a flush call (inbuf=nullptr) when input is exhausted
2. Add a `mock_iconv_flush` that produces output on flush
3. Add a real-iconv test with ISO-2022-JP

## What To Do Next — Step 51

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–50 are complete. The checklist does not yet have a step 51 entry.

### Coverage status

```
lines:     84.9% (7919/9332)
functions: 99.9% (2037/2038)
branches:  59.0% (2927/4965)
```

Remaining coverage gaps:

| File | Coverage | Uncovered |
|------|----------|-----------|
| `whatwg_encode_view.hpp` | 92.0% (332/361) | 29 lines |
| `iconv_transcode_or_error_view.hpp` | 92.9% (91/98) | 7 lines |
| `iconv_transcode_view.hpp` | 94.9% (75/79) | 4 lines |
| `gb18030.hpp` | 93.3% (111/119) | 8 lines |
| `euc_jp.hpp` | 92.9% (78/84) | 6 lines |
| `whatwg_decode_view.hpp` | 97.1% (631/650) | 19 lines (dead code) |

### Recommended options for step 51

**Option A: iconv stateful flush** — implement the end-of-range flush
described in `docs/iconv-testing.md` scenario D. This is new feature
work: add a flush call (`iconv(cd, nullptr, ...)`) when the input is
exhausted, to handle stateful encodings properly. Includes mock + real
iconv tests.

**Option B: `whatwg_encode_view.hpp` coverage** — the largest remaining
coverage gap (29 lines). Likely `_or_error` dispatch arms and specific
encoder error paths for CJK codecs.

**Option C: `gb18030.hpp` + `euc_jp.hpp` coverage** — smaller but
targeted coverage improvements for codec helper functions.

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
```

## Coding Rules (abbreviated)

- Include guard: mirrors file path, e.g. `INCLUDE_BEMAN_TRANSCODE_DETAIL_LABELS_HPP`
- Test file: include the primary header **twice** (idempotent check)
- Functions: out-of-line in headers with full `ClassName::method_name` qualification
- `constexpr` everything that can be; add `constify()` consteval test
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Key files for context

- `include/beman/transcode/iconv_transcode_view.hpp` — iconv view (94.9%)
- `include/beman/transcode/iconv_transcode_or_error_view.hpp` — iconv or_error view (92.9%)
- `tests/beman/transcode/iconv_mock.hpp` — mock iconv functions (4 mocks)
- `tests/beman/transcode/iconv_transcode.test.cpp` — iconv view tests
- `tests/beman/transcode/iconv_transcode_or_error.test.cpp` — or_error tests
- `tests/beman/transcode/iconv_real.test.cpp` — real iconv integration tests
- `docs/iconv-testing.md` — boundary-condition test design document
- `include/beman/transcode/whatwg_encode_view.hpp` — encode view (next coverage target)
