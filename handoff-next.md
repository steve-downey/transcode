# Handoff: beman.transcode — Step 37 (WPT EOF/truncation vectors)

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

**Step 36 complete and merged to main.**

490 C++ tests + 146 Python tests pass (`make test`). `make lint` clean.

### What Step 36 Added

WPT conformance tests from `textdecoder-fatal-single-byte.any.js`. For
each of the 28 single-byte codecs, bytes 0x80–0xFF that map to `null` in
the WHATWG table must produce an error in fatal mode; all other bytes must
decode successfully.

Key changes:
- Downloaded `docs/wpt/textdecoder-fatal-single-byte.any.js` (28 codecs)
- Added `parse_fatal_single_byte_cases()` + `render_fatal_single_byte_vectors_hpp()`
  to `tools/generate_wpt_vectors.py`
- Added 5 Python tests to `tools/tests/test_generate_wpt.py` (146 total)
- Generated `tests/beman/transcode/wpt_fatal_single_byte_vectors.hpp`
  (struct `WptFatalSingleByteCase` with `encoding`, `bad`)
- 28 `TEST_CASE`s in `wpt_fatal_single_byte.test.cpp` — one per codec,
  each checking all 256 bytes via `whatwg_decode_or_error<C>`

New files:
- `docs/wpt/textdecoder-fatal-single-byte.any.js` — pristine WPT JS
- `tests/beman/transcode/wpt_fatal_single_byte_vectors.hpp`
- `tests/beman/transcode/wpt_fatal_single_byte.test.cpp`

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
| `textdecoder-fatal-single-byte.any.js` | Fatal mode: bad bytes per codec |

## What To Do Next — Step 37

**Branch:** `step37-wpt-eof-vectors`

### WPT file: `textdecoder-eof.any.js`

This file tests end-of-queue (EOF / truncated sequence) handling. Fetch:

```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/textdecoder-eof.any.js" \
  --jq '.content' | base64 -d > docs/wpt/textdecoder-eof.any.js
```

The file has two `test(...)` blocks:
1. **Non-streaming** — uses `new TextDecoder(encoding).decode(new Uint8Array([...]))`.
   Parse and integrate these.
2. **Streaming** — uses `{ stream: true }`. Our library is a range view
   with no stateful streaming API. **Skip these** and add a comment in
   the test file noting the gap.

### Parser approach

The non-streaming cases use this format:
```javascript
assert_equals(new TextDecoder().decode(new Uint8Array([0xF0])), "�");
assert_equals(new TextDecoder("Big5").decode(new Uint8Array([0x81, 0x40])), "�@");
```

Regex:
```python
_EOF_RE = re.compile(
    r'assert_equals\('
    r'new TextDecoder\("?([^")\s]*)"?\)'
    r'\.decode\(new Uint8Array\(\[([^\]]*)\]\)\)'
    r',\s*"((?:[^"\\]|\\.)*)"\)',
    re.MULTILINE,
)
```

When the encoding argument is absent (`new TextDecoder()`), default to
`"utf-8"`. Only parse the first `test(...)` block (non-streaming).

```python
def parse_eof_vectors(content: str) -> list[dict[str, object]]:
    vectors: list[dict[str, object]] = []
    end_marker = '}, "TextDecoder end-of-queue handling");'
    end = content.find(end_marker)
    if end == -1:
        return vectors
    section = content[:end]
    for m in _EOF_RE.finditer(section):
        encoding, bytes_str, expected_str = m.groups()
        if not encoding:
            encoding = "utf-8"
        input_bytes = [int(x.strip(), 0) for x in bytes_str.split(",") if x.strip()]
        expected_cps = parse_js_string(expected_str)
        vectors.append({
            "encoding": encoding,
            "input": input_bytes,
            "expected": expected_cps,
        })
    return vectors
```

The file covers `utf-8` and `Big5` encodings. Expected non-streaming
case count: 8 for UTF-8, 3 for Big5 (11 total). Verify with Python tests.

### Generated header struct (`WptEofVector`)

