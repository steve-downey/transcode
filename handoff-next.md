# Handoff: beman.transcode — Step 29 (Round-trip composition)

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

**393 C++ tests + 102 Python tests pass** (`make test`). Steps 0–28
complete. On `main`.

### What Step 28 Built

Step 28 added EUC-KR (UHC/CP949) decode and encode — a **stateless**
two-byte codec for Korean.

Key design decisions:

- **Table size**: `euc_kr[23940]` — `126 × 190 = 23940` entries.
  The handoff document had an incorrect size of 17176; the correct
  size is 23940 based on lead∈[0x81,0xFE] × trail∈[0x41,0xFE].

- **0x7F skip**: In DECODE, `trail == 0x7F` is treated as invalid byte
  (explicitly checked). In the decode pointer formula: `offset = trail >
  0x7F ? 1 : 0; pointer = (lead-0x81)*190 + (trail-0x41) - offset`.
  In ENCODE: `trail = i%190 + 0x41; if (trail >= 0x7F) ++trail;`

- **No state fields** needed (EUC-KR is stateless, unlike ISO-2022-JP).

### Current codec enum (in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8, replacement, x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    utf_16be, utf_16le,
    gbk, gb18030, big5, shift_jis, euc_jp, iso_2022_jp,
    euc_kr,
};
```

## What To Do Next — Step 29

**Branch:** `step29-roundtrip`

**Read the checklist:** `docs/plans/phase2-checklist.md`

### Overview

Step 29 adds **comprehensive round-trip tests**: for each bidirectional
codec (all except `replacement` and `x_user_defined`), verify that
encoding then decoding returns the original codepoints, and vice versa.

### What round-trip means

For a codec like `windows_1252`:
- Take a set of codepoints that are known to be in the codec's range.
- Encode them: `cps | whatwg_encode<codec::windows_1252>` → bytes
- Decode them back: `bytes | whatwg_decode<codec::windows_1252>` → cps2
- Check: `cps == cps2`

And the reverse:
- Start with a known byte sequence.
- Decode: bytes → cps
- Encode back: cps → bytes2
- Check: `bytes == bytes2`

### Codecs to test (all bidirectional)

**Single-byte** (all share the same decode/encode infrastructure):
- `windows_1252` (representative — already has unit tests)
- Pick 2–3 more: `koi8_r`, `iso_8859_2`, `windows_1251`

**Multi-byte**:
- `gbk` — round-trip a known 2-byte sequence (e.g., U+4E00 = 一)
- `gb18030` — round-trip U+4E00 and a supplementary codepoint (U+1F600)
- `big5` — round-trip U+4E00 and a Big5-specific char
- `shift_jis` — round-trip U+3000 (IDEOGRAPHIC SPACE) and U+4E00
- `euc_jp` — round-trip U+3000, U+FF61 (half-width katakana), U+02D8 (jis0212)
- `iso_2022_jp` — round-trip U+3000 (stateful; special care needed)
- `euc_kr` — round-trip U+AC00 (가) and U+AC02
- `utf_8` — round-trip ASCII, BMP, and supplementary
- `utf_16be` and `utf_16le` — round-trip ASCII, BMP, surrogate pair

**Skip**: `replacement` (decode-only outputs U+FFFD), `x_user_defined`
(passthrough for encode only).

### Test structure

Create `tests/beman/transcode/roundtrip.test.cpp`. Register in
`tests/beman/transcode/CMakeLists.txt`.

Helper templates:
```cpp
template <codec C>
std::vector<char32_t> encode_then_decode(std::vector<char32_t> cps) {
    std::vector<char> bytes;
    for (char b : cps | whatwg_encode<C>)
        bytes.push_back(b);
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<C>)
        result.push_back(cp);
    return result;
}

template <codec C>
std::vector<char> decode_then_encode(std::vector<char> bytes) {
    std::vector<char32_t> cps;
    for (char32_t cp : bytes | whatwg_decode<C>)
        cps.push_back(cp);
    std::vector<char> result;
    for (char b : cps | whatwg_encode<C>)
        result.push_back(b);
    return result;
}
```

### Important caveats

- **ISO-2022-JP stateful encode**: encoding a JIS X 0208 codepoint
  emits escape + lead + trail + `ESC ( B` (reset). So encoding U+3000
  gives 8 bytes: `1B 24 42 21 21 1B 28 42`. Decoding those 8 bytes
  gives [U+3000]. Round-trip works but bytes differ from what you might
  expect. When testing decode→encode round-trip, start from the 8-byte
  sequence (already reset), not just 2 bytes.

- **replacement codec**: decode-only (outputs U+FFFD for every byte).
  Skip round-trip.

- **x_user_defined**: decode maps 0x80–0xFF to U+F780–U+F7FF. Encode
  maps U+F780–U+F7FF back to 0x80–0xFF. Round-trip for those works;
  ASCII also round-trips.

- **gb18030 supplementary**: gb18030 encodes all of Unicode. U+1F600
  encodes to 4 bytes. The 4-byte decode/encode round-trip should work.

### No new headers needed

Step 29 is test-only. No new `.hpp` files. Just:
1. `tests/beman/transcode/roundtrip.test.cpp`
2. Update `tests/beman/transcode/CMakeLists.txt`

### Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
```

## TDD Process

1. Branch: `git checkout -b step29-roundtrip`
2. Write failing tests (RED) — include `whatwg_encode` from
   `whatwg_encode_view.hpp`, which already exists. Tests simply verify
   round-trip invariants using the helpers above.
3. Commit RED → push both remotes
4. Since all codecs are already implemented, the tests should go GREEN
   immediately after adding the `roundtrip.test.cpp` file and building.
5. `make test` (all pass) + `make lint` (clean) → commit → push both
6. Merge to main + push both
7. Mark checklist `[x]`

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Test files: include the primary header **twice** (idempotent check)
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`
