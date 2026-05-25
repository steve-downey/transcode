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

**Branch:** `step36-wpt-fatal-single-byte`

### WPT file structure

`textdecoder-fatal-single-byte.any.js` contains a `singleByteEncodings`
array of `{encoding, bad}` objects. The `bad` array lists byte values
(0x80–0xFF) that are null entries in the codec's table — they must throw
`TypeError` in fatal mode. All 256 bytes are tested; bytes not in `bad`
must decode without error.

```javascript
var singleByteEncodings = [
    {encoding: 'IBM866',     bad: []},
    {encoding: 'ISO-8859-2', bad: []},
    {encoding: 'ISO-8859-3', bad: [0xA5, 0xAE, 0xBE, 0xC3, 0xD0, 0xE3, 0xF0]},
    // ...
    {encoding: 'ISO-8859-8', bad: [0xA1, 0xBF, 0xC0, ...]},
    // ... 27 codecs total
];
```

Key observations:
- Most codecs have an **empty** `bad` list (IBM866, ISO-8859-2/4/5,
  ISO-8859-10/13/14/15/16, KOI8-R/U, macintosh, windows-1250/51/52/54/56/58,
  x-mac-cyrillic). Every byte decodes successfully for these.
- Bytes 0x00–0x7F are **never** in any `bad` list (ASCII passthrough).
- ISO-8859-8 and ISO-8859-8-I have identical `bad` lists.

Fetch the file via GitHub API:
```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/textdecoder-fatal-single-byte.any.js" \
  --jq '.content' | base64 -d > docs/wpt/textdecoder-fatal-single-byte.any.js
```

### WPT encoding name → `codec::` enum mapping

| WPT name       | `codec::` value     |
|----------------|---------------------|
| IBM866         | ibm866              |
| ISO-8859-2     | iso_8859_2          |
| ISO-8859-3     | iso_8859_3          |
| ISO-8859-4     | iso_8859_4          |
| ISO-8859-5     | iso_8859_5          |
| ISO-8859-6     | iso_8859_6          |
| ISO-8859-7     | iso_8859_7          |
| ISO-8859-8     | iso_8859_8          |
| ISO-8859-8-I   | iso_8859_8_i        |
| ISO-8859-10    | iso_8859_10         |
| ISO-8859-13    | iso_8859_13         |
| ISO-8859-14    | iso_8859_14         |
| ISO-8859-15    | iso_8859_15         |
| ISO-8859-16    | iso_8859_16         |
| KOI8-R         | koi8_r              |
| KOI8-U         | koi8_u              |
| macintosh      | macintosh           |
| windows-874    | windows_874         |
| windows-1250   | windows_1250        |
| windows-1251   | windows_1251        |
| windows-1252   | windows_1252        |
| windows-1253   | windows_1253        |
| windows-1254   | windows_1254        |
| windows-1255   | windows_1255        |
| windows-1256   | windows_1256        |
| windows-1257   | windows_1257        |
| windows-1258   | windows_1258        |
| x-mac-cyrillic | x_mac_cyrillic      |

### Generator approach (`generate_wpt_vectors.py`)

Add to the generator:

```python
_SBFATAL_RE = re.compile(
    r"\{encoding:\s*'([^']+)'\s*,\s*bad:\s*\[([^\]]*)\]\s*\}",
    re.MULTILINE,
)

def parse_fatal_single_byte_cases(content: str) -> list[dict[str, object]]:
    cases: list[dict[str, object]] = []
    for m in _SBFATAL_RE.finditer(content):
        encoding, bad_str = m.groups()
        bad = [int(x.strip(), 0) for x in bad_str.split(",") if x.strip()]
        cases.append({"encoding": encoding, "bad": bad})
    return cases
```

Generated header struct (`WptFatalSingleByteCase`):

```cpp
struct WptFatalSingleByteCase {
    const char*          encoding;
    std::vector<uint8_t> bad;  // bytes that must produce an error
};
inline const WptFatalSingleByteCase wpt_fatal_single_byte_cases[] = {
    {"IBM866",     {}},
    {"ISO-8859-2", {}},
    {"ISO-8859-3", {0xA5, 0xAE, 0xBE, 0xC3, 0xD0, 0xE3, 0xF0}},
    // ...
};
```

### C++ test approach (`wpt_fatal_single_byte.test.cpp`)

The test must dispatch at compile time to the right `codec::` value.
The simplest pattern: write a templated helper, call it once per codec:

```cpp
template <codec C>
void check_fatal_single_byte(const WptFatalSingleByteCase& c) {
    for (int b = 0; b < 256; ++b) {
        char ch = static_cast<char>(static_cast<uint8_t>(b));
        std::array<char, 1> input{ch};
        bool has_error = false;
        for (auto r : input | whatwg_decode_or_error<C>)
            if (!r.has_value()) { has_error = true; break; }
        bool expect_error = std::ranges::contains(c.bad,
                                                  static_cast<uint8_t>(b));
        INFO("encoding=" << c.encoding << " byte=0x" << std::hex << b);
        CHECK(has_error == expect_error);
    }
}

TEST_CASE("WPT fatal single-byte: IBM866", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::ibm866>(
        beman::transcoding::tests::wpt::wpt_fatal_single_byte_cases[0]);
}
// ... one TEST_CASE per codec (27 total)
```

This produces 27 test cases, each running 256 sub-checks.

### Option B: `textdecoder-ignorebom.any.js` WPT vectors

Tests the `ignoreBOM` option (which preserves the BOM in the output).
Our library doesn't expose an `ignoreBOM` option — it always strips the
leading BOM. This step would either implement `ignoreBOM` as a template
parameter, or document the gap.

Fetch:
```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/textdecoder-ignorebom.any.js" \
  --jq '.content' | base64 -d
```

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
