# Phase 2: Full WHATWG Codec Implementation

## Overview

Implements the complete WHATWG Encoding Standard codec set: all single-byte decoders and encoders, the UTF-8 encoder, UTF-16 codecs, and all legacy multi-byte codecs (CJK + miscellaneous). The success condition is that every encoding listed in the WHATWG spec has both decode and encode support.

**Preconditions:** Steps 0–16 complete, 70 tests pass on `main`. The `codec` enum has `utf_8`, `replacement`, `x_user_defined`, `windows_1252`. The single-byte decode infrastructure (`detail::single_byte_decode_one` + 128-entry table pattern) is proven.

**TDD workflow per step:** Branch `step<N>-<slug>` from main → RED commit (failing tests) → GREEN commit (implementation passing) → `make lint` → merge `--no-ff` → push both remotes.

**Progress:** See [phase2-checklist.md](phase2-checklist.md) for current status.

---

## WHATWG Encoding Inventory

### Already implemented
| Codec | Step | Direction |
|-------|------|-----------|
| UTF-8 (decode) | 5–6 | decode |
| replacement | 14 | decode |
| x-user-defined | 15 | decode |
| windows-1252 | 16 | decode |

### Single-byte (22 total, 1 done)

All share one algorithm with a different 128-entry lookup table. Decode: `table[byte - 0x80]`. Encode: reverse lookup (find pointer for codepoint in table, return `pointer + 0x80`).

IBM866, ISO-8859-2, ISO-8859-3, ISO-8859-4, ISO-8859-5, ISO-8859-6, ISO-8859-7, ISO-8859-8, ISO-8859-8-I, ISO-8859-10, ISO-8859-13, ISO-8859-14, ISO-8859-15, ISO-8859-16, KOI8-R, KOI8-U, macintosh, windows-874, windows-1250, windows-1251, windows-1252 ✓, windows-1253, windows-1254, windows-1255, windows-1256, windows-1257, windows-1258, x-mac-cyrillic.

### Multi-byte (7 codecs, each with unique algorithm)

| Codec | Index size | Notes |
|-------|-----------|-------|
| GBK | ~23,000 | Chinese simplified; superset of GB2312 |
| gb18030 | ~23,000 + ranges | Chinese; mandatory in PRC |
| Big5 | ~20,000 | Chinese traditional |
| EUC-JP | ~7,000 + ~6,000 | Japanese; uses JIS X 0208 + 0212 |
| ISO-2022-JP | ~7,000 | Japanese; stateful (escape sequences) |
| Shift_JIS | ~7,000 | Japanese; Microsoft variant |
| EUC-KR | ~17,000 | Korean |

### Other
| Codec | Notes |
|-------|-------|
| UTF-16BE | Algorithmic, no index table |
| UTF-16LE | Algorithmic, no index table |

---

## Steps

### Completed

#### [Step 14: `codec::replacement`](step14-replacement-codec.md)
Introduced `if constexpr` dispatch in `load()`. Trivial codec: any non-empty input → one U+FFFD.

#### [Step 15: `codec::x_user_defined`](step15-x-user-defined.md)
Per-byte algorithmic codec. 0x00–0x7F identity, 0x80–0xFF → U+F780–U+F7FF. Never errors.

#### [Step 16: Single-byte infrastructure + `windows_1252`](step16-single-byte-infra.md)
Generic table-driven `single_byte_decode_one()` + 128-entry table pattern. First concrete table.

### Data tooling

#### [Step 17: Data tooling — WHATWG index acquisition & table generation](step17-iso-8859-tables.md)
Acquire all WHATWG index-*.txt files, write Python parser, generate `#embed`-able binary data and constexpr header fallbacks for all single-byte tables. No runtime library code.

### Single-byte codec loop

#### Step 18: All single-byte decoders (`step18-all-single-byte-decode`)
Using the generated tables from step 17, add all 21 remaining single-byte decoders. Each codec needs: a `codec` enum value, an `#include` for its table, and dispatch branches in both `load()` functions. The hand-written `windows_1252.hpp` is replaced by the generated version. **This is a loop over 21 codecs**, not 2.

#### Step 19: Single-byte encoder infrastructure + all encoders (`step19-single-byte-encode`)
The WHATWG single-byte encoder is a reverse lookup: for a codepoint, find the table pointer that maps to it, return `pointer + 0x80`. Needs a `single_byte_encode_one()` function (possibly with a compile-time-generated reverse map), `whatwg_encode_view` dispatch for all 22 single-byte codecs, and tests for representative codecs.

### UTF-8 encoder

#### [Step 20: UTF-8 encoder](step18-utf8-encoder.md)
`whatwg_encode_view<codec::utf_8>`: takes `char32_t` range, yields `char` bytes. Adds `unicode_scalar_range` concept, encoder view, or_error variant, pipe closures, negative compile test.

### UTF-16

#### Step 21: UTF-16 decoder + encoder (`step21-utf16`)
Algorithmic (no table). UTF-16BE and UTF-16LE decode and encode. Handles surrogate pairs, BOM, and byte order.

### Multi-byte codecs (decode + encode per codec)

Each multi-byte codec has a unique algorithm defined in the WHATWG spec. Both decode and encode directions use the same index data. Steps are one per codec.

#### Step 22: GBK decode + encode (`step22-gbk`)
Chinese simplified. Superset of GB2312. Two-byte encoding with lead byte 0x81–0xFE.

#### Step 23: gb18030 decode + encode (`step23-gb18030`)
Chinese. Four-byte extension of GBK. Mandatory in PRC. Uses index + range mapping.

#### Step 24: Big5 decode + encode (`step24-big5`)
Chinese traditional. Two-byte encoding.

#### Step 25: Shift_JIS decode + encode (`step25-shift-jis`)
Japanese. Microsoft variant of JIS X 0208.

#### Step 26: EUC-JP decode + encode (`step26-euc-jp`)
Japanese. Uses JIS X 0208 + JIS X 0212 indexes.

#### Step 27: ISO-2022-JP decode + encode (`step27-iso-2022-jp`)
Japanese. Stateful encoding using escape sequences to switch character sets.

#### Step 28: EUC-KR decode + encode (`step28-euc-kr`)
Korean. Two-byte encoding.

### Composition

#### Step 29: Round-trip composition (`step29-roundtrip`)
Comprehensive round-trip tests for all implemented codecs. Validates that decode→encode is identity for valid input. First test of view composition across the full codec set.

---

## Key Architecture Decisions

| Decision | Rationale |
|----------|-----------|
| `if constexpr` dispatch on codec NTTP | Zero-cost; unused branches discarded at compile time |
| Table sentinel `char32_t(0)` | No WHATWG single-byte index maps high bytes to U+0000 |
| Generated tables from WHATWG indexes | Normative data source; avoids hand-transcription errors |
| `#embed` binary + constexpr header fallback | Forward-looking for C++26; works today with arrays |
| Reuse `whatwg_error` for encoder | `surrogate_code_point` and `out_of_range` already exist |
| Encoder internal buffer (char[4] for UTF-8) | Bounded output per codepoint; no external span needed |
| Single-byte encoder = reverse table lookup | WHATWG spec: "find index pointer for codepoint" |
| One step per multi-byte codec | Each has a unique algorithm; can't be looped |
