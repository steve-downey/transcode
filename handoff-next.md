# Handoff: beman.transcode — Step 53

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

**Step 52 complete and merged to main.**

595 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in `papers/wg21/` (Python ruff/codespell
issues unrelated to the transcode C++ library). All C++, CMake, and
`tools/` Python lint passes cleanly.

### What Step 52 Added

Real-iconv integration tests for ISO-2022-JP stateful flush (validating
the flush feature added in step 51).

Tests in `tests/beman/transcode/iconv_real.test.cpp`:
- **あ (U+3042) → ISO-2022-JP**: input `E3 81 82` produces
  `1B 24 42 24 22 1B 28 42` — the ESC $B switch-to-JIS, encoded char,
  and the flush-produced ESC (B switch-back-to-ASCII.
- **Mixed "Aあ"**: verifies correct escape insertion between ASCII and
  JIS segments.
- **or_error variant**: same あ test through `iconv_transcode_or_error`,
  confirming all bytes are `has_value()`.
- **Small buffer (3 bytes)**: forces repeated E2BIG during the
  conversion plus flush, verifying no bytes are lost.

### iconv testing summary (steps 50–52)

All four scenarios from `docs/iconv-testing.md` are fully implemented
and integration-tested:
- A: E2BIG with real iconv (100 chars, 4-byte buffer)
- B: EINVAL split multi-byte (U+1D11E, 4-byte buffer)
- C: EILSEQ (mock: skip-byte + terminate)
- D: Stateful flush (mock: shift-in byte; real: ISO-2022-JP ESC sequence)

## What To Do Next — Step 53

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–52 are complete. The checklist does not yet have a step 53 entry.

### Recommended options for step 53

**Option A: `whatwg_encode_view.hpp` coverage** — the largest remaining
coverage gap (29 uncovered lines at 92.0%). Run:
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
Then read those lines to understand what paths are untested and write
targeted tests.

**Option B: `gb18030.hpp` + `euc_jp.hpp` coverage** — smaller codec
helper coverage improvements (8 + 6 uncovered lines).

**Option C: module support** — ensure `transcode.cppm` exports all
headers added since step 42.

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

- `include/beman/transcode/whatwg_encode_view.hpp` — encode view (next coverage target)
- `include/beman/transcode/iconv_transcode_view.hpp` — iconv view (with flush)
- `include/beman/transcode/iconv_transcode_or_error_view.hpp` — iconv or_error (with flush)
- `tests/beman/transcode/iconv_real.test.cpp` — real iconv integration tests (10 tests)
- `tests/beman/transcode/iconv_mock.hpp` — mock iconv functions (6 mocks)
- `docs/iconv-testing.md` — boundary-condition test design document (all 4 scenarios done)
