# Phase 2 Progress Checklist

Mark items `[x]` as they complete. Read this file first when resuming work.

**Current state:** Steps 0–55 complete. 612 C++ + 171 Python tests pass. On `main`.

---

## Step 14: `codec::replacement` (`step14-replacement-codec`)

- [x] Create branch `step14-replacement-codec` from `main`
- [x] Write failing tests (RED)
- [x] Commit RED: `"step14: replacement codec — tests (RED)"`
- [x] Push RED to both remotes
- [x] Implement `if constexpr` dispatch + replacement logic (GREEN)
- [x] `make test` — all pass
- [x] `make lint` — clean
- [x] Commit GREEN: `"step14: replacement codec — implement dispatch + replacement (GREEN)"`
- [x] Push GREEN to both remotes
- [x] Merge to main: `git checkout main && git merge --no-ff step14-replacement-codec`
- [x] Push main to both remotes

## Step 15: `codec::x_user_defined` (`step15-x-user-defined`)

- [x] Create branch `step15-x-user-defined` from `main`
- [x] Write failing tests (RED)
- [x] Commit RED + push both remotes
- [x] Implement `detail/x_user_defined.hpp` + dispatch (GREEN)
- [x] `make test` + `make lint`
- [x] Commit GREEN + push both remotes
- [x] Merge to main + push both remotes

## Step 16: Single-byte infra + `windows_1252` (`step16-single-byte-infra`)

- [x] Create branch `step16-single-byte-infra` from `main`
- [x] Write failing tests (RED)
- [x] Commit RED + push both remotes
- [x] Implement `detail/single_byte.hpp` + `detail/tables/windows_1252.hpp` + dispatch (GREEN)
- [x] `make test` + `make lint`
- [x] Commit GREEN + push both remotes
- [x] Merge to main + push both remotes

## Step 17: Data tooling (`step17-data-tooling`)

- [x] Create branch `step17-data-tooling` from `main`
- [x] Add ruff, mypy, pytest to pyproject.toml + pre-commit config
- [x] Write `tools/download_indexes.py` (typed, stdlib only)
- [x] Write `tools/tests/test_download.py`
- [x] Download all WHATWG index files → `docs/whatwg/` (pristine)
- [x] Write `docs/whatwg/SOURCE.md` (provenance, license, checksums)
- [x] Write `docs/whatwg/source.bib` (BibTeX citation)
- [x] Commit pristine upstream data + provenance + script + tests
- [x] Write `tools/generate_tables.py` (typed, stdlib only)
- [x] Write `tools/tests/test_generate.py`
- [x] Generate tables → `data/tables/`
- [x] Commit generated/derived data + script + tests
- [x] `uv run pytest tools/tests/` + `uv run mypy tools/`
- [x] `make test` + `make lint` (includes ruff)
- [x] Push both remotes + merge to main

## Step 18: All single-byte decoders (`step18-all-single-byte-decode`)

21 remaining single-byte codecs. Each needs: enum value, table include,
dispatch in both `load()` functions, tests.

- [x] Create branch from `main`
- [x] Add all 27 enum values + includes + dispatch branches
- [x] Replace hand-written `windows_1252.hpp` with WHATWG-correct generated version
- [x] Tests for representative codecs (RED → GREEN)
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

Tables placed in `include/beman/transcode/detail/tables/` (proper include
tree, not `data/tables/`). iso-8859-8-i shares iso-8859-8 table.

Codecs: IBM866, ISO-8859-2, ISO-8859-3, ISO-8859-4, ISO-8859-5,
ISO-8859-6, ISO-8859-7, ISO-8859-8, ISO-8859-8-I, ISO-8859-10,
ISO-8859-13, ISO-8859-14, ISO-8859-15, ISO-8859-16, KOI8-R, KOI8-U,
macintosh, windows-874, windows-1250, windows-1251, windows-1253,
windows-1254, windows-1255, windows-1256, windows-1257, windows-1258,
x-mac-cyrillic.

