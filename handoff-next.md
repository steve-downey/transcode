# Handoff: beman.transcode — Step 38 (WPT UTF-8 surrogate encode vectors)

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

**Step 37 complete and merged to main.**

492 C++ tests + 151 Python tests pass (`make test`). `make lint` clean.

### What Step 37 Added

WPT EOF/truncation vectors from `textdecoder-eof.any.js` and a conformance
fix discovered by the tests.

Key changes:
- Downloaded `docs/wpt/textdecoder-eof.any.js` (10 non-streaming cases parsed)
  and `docs/wpt/textdecoder-fatal-streaming.any.js` (kept for reference;
  format is more complex, not yet parsed)
- Added `parse_eof_vectors()` + `render_eof_vectors_hpp()` to generator
- Generated `tests/beman/transcode/wpt_eof_vectors.hpp` (7 UTF-8 + 3 Big5 cases)
- **Big5 conformance fix**: WHATWG spec step 3.6 says if a Big5 trail byte
  encounters an error AND the trail is an ASCII byte, it must be re-processed
  (prepended to the input queue). Our implementation now returns the ASCII
  trail as `code_point2` in `big5_decode_result`, and both view `load()`
  functions propagate it via `pending_cp_` / `has_pending_cp_`. Two existing
  Big5 tests updated: `[0x81, 0x40]` now correctly yields U+FFFD + U+0040.

New files:
- `docs/wpt/textdecoder-eof.any.js` — pristine WPT JS
- `docs/wpt/textdecoder-fatal-streaming.any.js` — pristine WPT JS (not yet parsed)
- `tests/beman/transcode/wpt_eof_vectors.hpp` — 10 WptEofVector entries
- `tests/beman/transcode/wpt_eof.test.cpp` — 2 TEST_CASEs (UTF-8 + Big5)

### WPT Files in `docs/wpt/`

| File | What it covers | Status |
|------|----------------|--------|
| `gb18030-decoder.any.js` | GB18030 multi-byte decode | integrated |
| `iso-2022-jp-decoder.any.js` | ISO-2022-JP stateful decode | integrated |
| `single-byte-decoder.window.js` | All 27 single-byte codec tables | integrated |
| `textdecoder-mistakes.any.js` | UTF-8 invalid sequences | integrated |
| `textdecoder-utf16-surrogates.any.js` | UTF-16LE surrogate handling | integrated |
| `textdecoder-fatal.any.js` | Fatal mode: all encodings | integrated |
| `textdecoder-byte-order-marks.any.js` | BOM stripping | integrated |
| `textdecoder-fatal-single-byte.any.js` | Fatal mode: bad bytes per codec | integrated |
| `textdecoder-eof.any.js` | EOF/truncation (non-streaming) | integrated |
| `textdecoder-fatal-streaming.any.js` | Fatal streaming (skipped) | downloaded |

## What To Do Next — Step 38

**Branch:** `step38-wpt-surrogates-utf8`

### WPT file: `api-surrogates-utf8.any.js`

This file is about the UTF-8 **encoder**: encoding JavaScript strings that
contain lone surrogates (U+D800–U+DFFF). Lone surrogates are not valid
Unicode scalar values; the WHATWG spec requires them to be replaced with
U+FFFD when encoding.

Fetch:
```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/api-surrogates-utf8.any.js" \
  --jq '.content' | base64 -d > docs/wpt/api-surrogates-utf8.any.js
```

### File structure

```javascript
var badStrings = [
    {
        input: 'abc123',
        expected: [0x61, 0x62, 0x63, 0x31, 0x32, 0x33],
        decoded: 'abc123',
        name: 'Sanity check'
    },
    {
        input: '\uD800',
        expected: [0xef, 0xbf, 0xbd],
        decoded: '�',
        name: 'Surrogate half (low)'
    },
    // ... 6 entries total
];
```

Fields:
- `input`: JS string → convert to `char32_t` codepoints using `parse_js_string()`
- `expected`: byte array → bytes produced by encoding `input` with UTF-8
- `decoded`: JS string → codepoints expected when those bytes are decoded back
- `name`: description string

**Key behavior for our library:**
- `whatwg_encode<codec::utf_8>` with a surrogate codepoint emits
  `0xEF, 0xBF, 0xBD` (U+FFFD in UTF-8) — already implemented (line 241-244
  of `whatwg_encode_view.hpp`)
- `whatwg_encode_or_error<codec::utf_8>` with a surrogate returns an error
  (`whatwg_error::surrogate_code_point`)

### Parser approach

```python
_SURR_ENTRY_RE = re.compile(
    r"\{[^}]*?input:\s*'((?:[^'\\]|\\.)*)'.*?"
    r"expected:\s*\[([^\]]*)\].*?"
    r"decoded:\s*'((?:[^'\\]|\\.)*)'.*?"
    r"name:\s*'((?:[^'\\]|\\.)*)'",
    re.DOTALL,
)

def parse_surrogates_utf8_vectors(content: str) -> list[dict[str, object]]:
    vectors: list[dict[str, object]] = []
    for m in _SURR_ENTRY_RE.finditer(content):
        input_str, bytes_str, decoded_str, name = m.groups()
        input_cps = parse_js_string(input_str)
        expected_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        decoded_cps = parse_js_string(decoded_str)
        vectors.append({
            "input": input_cps,
            "expected": expected_bytes,
            "decoded": decoded_cps,
            "name": name,
        })
    return vectors
```

**Note:** `parse_js_string` handles lone surrogates correctly — `\uD800` as
a standalone escape (not followed by a low surrogate) is returned as the
codepoint 0xD800.

### Generated header struct (`WptSurrogatesUtf8Vector`)

