# Handoff: beman.transcode — Step 35 (next WPT coverage)

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

**Step 34 complete and merged to main.**

455 C++ tests + 135 Python tests pass (`make test`). `make lint` clean.

### What Step 34 Added

WPT fatal-mode conformance tests from `textdecoder-fatal.any.js`. The
file has a `bad[]` array of input byte sequences that must produce an
error (TypeError) in `TextDecoder({fatal: true})`. In our library this
maps to `whatwg_decode_or_error<codec::X>` yielding at least one
`std::unexpected` result.

34 vectors parsed from the WPT file:
- 33 UTF-8 vectors (invalid bytes, overlong encodings, surrogates)
- 1 UTF-16LE vector (`[0x00]` — truncated code unit)

All 34 vectors pass: every invalid input yields at least one error.

New files:
- `docs/wpt/textdecoder-fatal.any.js` — pristine WPT JS
- `tests/beman/transcode/wpt_fatal_vectors.hpp` — 34 generated vectors
  (struct `WptFatalVector` with `input` + `description`, two arrays:
  `utf8_fatal_wpt_vectors[]` and `utf16le_fatal_wpt_vectors[]`)
- `tests/beman/transcode/wpt_fatal.test.cpp` — 2 test cases
- `tools/generate_wpt_vectors.py` — added `parse_fatal_vectors()` +
  `render_fatal_vectors_hpp()`
- `tools/tests/test_generate_wpt.py` — added 4 new Python tests

### WPT Files in `docs/wpt/`

| File | What it covers |
|------|----------------|
| `gb18030-decoder.any.js` | GB18030 multi-byte decode |
| `iso-2022-jp-decoder.any.js` | ISO-2022-JP stateful decode |
| `single-byte-decoder.window.js` | All 27 single-byte codec tables |
| `textdecoder-mistakes.any.js` | UTF-8 invalid sequences |
| `textdecoder-utf16-surrogates.any.js` | UTF-16LE surrogate handling |
| `textdecoder-fatal.any.js` | Fatal mode: all encodings, invalid inputs |

## What To Do Next — Step 35

**Branch:** `step35-<slug>`

### Option A: `textdecoder-utf16-bom-option.any.js` WPT vectors (recommended)

Download and parse `textdecoder-utf16-bom-option.any.js` from WPT. It
tests BOM-stripping behavior for UTF-16 streams.

```
WPT URL: https://raw.githubusercontent.com/web-platform-tests/wpt/master/encoding/textdecoder-utf16-bom-option.any.js
```

The WHATWG spec strips a BOM at the start of a UTF-16 stream (U+FEFF as
the first code unit) and uses it to determine byte order. Our current
UTF-16 decoders (`codec::utf_16le`, `codec::utf_16be`) do not implement
BOM detection/stripping — this WPT file will reveal exactly what the gap
is.

Parse the file structure (likely `test()` calls or an array similar to
previous WPT files). Determine what our decoder actually produces for
BOM-prefixed inputs and write tests accordingly. If BOM stripping is
needed, implement it; if not, document why.

### Option B: `textdecoder-streaming.any.js` WPT vectors

Tests streaming decode (multiple calls to `decode()` with `stream: true`).
Our library doesn't currently expose a streaming API, so this would be
mostly informational — documenting the gap.

### Option C: Expand fatal mode test coverage to legacy encodings

The WPT `textdecoder-fatal.any.js` has a `// FIXME: Add legacy encoding
cases` comment. We could add our own tests covering fatal mode for
single-byte codecs, GB18030, EUC-KR, etc. — verifying that invalid byte
sequences produce errors in `_or_error` variants for every codec we
implement.

## TDD Process

1. Branch: `git checkout -b step35-<slug>`
2. Write failing tests (RED) -> commit -> push both remotes
3. Implement (GREEN) -> `make test` + `make lint` -> commit -> push both
4. Merge to main: `git checkout main && git merge --no-ff step35-<slug>`
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
5. Add 4 Python tests to `tools/tests/test_generate_wpt.py`

All WPT JS files go in `docs/wpt/` with provenance in `SOURCE.md`.
All generated C++ headers go in `tests/beman/transcode/`.

## Key Codec Names

The `codec` enum is in `include/beman/transcode/whatwg_decode_view.hpp`:
- `codec::utf_8`, `codec::utf_16le`, `codec::utf_16be`
- `codec::gbk`, `codec::gb18030`, `codec::big5`
- `codec::shift_jis`, `codec::euc_jp`, `codec::iso_2022_jp`
- `codec::euc_kr`
- All single-byte codecs: `codec::ibm866`, `codec::iso_8859_2`, etc.
