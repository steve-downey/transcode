# Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation

**Branch:** `step17-data-tooling`
**Depends on:** Step 16 (single-byte infrastructure exists, proves the table pattern)

---

## Goal

Build the offline tooling pipeline that acquires WHATWG encoding index
data and generates `#embed`-able binary table files for all codecs.
This step produces no runtime library code — only data files, provenance
documentation, and the scripts to regenerate tables. It unblocks the
loop over all remaining single-byte codecs (step 18) and later the
multi-byte codecs.

## Why This Step Exists

The WHATWG Encoding Standard defines **22 single-byte encodings** and
**7 multi-byte encodings** (plus UTF-16). Each has an official index
file — the normative source of truth for byte↔codepoint mappings.
Hand-transcribing tables (as done for `windows_1252` in step 16)
does not scale to 28+ codecs with tables ranging from 128 entries
(single-byte) to ~23,000 entries (multi-byte). We need:

1. A canonical copy of the index data in the repo (for reproducibility)
2. A provenance document recording source, license, and constraints
3. A Python script to parse the raw indexes and emit binary data
4. Binary files suitable for C++23 `#embed` (or constexpr array fallback)

## WHATWG Index Data Sources

### Single-byte indexes (one per codec)

URL pattern: `https://encoding.spec.whatwg.org/index-<name>.txt`

Format: each line is `<pointer>\t<codepoint>\t# <comment>` where
pointer is the byte offset (0–127, for bytes 0x80–0xFF) and codepoint
is the Unicode scalar value in hex.

Complete list (22 files):

- `index-ibm866.txt`
- `index-iso-8859-2.txt` through `index-iso-8859-16.txt`
  (skipping 1, 9, 11, 12)
- `index-koi8-r.txt`, `index-koi8-u.txt`
- `index-macintosh.txt`
- `index-windows-874.txt`
- `index-windows-1250.txt` through `index-windows-1258.txt`
- `index-x-mac-cyrillic.txt`

Note: `iso-8859-8` and `iso-8859-8-i` share the same index (differ
only in bidi handling).

### Multi-byte indexes

- `index-big5.txt` (~20,000 pointer→codepoint pairs)
- `index-euc-kr.txt` (~17,000 pairs)
- `index-gb18030.txt` (~23,000 pairs)
- `index-gb18030-ranges.txt` (range-based supplementary mapping)
- `index-jis0208.txt` (~7,000 pairs)
- `index-jis0212.txt` (~6,000 pairs)
- `index-iso-2022-jp-katakana.txt` (small)

### Metadata

- `encodings.json` — codec names, labels, aliases

## Directory Layout

### `docs/whatwg/` — pristine upstream data

Unmodified copies of downloaded WHATWG Encoding Standard files.
Nothing in this directory is processed or edited. Includes:

- All `index-*.txt` files (single-byte + multi-byte)
- `encodings.json`
- `SOURCE.md` — human-readable provenance (see below)
- `source.bib` — BibTeX citation entry

### `docs/wpt/` — Web Platform Tests (future)

Unmodified test vectors from the WPT encoding test suite, if/when
needed for conformance validation. Same provenance discipline:
pristine files + SOURCE.md + BibTeX.

### `data/` — our derived/processed artifacts

Generated tables, test vectors, processed binary data. Everything
here is reproducible from the pristine upstream in `docs/` plus
the scripts in `tools/`.

- `data/tables/` — generated `.bin` and `.hpp` files for `#embed`
  and constexpr fallback
- `data/test-vectors/` — derived test fixtures (future)

## Provenance: `docs/whatwg/SOURCE.md`

Human-readable document recording:

- **Title:** WHATWG Encoding Standard — Index Data
- **Source:** Each file's full URL
- **Retrieved:** ISO-8601 date of download
- **SHA-256:** Checksum of each file at time of download
- **License:** CC-BY 4.0 (data/documentation), BSD-3-Clause
  (code portions derived from the spec)
- **Attribution:** Derived from the WHATWG Encoding Standard,
  https://encoding.spec.whatwg.org/ , Living Standard,
  maintained by the WHATWG community.
- **Conditions:** CC-BY 4.0 requires attribution. No additional
  restrictions. Data files are non-normative but are the
  canonical reference for implementors.
- **How to refresh:** `uv run tools/download_indexes.py`

## Provenance: `docs/whatwg/source.bib`

```bibtex
@misc{whatwg-encoding,
  title        = {Encoding Standard},
  author       = {{WHATWG}},
  howpublished = {Living Standard},
  url          = {https://encoding.spec.whatwg.org/},
  note         = {Index data retrieved [DATE]},
  year         = {2024},
  license      = {CC-BY-4.0 and BSD-3-Clause}
}
```

## Deliverables

### Script: `tools/download_indexes.py`

Python script (stdlib only) that downloads all WHATWG index files into
`docs/whatwg/`. Idempotent — checks SHA-256 before re-downloading.
Generates/updates `SOURCE.md` and `source.bib` with checksums and
download date.

### Script: `tools/generate_tables.py`

Python script (stdlib only) that:

1. Reads `docs/whatwg/index-*.txt` files
2. For single-byte codecs: emits a 128-entry binary file
   (128 × 4 bytes, little-endian uint32 per codepoint, 0 = unmapped)
3. For multi-byte codecs: emits appropriate binary structures
   (format TBD per codec in later steps)
4. Generates C++ header fallbacks (constexpr arrays) for compilers
   without `#embed`

Output directory: `data/tables/`

### Generated files (single-byte, 22 total)

Each codec gets a `.bin` (for future `#embed`) and a `.hpp` (constexpr
array fallback). The `.hpp` files are included from
`include/beman/transcode/detail/tables/` which will contain thin
wrappers or direct includes of the generated data.

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-iso-8859-2.txt

#ifndef DATA_TABLES_ISO_8859_2_HPP
#define DATA_TABLES_ISO_8859_2_HPP

namespace beman::transcoding::detail::tables {

inline constexpr char32_t iso_8859_2[128] = { /* ... */ };

} // namespace beman::transcoding::detail::tables

#endif
```

## Procedure

1. Create branch `step17-data-tooling` from `main`
2. Write `tools/download_indexes.py` — fetch all index files
3. Run it → creates `docs/whatwg/` with pristine data +
   SOURCE.md + source.bib
4. Commit the downloaded data + provenance
5. Write `tools/generate_tables.py` — parse single-byte indexes,
   emit `.hpp` + `.bin` into `data/tables/`
6. Run it → populates `data/tables/`
7. Commit generated files
8. `make test` — all existing tests still pass (no runtime changes)
9. `make lint` — clean
10. Push both remotes
11. Merge to main

## Verification

```bash
# Scripts run without error
uv run tools/download_indexes.py
uv run tools/generate_tables.py

# Existing tests still pass (no runtime changes)
make test

# Generated files pass lint
make lint
```

## Notes

- The hand-written `detail/tables/windows_1252.hpp` from step 16 will
  be superseded by the generated version in step 18. Keep it for now;
  step 18 will replace it.
- Multi-byte table generation is deferred to those codec steps; this
  step focuses on the single-byte pipeline and acquiring all raw data.
- `#embed` is C23/C++26. The `.bin` files are forward-looking; the
  generated `.hpp` fallbacks provide the tables today.
- WPT test vectors (`docs/wpt/`) are a future addition when we need
  conformance testing against the web-platform-tests suite.
