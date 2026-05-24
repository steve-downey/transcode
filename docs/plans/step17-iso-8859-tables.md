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

- `https://encoding.spec.whatwg.org/encodings.json` — codec names,
  labels, aliases

## Deliverables

### Directory: `docs/whatwg-indexes/`

Raw downloaded index files, committed to the repo as-is. This is the
canonical reference copy of the upstream data.

Must include a provenance document (`docs/whatwg-indexes/PROVENANCE.md`)
recording:

- Source URL for each file
- Date of download
- SHA-256 checksums of each downloaded file
- License: CC-BY 4.0 (documentation/data) and BSD-3-Clause (code
  portions), per the WHATWG Encoding Standard LICENSE
- Attribution: "Derived from the WHATWG Encoding Standard,
  https://encoding.spec.whatwg.org/"
- Any conditions or constraints on redistribution

### Script: `tools/download_indexes.py`

Python script (stdlib only) that downloads all WHATWG index files into
`docs/whatwg-indexes/`. Idempotent — checks SHA-256 before
re-downloading. Generates/updates `PROVENANCE.md` with checksums
and download date. Run once to populate, then commit.

### Script: `tools/generate_tables.py`

Python script (stdlib only) that:

1. Reads `docs/whatwg-indexes/index-*.txt` files
2. For single-byte codecs: emits a 128-entry binary file
   (128 × 4 bytes, little-endian uint32 per codepoint, 0 = unmapped)
3. For multi-byte codecs: emits appropriate binary structures
   (format TBD per codec in later steps)
4. Generates C++ header fallbacks (constexpr arrays) for compilers
   without `#embed`

Output directory: `include/beman/transcode/detail/tables/generated/`

### Generated files (single-byte, 22 total)

Each codec gets a `.bin` (for future `#embed`) and a `.hpp` (constexpr
array fallback). Example:

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-iso-8859-2.txt

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GENERATED_ISO_8859_2_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GENERATED_ISO_8859_2_HPP

namespace beman::transcoding::detail::tables {

inline constexpr char32_t iso_8859_2[128] = { /* ... */ };

} // namespace beman::transcoding::detail::tables

#endif
```

## Procedure

1. Create branch `step17-data-tooling` from `main`
2. Write `tools/download_indexes.py` — fetch all index files
3. Run it → creates `docs/whatwg-indexes/` with data + PROVENANCE.md
4. Commit the downloaded data
5. Write `tools/generate_tables.py` — parse single-byte indexes,
   emit `.hpp` + `.bin`
6. Run it → creates `include/.../tables/generated/`
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
