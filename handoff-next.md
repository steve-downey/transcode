# Handoff: beman.transcode — Phase 3 (WPT Conformance)

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

**Phase 2 is complete.** 420 C++ tests + 102 Python tests pass (`make
test`). Steps 0–29 done. On `main`.

### What Phase 2 Built (Steps 14–29)

All WHATWG codecs are implemented with full decode **and** encode support:

- **Single-byte** (28 codecs): windows-1252, KOI8-R, KOI8-U,
  ISO-8859-2..16, IBM866, macintosh, windows-874, windows-1250..1258,
  x-mac-cyrillic
- **Multi-byte**: GBK, GB18030, Big5, Shift_JIS, EUC-JP (JIS X 0208 +
  0212), ISO-2022-JP (stateful), EUC-KR
- **Algorithmic**: UTF-8 (encode), UTF-16BE, UTF-16LE
- **Special**: `replacement` (decode-only), `x_user_defined`
- **Round-trip tests** (step 29): `encode_then_decode` and
  `decode_then_encode` for all bidirectional codecs

### Codec enum (in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8, replacement, x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    utf_16be, utf_16le,
    gbk, gb18030, big5, shift_jis, euc_jp, iso_2022_jp, euc_kr,
};
```

---

## What To Do Next — Phase 3

**Phase 3: WPT Conformance Testing**

The goal is to run the official W3C Web Platform Tests (WPT) encoding
test vectors through the library to verify bug-for-bug browser
compatibility.

### Existing Stubs (UNTRACKED — not in git yet)

`tests/whatwg/codec/tests/` contains aspirational drafts:
- `test_cases.hpp` — `DecodeTestCase` / `EncodeTestCase` structs
- `vectors.hpp` — a few hand-written test vectors
- `verify.hpp` — a template `verify_decoder_vectors` using a mock
  `TextDecoder` that doesn't exist yet
- `CMakeLists.txt` — a draft build description (not wired into the main
  CMakeLists.txt)
- `whatwg_indices.hpp` — sketch of C++26 `#embed` approach
- `generate_indices.py` — referenced but doesn't exist in `tools/`
- `whatwg_wpt_testing_plan.md` — the planning document

**These stubs are incomplete and unintegrated.** They describe an
aspirational C++26 `consteval`/`#embed` architecture, but the immediate
practical step is simpler: use the existing library APIs
(`whatwg_decode_view`, `whatwg_encode_view`) with WPT JSON test vectors
and Catch2, following the same patterns as Phase 2.

### The WPT Encoding Tests

The WPT encoding tests are at:
```
https://github.com/web-platform-tests/wpt/tree/master/encoding
```

Key files:
- `idna.json`, `legacy-mb-*.json`, `utf-8.json` — JSON test vectors
- Each vector: `{input: [...bytes...], output: [...codepoints...], encoding: "..."}`

The WHATWG index data is already downloaded (from step 17) at:
```
docs/whatwg/   (SOURCE.md and source.bib describe provenance)
data/tables/   (generated C++ tables)
```

### Recommended Next Step: Step 30 — WPT Test Vector Integration

**Branch:** `step30-wpt-vectors`

1. **Download WPT encoding test vectors** into `docs/wpt/` (pristine
   upstream, with SOURCE.md + source.bib for provenance, matching the
   pattern from step 17).

2. **Write a Python tool** `tools/run_wpt_vectors.py` or extend
   `tools/generate_tables.py` to extract WPT JSON decode/encode test
   vectors and run them against the library.

3. **Or**: Write a C++ test file `tests/beman/transcode/wpt.test.cpp`
   that hard-codes a representative selection of WPT vectors (the
   non-trivial ones that exercise error handling and edge cases), wired
   into ctest the same way as all Phase 2 tests.

4. **Wire into CMakeLists.txt** following the existing pattern.

### Alternative Next Step: API Polish / Beman Integration

If WPT conformance is not the priority, other candidates:
- `beman.transcode` CMake install targets (for downstream use)
- C++23 module interface (`transcode.cppm`) — already stubbed
- Benchmarks
- PR/issue cleanup for Beman project incubation review

---

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
```

## TDD Process (unchanged from Phase 2)

1. Branch: `git checkout -b step30-<slug>`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN) → `make test` + `make lint` → commit → push both
4. Merge to main: `git checkout main && git merge --no-ff step30-<slug>`
5. Push main to both remotes
6. Mark checklist `[x]`

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Test files: include the primary header **twice** (idempotent check)
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Phase 2 History Summary (for context)

Steps 14–29 added all WHATWG codecs. Each step followed: RED commit
(failing tests) → GREEN commit (implementation) → `make lint` → merge
to main. The checklist at `docs/plans/phase2-checklist.md` is fully
marked `[x]`.
