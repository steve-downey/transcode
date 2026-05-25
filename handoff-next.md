# Handoff: beman.transcode — Step 34 (next WPT coverage)

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

**Step 33 complete and merged to main.**

453 C++ tests + 131 Python tests pass (`make test`). `make lint` clean.

### What Step 33 Added

WPT conformance tests for UTF-16LE surrogate handling from
`textdecoder-utf16-surrogates.any.js`. All 5 vectors pass:

1. `[0x00, 0xD8]` → `[U+FFFD]` — lone surrogate lead (high surrogate at EOS)
2. `[0x00, 0xDC]` → `[U+FFFD]` — lone surrogate trail
3. `[0x00, 0xD8, 0x00, 0x00]` → `[U+FFFD, U+0000]` — unmatched surrogate lead
4. `[0x00, 0xDC, 0x00, 0x00]` → `[U+FFFD, U+0000]` — unmatched surrogate trail
5. `[0x00, 0xDC, 0x00, 0xD8]` → `[U+FFFD, U+FFFD]` — swapped surrogate pair

Two test cases per vector: normal decode (U+FFFD replacement) and
or_error decode (at least one error in output, verifying fatal mode).

New files:
- `docs/wpt/textdecoder-utf16-surrogates.any.js` — pristine WPT JS
- `tests/beman/transcode/wpt_utf16_surrogates_vectors.hpp` — 5 generated vectors
- `tests/beman/transcode/wpt_utf16_surrogates.test.cpp` — 2 test cases
- `tools/generate_wpt_vectors.py` — added `parse_utf16_surrogate_vectors()` + renderer
- `tools/tests/test_generate_wpt.py` — added 4 new Python tests

### WPT Files in `docs/wpt/`

| File | What it covers |
|------|---------------|
| `gb18030-decoder.any.js` | GB18030 multi-byte decode |
| `iso-2022-jp-decoder.any.js` | ISO-2022-JP stateful decode |
| `single-byte-decoder.window.js` | All 27 single-byte codec tables |
| `textdecoder-mistakes.any.js` | UTF-8 invalid sequences |
| `textdecoder-utf16-surrogates.any.js` | UTF-16LE surrogate handling |

## What To Do Next — Step 34

**Branch:** `step34-<slug>`

### Option A: `textdecoder-fatal.any.js` WPT vectors (recommended)

Download and parse `textdecoder-fatal.any.js` from WPT. It tests
that `TextDecoder` with `{fatal: true}` throws `TypeError` for
invalid byte sequences in all major codecs.

In our library, this maps to `whatwg_decode_or_error<codec::...>`
returning an error (`std::unexpected`) at each invalid position.

```
WPT URL: https://raw.githubusercontent.com/web-platform-tests/wpt/master/encoding/textdecoder-fatal.any.js
```

The file contains `fatal_tests` array with entries like:
```js
{ encoding: "utf-8",  input: [0xC0], description: "..." }
```
Each entry is an input that should produce an error in fatal mode.

New vector struct needed (just input + description, no expected output):
```hpp
struct WptFatalVector {
    std::vector<uint8_t> input;
    const char*          description;
};
```

Test: for each vector, `whatwg_decode_or_error<codec::X>` applied to
`input` must yield at least one `std::unexpected` result.

### Option B: `textdecoder-utf16-bom-option.any.js` WPT vectors

Tests BOM-stripping behavior for UTF-16. The WHATWG spec strips a BOM
at the start of a UTF-16 stream and uses it to determine byte order.
We don't currently implement BOM detection — this would reveal any gap.

### Option C: GB18030 2022 range update

The WHATWG spec was updated (2022) for GB18030 to handle the full
Unicode 13+ range. Check if our GB18030 decoder handles the new ranges.

## TDD Process

1. Branch: `git checkout -b step34-<slug>`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN) → `make test` + `make lint` → commit → push both
4. Merge to main: `git checkout main && git merge --no-ff step34-<slug>`
5. Push main to both remotes
6. Update `docs/plans/phase2-checklist.md`

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

## Python Tool Pattern

The generator `tools/generate_wpt_vectors.py` follows this pattern:
1. Add a `FOO_JS = WPT_DIR / "foo.any.js"` constant
2. Add `parse_foo_vectors(content: str) -> list[dict[str, object]]`
3. Add `render_foo_vectors_hpp(vectors, out_path)` 
4. Add both to `main()` + the clang-format list
5. Add 3-4 Python tests to `tools/tests/test_generate_wpt.py`

All WPT JS files go in `docs/wpt/` with provenance in `SOURCE.md`.
All generated C++ headers go in `tests/beman/transcode/`.
