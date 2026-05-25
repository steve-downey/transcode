# Handoff: beman.transcode — Step 33 (next WPT coverage)

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

**Step 32 complete and merged to main.**

451 C++ tests + 127 Python tests pass (`make test`). `make lint` clean.

### What Step 32 Fixed

The ISO-2022-JP stateful decoder now fully conforms to the WHATWG spec.
All 34 WPT vectors in `tests/beman/transcode/wpt_iso2022jp_vectors.hpp`
pass (previously 12 of 34 passed).

Key fixes:
1. **Katakana byte range**: 0x21–0x5F → U+FF61–U+FF9F (was incorrectly
   0xA1–0xDF, the 8-bit JIS range; ISO-2022-JP uses 7-bit encoding)
2. **SO/SI handling**: 0x0E and 0x0F emit U+FFFD in all states
3. **`iso2022jp_output_state_`**: tracks where to return after errors
   (was always going to ASCII; now restores to last valid output state)
4. **`iso2022jp_output_flag_`**: detects redundant (same-state) ESC
   sequences and emits U+FFFD for them
5. **Escape end-of-stream**: prepend lead byte to stream, emit U+FFFD
6. **Invalid escape**: prepend lead + byte (2 bytes) to stream, emit U+FFFD
7. **Lead_Byte at end-of-stream**: finished cleanly (no extra U+FFFD)
8. **`pending_count_`**: replaces `has_pending_` (bool) with an int (0/1/2)
   to support 2-byte replay for escape error recovery

### Decoder Location

`include/beman/transcode/whatwg_decode_view.hpp` — two `load()` functions
(one for `whatwg_decode_view`, one for `whatwg_decode_or_error_view`),
both with `else if constexpr (C == codec::iso_2022_jp)` blocks using
a unified `while (true)` loop with a get-byte step + switch-on-state.

## What To Do Next — Step 33

**Branch:** `step33-<slug>`

### Option A: UTF-16 surrogates WPT vectors (recommended)

Download `textdecoder-utf16-surrogates.any.js` from the WPT repo and add
conformance tests for UTF-16LE/BE surrogate handling. The decoder handles
surrogates in the `pending_count_`/`pending_[2]` mechanism already.

```
docs/wpt/                    ← store pristine WPT JS file here
data/                        ← store generated C++ vectors here
tests/beman/transcode/       ← add wpt_utf16_surrogates.test.cpp
```

WPT URL: `https://github.com/web-platform-tests/wpt/blob/master/encoding/textdecoder-utf16-surrogates.any.js`

### Option B: Fatal mode WPT vectors

Download `textdecoder-fatal.any.js` from WPT and verify the
`whatwg_decode_or_error<codec::...>` variant correctly reports errors
(instead of replacing with U+FFFD) for invalid sequences.

### Option C: Checklist step 30+ (WPT remaining encodings)

`docs/plans/phase2-checklist.md` shows Steps 14–29 complete but the
checklist hasn't been updated for steps 30–32. Update the checklist
and add entries for steps 30–33.

## TDD Process

1. Branch: `git checkout -b step33-<slug>`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN) → `make test` + `make lint` → commit → push both
4. Merge to main: `git checkout main && git merge --no-ff step33-<slug>`
5. Push main to both remotes

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
```

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Test files: include the primary header **twice** (idempotent check)
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`