```cpp
struct WptEofVector {
    const char*           encoding;
    std::vector<uint8_t>  input;
    std::vector<char32_t> expected;
};
inline const WptEofVector wpt_eof_vectors[] = {
    {"utf-8", {0xF0},             {0xFFFD}},
    {"utf-8", {0xF0, 0x9F},       {0xFFFD}},
    {"utf-8", {0xF0, 0x9F, 0x92}, {0xFFFD}},
    // ... more utf-8 cases ...
    {"Big5",  {0x81, 0x40},       {0xFFFD, 0x0040}},
    // ...
};
```

Guard: `TESTS_BEMAN_TRANSCODE_WPT_EOF_VECTORS_HPP`.
Array name: `wpt_eof_vectors`.

### C++ test file (`wpt_eof.test.cpp`)

Two `TEST_CASE`s — one per encoding:

```cpp
TEST_CASE("WPT EOF: UTF-8 truncated sequences", "[wpt::eof]") {
    using namespace beman::transcoding::tests::wpt;
    for (const auto& v : wpt_eof_vectors) {
        if (std::string_view(v.encoding) != "utf-8") continue;
        std::vector<char> bytes(v.input.begin(), v.input.end());
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
            got.push_back(cp);
        INFO("input bytes=" << v.input.size());
        CHECK(got == v.expected);
    }
}

TEST_CASE("WPT EOF: Big5 truncated sequences", "[wpt::eof]") {
    using namespace beman::transcoding::tests::wpt;
    for (const auto& v : wpt_eof_vectors) {
        if (std::string_view(v.encoding) != "Big5") continue;
        std::vector<char> bytes(v.input.begin(), v.input.end());
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::big5>)
            got.push_back(cp);
        INFO("input bytes=" << v.input.size());
        CHECK(got == v.expected);
    }
}
```

Register in `tests/beman/transcode/CMakeLists.txt` the same way as
`wpt_bom` (add_executable + target_sources + target_include_directories +
target_link_libraries + catch_discover_tests).

### Also consider: `textdecoder-fatal-streaming.any.js`

A very small file (3 non-streaming test cases) worth adding alongside the
EOF step. Parse the non-streaming block:

```bash
gh api "repos/web-platform-tests/wpt/contents/encoding/textdecoder-fatal-streaming.any.js" \
  --jq '.content' | base64 -d > docs/wpt/textdecoder-fatal-streaming.any.js
```

Format inside the file:
```javascript
{encoding: 'utf-8',    sequence: [0xC0]},
{encoding: 'utf-16le', sequence: [0x00]},
{encoding: 'utf-16be', sequence: [0x00]}
```

These must produce an error under `whatwg_decode_or_error`. Already
covered by step 34, but including WPT-sourced confirmation is still
worthwhile. Could be a separate sub-step or bundled with step 37.

## TDD Process

1. Branch: `git checkout -b step37-wpt-eof-vectors`
2. Write failing tests (RED) referencing `wpt_eof_vectors.hpp`
   → commit → push both remotes
3. Download WPT file(s), add parser/renderer to generator, add Python
   tests, generate header, fix any conformance gaps → (GREEN)
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. Merge: `git checkout main && git merge --no-ff step37-wpt-eof-vectors`
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
- Test files: include primary header **twice** (idempotent check)
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Python Tool Pattern

The generator `tools/generate_wpt_vectors.py` follows this pattern:
1. Add a `FOO_JS = WPT_DIR / "foo.any.js"` constant
2. Add `parse_foo_vectors(content: str) -> list[dict[str, object]]`
3. Add `render_foo_vectors_hpp(vectors, out_path)`
4. Add both to `main()` + the clang-format list
5. Add Python tests to `tools/tests/test_generate_wpt.py`

**Important:** `parse_js_string()` handles `\uHHHH\uLLLL` surrogate
pairs into supplementary codepoints, `\xNN` hex escapes, and literal
multi-byte UTF-8 characters. Use it for all expected-string parsing.

All WPT JS files go in `docs/wpt/` with provenance in `SOURCE.md`.
All generated C++ headers go in `tests/beman/transcode/`.

## Key Codec Names

The `codec` enum is in `include/beman/transcode/whatwg_decode_view.hpp`:
- `codec::utf_8`, `codec::utf_16le`, `codec::utf_16be`
- `codec::gbk`, `codec::gb18030`, `codec::big5`
- `codec::shift_jis`, `codec::euc_jp`, `codec::iso_2022_jp`
- `codec::euc_kr`
- All single-byte codecs: `codec::ibm866`, `codec::iso_8859_2`, etc.