```cpp
struct WptSurrogatesUtf8Vector {
    std::vector<char32_t> input;    // codepoints to encode (may include surrogates)
    std::vector<uint8_t>  expected; // expected UTF-8 bytes
    std::vector<char32_t> decoded;  // expected codepoints when bytes are decoded
    const char*           name;
};
inline const WptSurrogatesUtf8Vector wpt_surrogates_utf8_vectors[] = { ... };
```

Guard: `TESTS_BEMAN_TRANSCODE_WPT_SURROGATES_UTF8_VECTORS_HPP`.

### C++ test file (`wpt_surrogates_utf8.test.cpp`)

```cpp
TEST_CASE("WPT surrogates UTF-8: encode produces correct bytes",
          "[wpt::surrogates_utf8]") {
    for (const auto& v : wpt_surrogates_utf8_vectors) {
        std::vector<char> got;
        for (char c : v.input | whatwg_encode<codec::utf_8>)
            got.push_back(c);
        std::vector<uint8_t> got_bytes(got.begin(), got.end());
        INFO("name=" << v.name);
        CHECK(got_bytes == v.expected);
    }
}

TEST_CASE("WPT surrogates UTF-8: decoded bytes match expected codepoints",
          "[wpt::surrogates_utf8]") {
    for (const auto& v : wpt_surrogates_utf8_vectors) {
        std::vector<char> bytes(v.expected.begin(), v.expected.end());
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
            got.push_back(cp);
        INFO("name=" << v.name);
        CHECK(got == v.decoded);
    }
}
```

Register in `tests/beman/transcode/CMakeLists.txt` following the same
pattern as `wpt_bom` (add_executable + target_sources +
target_include_directories + target_link_libraries + catch_discover_tests).

### Expected vectors (6 total)

| name | input codepoints | expected bytes | decoded |
|------|-----------------|----------------|---------|
| Sanity check | 0x61–0x33 | 0x61–0x33 | 0x61–0x33 |
| Surrogate half (low) | U+D800 | EF BF BD | U+FFFD |
| Surrogate half (high) | U+DC00 | EF BF BD | U+FFFD |
| Surrogate half (low), in a string | 'abc' + U+D800 + '123' | ... | 'abc' + U+FFFD + '123' |
| Surrogate half (high), in a string | 'abc' + U+DC00 + '123' | ... | 'abc' + U+FFFD + '123' |
| Wrong order | U+DC00 + U+D800 | EF BF BD EF BF BD | U+FFFD U+FFFD |

### Python tests to add

```python
_SURR_SAMPLE = """
var badStrings = [
    { input: 'a', expected: [0x61], decoded: 'a', name: 'ASCII' },
    { input: '\\uD800', expected: [0xef, 0xbf, 0xbd], decoded: '\\uFFFD',
      name: 'Lone high surrogate' },
];
"""

def test_parse_surrogates_utf8_count():
    vectors = parse_surrogates_utf8_vectors(_SURR_SAMPLE)
    assert len(vectors) == 2

def test_parse_surrogates_utf8_ascii():
    vectors = parse_surrogates_utf8_vectors(_SURR_SAMPLE)
    assert vectors[0]["input"] == [0x61]
    assert vectors[0]["expected"] == [0x61]
    assert vectors[0]["decoded"] == [0x61]

def test_parse_surrogates_utf8_surrogate():
    vectors = parse_surrogates_utf8_vectors(_SURR_SAMPLE)
    assert vectors[1]["input"] == [0xD800]
    assert vectors[1]["expected"] == [0xEF, 0xBF, 0xBD]
    assert vectors[1]["decoded"] == [0xFFFD]

def test_render_surrogates_utf8_vectors_hpp(tmp_path):
    vectors = [{"input": [0x61], "expected": [0x61], "decoded": [0x61], "name": "a"}]
    out = tmp_path / "wpt_surrogates_utf8_vectors.hpp"
    render_surrogates_utf8_vectors_hpp(vectors, out)
    content = out.read_text()
    assert "WptSurrogatesUtf8Vector" in content
    assert "wpt_surrogates_utf8_vectors" in content
```

## TDD Process

1. Branch: `git checkout -b step38-wpt-surrogates-utf8`
2. Write failing tests (RED) referencing `wpt_surrogates_utf8_vectors.hpp`
   → commit → push both remotes
3. Download WPT file, add parser/renderer to generator, add Python tests,
   generate header, fix conformance gaps if any → (GREEN)
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. Merge: `git checkout main && git merge --no-ff step38-wpt-surrogates-utf8`
6. Push main to both remotes
7. Update `docs/plans/phase2-checklist.md`

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
```

## Coding Rules (abbreviated)

- Include guards: `TESTS_BEMAN_TRANSCODE_*_HPP` for test headers
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

**Important:** `parse_js_string()` handles lone surrogates correctly:
`\uD800` as an isolated escape (not followed by `\uDC00`–`\uDFFF`)
produces codepoint 0xD800 unchanged. This is correct for this step.

All WPT JS files go in `docs/wpt/` with provenance in `SOURCE.md`.
All generated C++ headers go in `tests/beman/transcode/`.

## Key API

The encode view:
```cpp
// Encodes char32_t range to char bytes.
// Surrogates → 0xEF 0xBF 0xBD (U+FFFD in UTF-8).
inline constexpr whatwg_encode_closure<codec::utf_8> whatwg_encode<codec::utf_8>;

// Error-reporting variant: surrogates → surrogate_code_point error.
inline constexpr whatwg_encode_or_error_closure<codec::utf_8>
    whatwg_encode_or_error<codec::utf_8>;
```

Defined in `include/beman/transcode/whatwg_encode_view.hpp`.
