# Handoff: beman.transcode — Step 36 (next WPT coverage)

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

**Step 35 complete and merged to main.**

462 C++ tests + 141 Python tests pass (`make test`). `make lint` clean.

### What Step 35 Added

WPT conformance tests from `textdecoder-byte-order-marks.any.js`. The
WHATWG spec requires BOM stripping: when a UTF-8, UTF-16LE, or UTF-16BE
stream starts with its matching BOM prefix (`EF BB BF`, `FF FE`, or
`FE FF`), the BOM is consumed and not emitted as a codepoint.

Key changes:
- Fixed `parse_js_string()` in `tools/generate_wpt_vectors.py` to
  combine `\uD800-\uDBFF` + `\uDC00-\uDFFF` surrogate pair escapes into
  supplementary codepoints (e.g. `𝄞` → U+1D11E). This was
  needed because the WPT BOM file's expected string contains supplementary
  characters expressed as JS surrogate pairs.
- Added `parse_bom_vectors()` + `render_bom_vectors_hpp()` to generator
- Added 6 new Python tests (4 BOM + 2 surrogate pair combining)
- Generated `wpt_bom_vectors.hpp` (3 cases: utf-8, utf-16le, utf-16be)
- Implemented BOM stripping in both iterator constructors: after the first
  `load()`, if the first codepoint is U+FEFF for `codec::utf_8`,
  `codec::utf_16le`, or `codec::utf_16be`, call `load()` again to skip it.
- 7 C++ test cases covering no-BOM, with-BOM, and mismatching-BOM for all
  three encodings.

New files:
- `docs/wpt/textdecoder-byte-order-marks.any.js` — pristine WPT JS
- `tests/beman/transcode/wpt_bom_vectors.hpp` — 3 generated cases
  (struct `WptBomCase` with `bytes`, `bom`, `expected`, `encoding`)
- `tests/beman/transcode/wpt_bom.test.cpp` — 7 test cases

### WPT Files in `docs/wpt/`

| File | What it covers |
|------|----------------|
| `gb18030-decoder.any.js` | GB18030 multi-byte decode |
| `iso-2022-jp-decoder.any.js` | ISO-2022-JP stateful decode |
| `single-byte-decoder.window.js` | All 27 single-byte codec tables |
| `textdecoder-mistakes.any.js` | UTF-8 invalid sequences |
| `textdecoder-utf16-surrogates.any.js` | UTF-16LE surrogate handling |
| `textdecoder-fatal.any.js` | Fatal mode: all encodings, invalid inputs |
| `textdecoder-byte-order-marks.any.js` | BOM stripping for UTF-8/16LE/16BE |

## What To Do Next — Step 36

**Branch:** `step36-<slug>`

### Option A: `textdecoder-fatal-single-byte.any.js` WPT vectors (recommended)

Download and parse `textdecoder-fatal-single-byte.any.js` from WPT. It
tests fatal mode for single-byte encodings with invalid bytes. This would
verify our `whatwg_decode_or_error` implementation for all 27+ single-byte
codecs.

Fetch:
```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/textdecoder-fatal-single-byte.any.js" \
  --jq '.content' | base64 -d
```

Parse the file structure (likely similar to `textdecoder-fatal.any.js`).
Generate `wpt_fatal_single_byte_vectors.hpp`. Write a test that verifies
every invalid byte for each single-byte encoding yields at least one
`std::unexpected` from `whatwg_decode_or_error`.

### Option B: `textdecoder-ignorebom.any.js` WPT vectors

Tests the `ignoreBOM` option (which preserves the BOM in the output).
Our library doesn't expose an `ignoreBOM` option — it always strips the
leading BOM. This step would either implement `ignoreBOM` as a template
parameter, or document the gap.

The relevant WPT file exists in the WPT repo:
```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/textdecoder-ignorebom.any.js" \
  --jq '.content' | base64 -d
```

### Option C: `textdecoder-streaming.any.js` gap documentation

Tests streaming decode (`stream: true` option). Our library exposes a
range/view API rather than a streaming API. This step would document the
gap and add `// FIXME: streaming not yet supported` notes.

## TDD Process

1. Branch: `git checkout -b step36-<slug>`
2. Write failing tests (RED) -> commit -> push both remotes
3. Implement (GREEN) -> `make test` + `make lint` -> commit -> push both
4. Merge to main: `git checkout main && git merge --no-ff step36-<slug>`
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
5. Add Python tests to `tools/tests/test_generate_wpt.py`

**Important:** `parse_js_string()` now correctly combines `\uHHHH\uLLLL`
surrogate pairs into supplementary codepoints (fixed in step 35). Keep
this in mind when writing new tests that involve supplementary characters.

All WPT JS files go in `docs/wpt/` with provenance in `SOURCE.md`.
All generated C++ headers go in `tests/beman/transcode/`.

## Key Codec Names

The `codec` enum is in `include/beman/transcode/whatwg_decode_view.hpp`:
- `codec::utf_8`, `codec::utf_16le`, `codec::utf_16be`
- `codec::gbk`, `codec::gb18030`, `codec::big5`
- `codec::shift_jis`, `codec::euc_jp`, `codec::iso_2022_jp`
- `codec::euc_kr`
- All single-byte codecs: `codec::ibm866`, `codec::iso_8859_2`, etc.
