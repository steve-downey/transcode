# Handoff: beman.transcode — Step 50

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

**Step 49 complete and merged to main.**

583 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in `papers/wg21/` (Python ruff/codespell
issues unrelated to the transcode C++ library). All C++, CMake, and
`tools/` Python lint passes cleanly.

### What Step 49 Added

Coverage improvement for `whatwg_decode_view.hpp`: **79.7% → 97.1%**.
The remaining 19 uncovered lines are dead code (unreachable error
branches for fully-populated single-byte codec tables).

Added tests for:
- UTF-16 truncated high surrogate (3-byte input: high surrogate + only 1
  trailing byte) — both non-`_or_error` and `_or_error` variants
- GB18030 replay with high lead byte — error path and success path where
  the replayed lead byte forms a valid 2-byte GBK character
- ISO-2022-JP `_or_error` comprehensive coverage: EOS in all states
  (Escape_Start, Escape, Lead_Byte), invalid escape bytes with pending
  re-processing, SO/SI bytes, Roman/Katakana states, output_flag error
  on consecutive state transitions, BOM stripping

Overall coverage: lines 83.4% → 84.7%, branches 56.9% → 58.8%.

## What To Do Next — Step 50

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–49 are complete. The checklist does not yet have a step 50 entry.

### Coverage status

```
lines:     84.7% (7825/9236)
functions: 99.9% (2025/2026)
branches:  58.8% (2890/4917)
```

Remaining coverage gaps (library headers only):

| File | Coverage | Uncovered |
|------|----------|-----------|
| `whatwg_encode_view.hpp` | 92.0% (332/361) | 29 lines |
| `iconv_transcode_or_error_view.hpp` | 83.7% (82/98) | 16 lines |
| `iconv_transcode_view.hpp` | 84.8% (67/79) | 12 lines |
| `gb18030.hpp` | 93.3% (111/119) | 8 lines |
| `euc_jp.hpp` | 92.9% (78/84) | 6 lines |
| `whatwg_decode_view.hpp` | 97.1% (631/650) | 19 lines (dead code) |
| `transcode_string.hpp` | 99.2% (260/262) | 2 lines |

### Recommended step 50: `whatwg_encode_view.hpp` coverage improvement

`whatwg_encode_view.hpp` at 92.0% has 29 uncovered lines. These are
likely in:
- `_or_error` dispatch arms for codecs not tested in error mode
- Specific encoder error paths (unmapped codepoints in CJK codecs)
- State-machine paths in ISO-2022-JP encoder

Steps:
1. Run `make coverage` and inspect the JSON report to identify which
   lines in `whatwg_encode_view.hpp` are uncovered:
   ```python
   python3 -c "
   import json
   with open('.build/build-system/coverage.json') as f:
       data = json.load(f)
   for sf in data['source_files']:
       if 'whatwg_encode_view' in sf['name']:
           cov = sf['coverage']
           for i, c in enumerate(cov):
               if c is not None and c == 0:
                   print(f'  line {i+1}')
   "
   ```
2. Read the code at those lines and determine what inputs trigger them.
3. Write targeted test cases in existing test files (e.g.,
   `whatwg_encode.test.cpp`, `whatwg_encode_or_error.test.cpp`,
   codec-specific encode tests).

### Alternative step 50 options

- **`gb18030.hpp` + `euc_jp.hpp` coverage** — these codec helpers have
  specific decode/encode error paths that might be reachable with
  carefully crafted inputs.
- **`iconv_transcode_view.hpp` coverage** — uses system iconv, may need
  mocking or specific locale configuration.
- **Module support** — ensure `transcode.cppm` exports all new headers.

## TDD Process

Each step is a separate branch: `step<N>-<slug>`, branched from `main`.

1. Write failing tests (RED) → commit → push both remotes
2. Implement (GREEN) → `make test` (all pass) → `make lint` → commit → push both remotes
3. `git checkout main && git merge --no-ff step<N>-...` → push both remotes
4. Update `docs/plans/phase2-checklist.md` — mark completed items `[x]`

Note: For pure coverage-improvement steps (like step 49), there is no
RED/GREEN distinction — the tests pass immediately since the code
already exists. Just commit as GREEN directly.

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

- `include/beman/transcode/whatwg_encode_view.hpp` — encode view (next coverage target)
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view (97.1%, step 49 reference)
- `include/beman/transcode/detail/gb18030.hpp` — GB18030 codec (93.3%)
- `include/beman/transcode/detail/euc_jp.hpp` — EUC-JP codec (92.9%)
- `tests/beman/transcode/whatwg_encode.test.cpp` — encode tests
- `tests/beman/transcode/whatwg_encode_or_error.test.cpp` — encode or_error tests
- `tests/beman/transcode/iso2022jp_encode.test.cpp` — ISO-2022-JP encode tests
- `tests/beman/transcode/CMakeLists.txt` — test registration
