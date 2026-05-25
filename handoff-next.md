# Handoff: beman.transcode — Step 54

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

**Step 53 complete and merged to main.**

608 C++ tests + 171 Python tests pass (`make test`). `make lint` has
pre-existing failures only in docs (codespell: "implementor" in
`docs/Rust Encoding for C++ Transcode.md`, "bu" in
`papers/wg21/generated/TEST.html`). All C++, CMake, tools, and ruff
lint passes cleanly (papers/wg21 now excluded from ruff via
`extend-exclude` in pyproject.toml).

### What Step 53 Added

Coverage improvement tests targeting three files:

**`whatwg_encode_view.hpp`**: 92.0% → 100.0%
- ISO-2022-JP encoder: consecutive YEN/OVERLINE (already in Roman state
  → no ESC sequence emitted, just the byte)
- ISO-2022-JP encoder: unmapped codepoint while in JIS state (ESC(B
  switch back + '?')
- ISO-2022-JP `_or_error` variant: all the same transitions (YEN,
  consecutive YEN+OVERLINE, ASCII-after-JIS, JIS-after-JIS)

**`euc_jp.hpp`**: 92.9% → 98.8%
- SS3 (0x8F) alone → truncated_sequence
- SS3 + invalid b1 / invalid b2 → invalid_byte
- SS3 unmapped JIS X 0212 pointer 0 / unmapped JIS X 0208 pointer 108
- Remaining 1 uncovered line (line 50): dead code — view checks
  `current_ == end_` before calling `euc_jp_decode_one`

**`gb18030.hpp`**: 93.3% → 95.0%
- U+E7C7 encode: exercises `gb18030_ranges_encode` special case
- U+0080 encode: exercises 4-byte range encoding binary search
- Remaining 6 uncovered lines are all dead code:
  - Lines 70, 72: pointer pre-check on line 51 prevents reaching these
  - Line 76: binary search can't fail (range 0 starts at pointer 0)
  - Line 97: gb18030 covers all Unicode (encode can't fall through)
  - Line 103: view checks before calling decode
  - Line 172: GBK table has no zero entries

Also excluded `papers/wg21` from ruff lint in pyproject.toml.

### Coverage summary

Overall: 85.4% lines, 99.9% functions.

Remaining meaningful coverage gaps (not dead code):
- `whatwg_decode_view.hpp`: some paths in GBK/GB18030 replay logic
  and ISO-2022-JP state machine
- `iconv_transcode_view.hpp`: ~5% uncovered (error paths in iconv
  interaction)
- `iconv_transcode_or_error_view.hpp`: ~7% uncovered

## What To Do Next — Step 54

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

Steps 0–53 are complete. The checklist does not yet have a step 54 entry.

### Recommended options for step 54

**Option A: C++23 module support update** — ensure `transcode.cppm`
exports all headers added since step 42 (umbrella header). Run:
```bash
grep '#include' include/beman/transcode/transcode.hpp
```
and compare with what's exported in `transcode.cppm`. Any missing
includes = missing module exports.

**Option B: `whatwg_decode_view.hpp` remaining coverage** — the replay
logic for GB18030/GBK and some ISO-2022-JP state machine paths still
have gaps. Run `make coverage` and inspect the JSON for details.

**Option C: Begin Phase 3 (benchmarking)** — plan docs exist in
`docs/plans/phase3-*.md`. Step p3-step1 sets up the benchmark harness
using Google Benchmark.

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

- `include/beman/transcode/whatwg_encode_view.hpp` — encode view (now 100% covered)
- `include/beman/transcode/whatwg_decode_view.hpp` — decode view (next coverage target)
- `include/beman/transcode/detail/gb18030.hpp` — GB18030 codec (dead code noted)
- `include/beman/transcode/detail/euc_jp.hpp` — EUC-JP codec (dead code noted)
- `pyproject.toml` — ruff config (papers/wg21 now excluded)
- `docs/plans/phase3-index.md` — Phase 3 benchmarking overview