(ISO-8859-8-I shares ISO-8859-8's table; counted as one codec pair.)

## Step 19: Single-byte encoder infra + all encoders (`step19-single-byte-encode`)

- [x] Create branch from `main`
- [x] Write failing tests (RED) + push both remotes
- [x] Add `unmapped_codepoint` to `whatwg_error` enum
- [x] Add `unicode_scalar_range` concept to `detail/concepts.hpp`
- [x] Add `single_byte_encode_one()` to `detail/single_byte.hpp`
- [x] Create `whatwg_encode_view.hpp` (28 single-byte codec arms)
- [x] `make test` + `make lint` + `make coverage` (100% on new files)
- [x] Push both remotes + merge to main

## Step 20: UTF-8 encoder (`step20-utf8-encoder`)

- [x] Create branch from `main`
- [x] Write failing tests + negative compile test (RED)
- [x] Implement `detail/utf8_encode.hpp` + `whatwg_encode_view.hpp`
- [x] Add `unicode_scalar_range` concept
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 21: UTF-16 decode + encode (`step21-utf16`)

Algorithmic (no table). UTF-16BE and UTF-16LE, both directions.

- [x] Create branch from `main`
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode for both byte orders
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 22: GBK decode + encode (`step22-gbk`)

- [x] Create branch from `main`
- [x] Generate GBK tables (extend `generate_tables.py`)
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 23: gb18030 decode + encode (`step23-gb18030`)

- [x] Create branch from `main`
- [x] Generate gb18030 tables + ranges
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 24: Big5 decode + encode (`step24-big5`)

- [x] Create branch from `main`
- [x] Generate Big5 tables
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 25: Shift_JIS decode + encode (`step25-shift-jis`)

- [x] Create branch from `main`
- [x] Generate Shift_JIS tables
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 26: EUC-JP decode + encode (`step26-euc-jp`)

- [x] Create branch from `main`
- [x] Generate EUC-JP tables (JIS X 0208 + 0212)
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 27: ISO-2022-JP decode + encode (`step27-iso-2022-jp`)

Stateful encoding (escape sequences).

- [x] Create branch from `main`
- [x] Tests (RED → GREEN)
- [x] Implement stateful decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 28: EUC-KR decode + encode (`step28-euc-kr`)

- [x] Create branch from `main`
- [x] Generate EUC-KR tables
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 29: Round-trip composition (`step29-roundtrip`)

Comprehensive round-trip tests for all implemented codecs.

- [x] Create branch from `main`
- [x] Write composition tests for all codec pairs
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

---

## Step 30: WPT test vector integration (`step30-wpt-vectors`)

- [x] Create branch from `main`
- [x] Download WPT JS files → `docs/wpt/` (gb18030, mistakes/utf8)
- [x] Write `tools/generate_wpt_vectors.py` + parser/renderer
- [x] Generate `wpt_gb18030_vectors.hpp`, `wpt_utf8_vectors.hpp`
- [x] Write `wpt_gb18030.test.cpp`, `wpt_utf8.test.cpp`
- [x] Fix conformance bugs found by WPT vectors
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 31: WPT ISO-2022-JP + single-byte exhaustive (`step31-wpt-iso2022jp-singlebyte`)

- [x] Create branch from `main`
- [x] Download `iso-2022-jp-decoder.any.js` + `single-byte-decoder.window.js` → `docs/wpt/`
- [x] Extend generator for ISO-2022-JP + single-byte indexes
- [x] Generate `wpt_iso2022jp_vectors.hpp`, `wpt_single_byte_vectors.hpp`
- [x] Write `wpt_iso2022jp.test.cpp`, `wpt_single_byte.test.cpp`
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 32: ISO-2022-JP full WHATWG conformance (`step32-iso2022jp-conformance`)

- [x] Create branch from `main`
- [x] Fix all 7 ISO-2022-JP decoder bugs (katakana range, SO/SI, output state, output flag, escape EOS, invalid escape, lead byte EOS)
- [x] All 34 WPT ISO-2022-JP vectors pass
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 33: WPT UTF-16 surrogate conformance (`step33-wpt-utf16-surrogates`)

- [x] Create branch from `main`
- [x] Write failing test (RED) referencing missing vector header
- [x] Push RED to both remotes
- [x] Download `textdecoder-utf16-surrogates.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksum
- [x] Add `parse_utf16_surrogate_vectors()` + `render_utf16_surrogates_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 4 new Python tests to `test_generate_wpt.py`
- [x] Generate `wpt_utf16_surrogates_vectors.hpp` (5 vectors)
- [x] `make test` (453 C++ + 131 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

## Step 34: WPT fatal mode vectors (`step34-wpt-fatal-vectors`)

- [x] Create branch from `main`
- [x] Write failing test (RED) referencing missing vector header
- [x] Push RED to both remotes
- [x] Download `textdecoder-fatal.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksum
- [x] Add `parse_fatal_vectors()` + `render_fatal_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 4 new Python tests to `test_generate_wpt.py`
- [x] Generate `wpt_fatal_vectors.hpp` (33 UTF-8 + 1 UTF-16LE vectors)
- [x] `make test` (455 C++ + 135 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

## Step 35: WPT BOM stripping conformance (`step35-wpt-bom-vectors`)

- [x] Create branch `step35-wpt-bom-vectors` from `main`
- [x] Write failing test (RED) referencing missing vector header
- [x] Push RED to both remotes
- [x] Download `textdecoder-byte-order-marks.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksum
- [x] Fix `parse_js_string()` to combine surrogate pairs into supplementary codepoints
- [x] Add `parse_bom_vectors()` + `render_bom_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 4 new Python tests + 2 surrogate pair tests to `test_generate_wpt.py`
- [x] Generate `wpt_bom_vectors.hpp` (3 cases: utf-8, utf-16le, utf-16be)
- [x] Implement BOM stripping in `whatwg_decode_view` and `whatwg_decode_or_error_view` constructors
- [x] `make test` (462 C++ + 141 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

---

## Step 37: WPT EOF vectors + Big5 conformance fix (`step37-wpt-eof-vectors`)

- [x] Create branch `step37-wpt-eof-vectors` from `main`
- [x] Write failing tests (RED) referencing missing `wpt_eof_vectors.hpp`
- [x] Push RED to both remotes
- [x] Download `textdecoder-eof.any.js` + `textdecoder-fatal-streaming.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksums
- [x] Add `parse_eof_vectors()` + `render_eof_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 4 Python tests to `test_generate_wpt.py` (49 total)
- [x] Generate `wpt_eof_vectors.hpp` (10 vectors: 7 UTF-8 + 3 Big5)
- [x] Fix Big5 conformance bug: WHATWG step 3.6 requires prepending ASCII trail
      bytes back when pointer is null — set `code_point2` in `big5_decode_one`
      and propagate in both view `load()` functions
- [x] Update 2 existing Big5 tests (0x81,0x40 now correctly yields U+FFFD+U+0040)
- [x] `make test` (492 C++ + 151 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

---

## Step 36: WPT fatal single-byte (`step36-wpt-fatal-single-byte`)

- [x] Create branch `step36-wpt-fatal-single-byte` from `main`
- [x] Write failing test (RED) referencing missing vector header
- [x] Push RED to both remotes
- [x] Download `textdecoder-fatal-single-byte.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksum
- [x] Add `parse_fatal_single_byte_cases()` + `render_fatal_single_byte_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 5 new Python tests to `test_generate_wpt.py`
- [x] Generate `wpt_fatal_single_byte_vectors.hpp` (28 cases)
- [x] `make test` (490 C++ + 146 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

## Step 38: WPT UTF-8 surrogate encode vectors (`step38-wpt-surrogates-utf8`)

- [x] Create branch `step38-wpt-surrogates-utf8` from `main`
- [x] Write failing tests (RED) referencing missing `wpt_surrogates_utf8_vectors.hpp`
- [x] Push RED to both remotes
- [x] Download `api-surrogates-utf8.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksum
- [x] Add `_SURR_ENTRY_RE` + `parse_surrogates_utf8_vectors()` + `render_surrogates_utf8_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 4 Python tests to `test_generate_wpt.py` (155 total)
- [x] Generate `wpt_surrogates_utf8_vectors.hpp` (6 vectors)
- [x] `make test` (494 C++ + 155 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

---

## Upcoming

## Step 39: WPT TextEncoder UTF-16 surrogate round-trip (`step39-wpt-encoder-surrogates`)

- [x] Create branch from `main`
- [x] Write failing tests (RED) + push both remotes
- [x] Download `textencoder-utf16-surrogates.any.js` → `docs/wpt/`
- [x] Update `docs/wpt/SOURCE.md` with provenance + checksum
- [x] Add `parse_encoder_surrogates_vectors()` + `render_encoder_surrogates_vectors_hpp()` to `generate_wpt_vectors.py`
- [x] Add 4 Python tests to `test_generate_wpt.py` (159 total)
- [x] Generate `wpt_encoder_surrogates_vectors.hpp` (6 vectors)
- [x] `make test` (495 C++ + 159 Python all pass)
- [x] `make lint` — clean
- [x] Push GREEN to both remotes + merge to main

### WPT TextEncoder UTF-16 surrogate round-trip (`step39-wpt-encoder-surrogates`) — DONE

### Label lookup API (`step39-label-lookup` or `step40-label-lookup`)

WHATWG Encoding Standard §4.2 "Names and labels" defines ~200+ string labels
that map to canonical encodings (e.g., `"sjis"`, `"x-sjis"`, `"shift_jis"` all
→ Shift_JIS). This is essential for:

- Parsing `charset` attributes in HTML `<meta>` tags
- Parsing `Content-Type` headers
- Web-compatible encoding sniffing

API sketch:
```cpp
// Returns nullopt for unknown labels
constexpr std::optional<codec> get_encoding(std::string_view label);

// Case-insensitive, ASCII-whitespace-stripped per WHATWG
static_assert(get_encoding("  UTF-8  ") == codec::utf_8);
static_assert(get_encoding("shift_jis") == codec::shift_jis);
static_assert(get_encoding("SJIS") == codec::shift_jis);
static_assert(get_encoding("x-sjis") == codec::shift_jis);
```

- [x] Create branch from `main`
- [x] Download `encodings.json` from WHATWG → `docs/whatwg/` (already present)
- [x] Write `tools/generate_labels.py` to produce label→codec map
- [x] Write `tools/tests/test_generate_labels.py` (12 Python tests)
- [x] Write failing C++ tests (RED) + push both remotes
- [x] Implement `get_encoding()` in `detail/labels.hpp` (generated)
- [x] `make test` (502 C++ + 171 Python) + `make lint`
- [x] Push both remotes + merge to main

## Step 41: `sniff_encoding` BOM detection (`step41-sniff-encoding`)

- [x] Create branch `step41-sniff-encoding` from `main`
- [x] Write failing C++ tests (RED) + push both remotes
- [x] Implement `include/beman/transcode/detail/sniff.hpp`
- [x] `make test` (513 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] Push GREEN to both remotes + merge to main

## Step 42: Umbrella header (`step42-umbrella-header`)

- [x] Create branch `step42-umbrella-header` from `main`
- [x] Write failing C++ tests (RED) — `transcode.test.cpp` using only `<beman/transcode/transcode.hpp>`
- [x] Register `beman.transcode.tests.transcode` in `tests/beman/transcode/CMakeLists.txt`
- [x] Push RED to both remotes
- [x] Update `include/beman/transcode/transcode.hpp` to include all public headers
- [x] `make test` (521 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] Push GREEN to both remotes + merge to main

## Step 43: `transcode_string` one-shot function (`step43-transcode-string`)

- [x] Create branch `step43-transcode-string` from `main`
- [x] Write failing C++ tests (RED) — `transcode_string.test.cpp`
- [x] Register `beman.transcode.tests.transcode_string` in `tests/beman/transcode/CMakeLists.txt`
- [x] Push RED to both remotes
- [x] Implement `include/beman/transcode/detail/transcode_string.hpp`
- [x] Add `transcode_string.hpp` to umbrella header `transcode.hpp`
- [x] `make test` (531 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] Push GREEN to both remotes + merge to main

## Step 44: Coverage audit of `transcode_string` (`step44-coverage-audit`)

- [x] Create branch `step44-coverage-audit` from `main`
- [x] Add 10 new test cases targeting all previously uncovered switch arms
  - ASCII identity through all 27 uncovered single-byte decode arms
  - ASCII identity through all 27 uncovered single-byte encode arms
  - UTF-16LE decode: "hi" in UTF-16LE bytes → UTF-8
  - UTF-16BE decode: "hi" in UTF-16BE bytes → UTF-8
  - UTF-16LE encode: UTF-8 "hi" → correct LE byte pairs
  - UTF-16BE encode: UTF-8 "hi" → correct BE byte pairs
  - ASCII identity through all 5 CJK decode arms (gb18030, big5, euc_jp, iso_2022_jp, euc_kr)
  - ASCII identity through all 5 CJK encode arms
  - Default encode arm: replacement and x_user_defined as `to` → empty string
- [x] `make test` (541 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] `make coverage`: lines 58.9%→83.2%, functions 67.2%→99.9%
- [x] Push GREEN to both remotes + merge to main

## Step 45: `transcode_string` label overload (`step45-transcode-string-label`)

- [x] Create branch `step45-transcode-string-label` from `main`
- [x] Write failing tests (RED) — `transcode_string_label.test.cpp`
- [x] Register `beman.transcode.tests.transcode_string_label` in CMakeLists.txt
- [x] Push RED to both remotes
- [x] Add `std::optional<std::string> transcode_string(span, string_view, string_view)` to `transcode_string.hpp`
- [x] Include `<beman/transcode/detail/labels.hpp>` and `<optional>` in `transcode_string.hpp`
- [x] `make test` (550 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] `make coverage`: lines 83.2%, functions 99.9% (unchanged — thin delegate)
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 46: `transcode_view` pipe composition helper (`step46-transcode-view`)

- [x] Create branch `step46-transcode-view` from `main`
- [x] Write failing tests (RED) — `transcode_view.test.cpp`
- [x] Register `beman.transcode.tests.transcode_view` in CMakeLists.txt
- [x] Push RED to both remotes
- [x] Implement `include/beman/transcode/detail/transcode_view.hpp`
- [x] Add `transcode_view.hpp` to umbrella header `transcode.hpp`
- [x] `make test` (557 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] `make coverage`: lines 83.4%, functions 99.9%
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 47: `transcode_view` negative compile test + consteval test (`step47-transcode-view-negative-consteval`)

- [x] Create branch `step47-transcode-view-negative-consteval` from `main`
- [x] Write `transcode_view_reject_array_fail.cpp` (RED)
- [x] Register in `tests/beman/transcode/CMakeLists.txt` as negative compile test
- [x] Push RED to both remotes
- [x] Replace stub `constify(true)` with real round-trip consteval test
- [x] Add `constexpr` to four `operator==(iterator, default_sentinel_t)` friends in `whatwg_decode_view.hpp` and `whatwg_encode_view.hpp` (GREEN)
- [x] `make test` (558 C++ + 171 Python all pass) + `make lint` (C++ clean)
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 48: `sniff_encoding` negative compile test + Python lint fix (`step48-sniff-negative-lint-fix`)

- [x] Create branch `step48-sniff-negative-lint-fix` from `main`
- [x] Write `sniff_reject_char32_range_fail.cpp` (negative compile test for `sniff_encoding` rejecting `char32_t` ranges)
- [x] Register in `tests/beman/transcode/CMakeLists.txt` (EXCLUDE_FROM_ALL + PASS_REGULAR_EXPRESSION "legacy_byte_range")
- [x] Push RED to both remotes
- [x] Fix ruff E501 + SIM105 violations in `tools/generate_labels.py`
- [x] Fix ruff E501 in `tools/tests/test_generate_labels.py`
- [x] `make test` (559 C++ + 171 Python all pass) + `make lint` (C++/tools clean; papers/wg21 pre-existing only)
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 49: `whatwg_decode_view` coverage improvement (`step49-decode-view-coverage`)

- [x] Create branch `step49-decode-view-coverage` from `main`
- [x] Add UTF-16 truncated-surrogate tests (high surrogate + only 1 trailing byte)
- [x] Add GB18030 replay-with-high-lead-byte tests (error + success paths)
- [x] Add ISO-2022-JP `_or_error` coverage tests (all uncovered state transitions)
- [x] Add `_or_error` BOM stripping test for UTF-16BE
- [x] `make test` (583 C++ + 171 Python all pass) + `make lint` (C++/tools clean)
- [x] `make coverage`: `whatwg_decode_view.hpp` 79.7% → 97.1%; overall lines 83.4% → 84.7%
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 50: iconv view boundary-condition tests (`step50-iconv-boundary-tests`)

- [x] Create branch `step50-iconv-boundary-tests` from `main`
- [x] Add `mock_iconv_partial_consume` and `mock_iconv_e2big_zero_output` to `iconv_mock.hpp`
- [x] Add EILSEQ and partial-staging-consume tests to `iconv_transcode.test.cpp`
- [x] Add E2BIG-zero-output and partial-consume tests to `iconv_transcode_or_error.test.cpp`
- [x] Add real-iconv E2BIG (100 chars, 4-byte buf) and split multi-byte U+1D11E tests to `iconv_real.test.cpp`
- [x] `make test` (589 C++ + 171 Python all pass) + `make lint` (C++/tools clean)
- [x] `make coverage`: `iconv_transcode_view` 84.8% → 94.9%; `iconv_transcode_or_error_view` 83.7% → 92.9%
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 51: iconv stateful flush (`step51-iconv-stateful-flush`)

- [x] Create branch `step51-iconv-stateful-flush` from `main`
- [x] Add `flushed_` member to both iconv view iterators
- [x] Implement flush call (`iconv(cd, nullptr, ...)`) in both `load()` functions
- [x] Fix all existing mocks to handle `inbuf=nullptr` (null flush = no-op)
- [x] Add `mock_iconv_stateful` — writes 0x0F reset byte on flush
- [x] Add flush tests for both `iconv_transcode_view` and `_or_error` variants
- [x] Fix inline `close_counting_fns` structs to handle null flush
- [x] `make test` (591 C++ + 171 Python all pass) + `make lint` (C++/tools clean)
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 52: real-iconv ISO-2022-JP flush integration tests (`step52-iconv-iso2022jp-flush-test`)

- [x] Create branch `step52-iconv-iso2022jp-flush-test` from `main`
- [x] Add あ (U+3042) → ISO-2022-JP test (verifies ESC sequences + flush)
- [x] Add mixed ASCII+Japanese → ISO-2022-JP test
- [x] Add `_or_error` variant ISO-2022-JP test
- [x] Add small-buffer (3 bytes) ISO-2022-JP test (E2BIG + flush)
- [x] `make test` (595 C++ + 171 Python all pass) + `make lint` (C++/tools clean)
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 53: coverage improvements (`step53-coverage-improvements`)

- [x] Create branch `step53-coverage-improvements` from `main`
- [x] Add ISO-2022-JP encoder coverage: consecutive YEN/OVERLINE (Roman state), unmapped-after-JIS
- [x] Add ISO-2022-JP or_error coverage: YEN, consecutive YEN+OVERLINE, ASCII-after-JIS, JIS-after-JIS
- [x] Add EUC-JP decode coverage: SS3 truncated, SS3 invalid trails, unmapped JIS X 0212/0208 pointers
- [x] Add GB18030 encode coverage: U+E7C7 special case, U+0080 4-byte range encoding
- [x] Exclude `papers/wg21` from ruff lint (pre-existing failures)
- [x] `make test` (608 C++ + 171 Python all pass) + `make lint` (C++/tools clean)
- [x] `make coverage`: `whatwg_encode_view.hpp` 92.0% → 100.0%; `euc_jp.hpp` 92.9% → 98.8%; `gb18030.hpp` 93.3% → 95.0%; overall 85.1% → 85.4%
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 53b: unreachable code annotations (`step53b-unreachable-annotations`)

- [x] Create branch `step53b-unreachable-annotations` from `main`
- [x] Add `#include <utility>` to gb18030.hpp for std::unreachable()
- [x] Mark 4 unreachable locations in gb18030.hpp with std::unreachable()
  - Lines 70, 72: Pre-check prevents pointer overflow
  - Line 76: Binary search can't fail
  - Line 96: gb18030 covers all Unicode
  - Line 172: GBK table has no zeros
- [x] Mark 1 unreachable location in whatwg_decode_view.hpp (line 869: windows_1252 no nulls)
- [x] `make test` (608 C++ + 171 Python all pass) + `make lint` (C++/tools clean)
- [x] Push GREEN to both remotes + merge to main + push both remotes

## Step 54: C++23 module support audit (`step54-module-audit`)

- [x] Create branch `step54-module-audit` from `main`
- [x] Audit umbrella header vs. public API headers: find missing ones
- [x] Write failing tests (RED) — verify iconv headers accessible
- [x] Add iconv_transcode_view.hpp, iconv_transcode_or_error_view.hpp, iconv_real.hpp to umbrella
- [x] `make test` (611 C++ + 171 Python all pass) + `make lint` (C++/CMake clean)
- [x] Commit: `"step54: C++23 module support audit — add missing iconv headers to umbrella (GREEN)"`
- [x] Push GREEN to both remotes
- [x] Merge to main: `git checkout main && git merge --no-ff step54-module-audit`
- [x] Push main to both remotes

**Result**: Three iconv headers were found missing from the umbrella header
(added in steps 11–13, predating the umbrella). Now all public APIs are
properly exported by the C++23 module interface.

## Step 55: Module integration and smoke tests (`step55-module-tests`)

- [x] Create branch `step55-module-tests` from `main`
- [x] Write module integration test: import beman.transcode; use whatwg + iconv APIs
- [x] Register module test in CMakeLists.txt (conditional on BEMAN_TRANSCODE_USE_MODULES=ON)
- [x] Enhanced installtest/test.cpp — now uses 7 functional tests instead of just returning 0
- [x] Created installtest/test_module.cpp — module-mode version of installtest
- [x] Enhanced installtest/CMakeLists.txt to test both header and module modes
- [x] Fixed missing headers in FILE_SET (labels, sniff, transcode_string, transcode_view, x_user_defined, iconv_real, iconv_transcode_or_error_view)
- [x] `make test` (612 C++ + 171 Python all pass) + installtest passes
- [x] `make lint` (C++/CMake clean; pre-existing failures in docs/papers only)
- [x] Commit GREEN + linter fixes
- [x] Push both remotes + merge to main

**Result**: Module tests framework in place (requires GCC-16+ for compilation).
Installtest now validates all key APIs work correctly. Fixed pre-existing issue
where many headers weren't included in the installed package's FILE_SET.
All 612 C++ + 171 Python tests pass.

---

## Notes

- Each step's detailed plan: `docs/plans/step<N>-<slug>.md`
- Phase 2 overview: `docs/plans/phase2-index.md`
- TDD convention: RED = `"step<N>: <desc> (RED)"`, GREEN = `"step<N>: <desc> (GREEN)"`
- Push both remotes: `git push origin <branch> && git push bbgithub <branch>`
- No `Co-Authored-By` trailers in this project
- Pristine upstream: `docs/whatwg/` (WHATWG), `docs/wpt/` (WPT) — each
  with SOURCE.md (human-readable provenance) + source.bib (BibTeX)
- Derived/processed data: `data/` (tables, test vectors, etc.)
