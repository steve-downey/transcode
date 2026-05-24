# Handoff: beman.transcode — Step 27 (ISO-2022-JP decode + encode)

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

**330 C++ tests + 91 Python tests pass** (`make test`). Steps 0–26
complete. On `main`.

### What Step 26 Built

Step 26 added EUC-JP decode and encode:

- **`tools/generate_tables.py`** — extended with
  `EUC_JP_JIS0212_POINTER_COUNT = 8836`, `parse_euc_jp_jis0212_index()`,
  `render_euc_jp_jis0212_hpp()`, `generate_euc_jp_jis0212()`. Called
  from `main()`.

- **`include/beman/transcode/detail/tables/euc_jp_jis0212.hpp`** —
  generated `inline constexpr char32_t euc_jp_jis0212[8836]`. Source:
  `index-jis0212.txt`. Pointer formula: `(b1 - 0xA1) * 94 + (b2 - 0xA1)`.

- **`include/beman/transcode/detail/euc_jp.hpp`** —
  `euc_jp_decode_result`, `euc_jp_encode_result`, `euc_jp_decode_one()`,
  `euc_jp_encode_one()`. Handles ASCII, SS2 (half-width katakana via
  0x8E), SS3 (JIS X 0212 via 0x8F), and JIS X 0208 (0xA1–0xFE lead).
  Encode searches first 8836 entries of `tables::shift_jis` (jis0208
  range) then all of `tables::euc_jp_jis0212`.

- **`include/beman/transcode/whatwg_decode_view.hpp`** — added
  `codec::euc_jp` to enum; added dispatch arms in both `load()`
  functions.

- **`include/beman/transcode/whatwg_encode_view.hpp`** — added
  `#include <beman/transcode/detail/euc_jp.hpp>` and dispatch arms in
  both `load()` functions.

### Current codec enum (in `whatwg_decode_view.hpp`)

```cpp
enum class codec {
    utf_8,
    replacement,
    x_user_defined,
    ibm866, iso_8859_2, ..., x_mac_cyrillic,
    utf_16be,
    utf_16le,
    gbk,
    gb18030,
    big5,
    shift_jis,
    euc_jp,
    // iso_2022_jp, euc_kr NOT YET — step 27+
};
```

### IMPORTANT: Table reuse for JIS X 0208

Both EUC-JP and ISO-2022-JP share JIS X 0208. The first 8836 entries of
`tables::shift_jis` map the JIS X 0208 index (pointer =
`(b1 - 0xA1) * 94 + (b2 - 0xA1)`). Do NOT introduce a new table for
ISO-2022-JP — reuse `tables::shift_jis[0..8835]` exactly as EUC-JP
encode does.

## What To Do Next — Step 27

**Branch:** `step27-iso-2022-jp`

**Read the checklist:** `docs/plans/phase2-checklist.md`

There is no detailed step27 plan file. Use this handoff as the
authoritative spec.

### ISO-2022-JP is STATEFUL — critical design constraint

Unlike all previous codecs, ISO-2022-JP is a stateful encoding. The
decoder maintains a state machine across bytes; the encoder must track
the current output charset and emit escape sequences when switching.

This means the decode iterator must carry an extra state field (which
charset is currently selected). The encode iterator must carry the
current output state field (for escape sequence emission at codec change
or stream end).

Both current iterators already have spare fields (`has_pending_`,
`pending_[2]`, `pending_cp_`, `has_pending_cp_`) used only by Big5/GB18030.
For ISO-2022-JP, add two fields to **both** iterator classes:
```cpp
int           iso2022jp_state_{0};     // enum: 0=ASCII, 1=Roman, 2=Katakana, 3=JIS0208
unsigned char iso2022jp_lead_{0};      // stored lead byte in JIS0208 mode
```
These fields are always present (they're just zeroed for all other codecs).

For the **encode** iterator, the current buffer is `char buf_[4]` (or
`result_t buf_[4]`). ISO-2022-JP encode may need up to 5 bytes for a
single codepoint (ESC $ B = 3 bytes + lead + trail = 5 bytes). Resize
**both** buffers from `[4]` to `[8]` to accommodate this. Verify no
existing tests break.

### WHATWG ISO-2022-JP decoder algorithm

Source: https://encoding.spec.whatwg.org/#iso-2022-jp-decoder

**Decoder states** (values for `iso2022jp_state_`):
```
0 = ASCII       (initial state)
1 = Roman
2 = Katakana
3 = Lead_Byte   (JIS X 0208 first byte pending)
4 = Escape      (just saw 0x1B)
5 = Escape_Middle (just saw 0x1B then 0x24 or 0x28)
```

**Algorithm** — the `load()` for ISO-2022-JP must loop consuming bytes
until it produces one output value or hits end-of-input:

```
loop:
  if current == end:
    if state is Lead_Byte: emit U+FFFD (or error)
    else: done_ = true; return

  byte = *current; ++current

  state == ASCII or Roman or Katakana:
    if byte == 0x1B: state = Escape; continue loop
    if state == ASCII:
      if 0x00–0x7F: emit char32_t(byte); return
      else: emit U+FFFD (or error); return
    if state == Roman:
      if byte == 0x5C: emit U+00A5; return
      if byte == 0x7E: emit U+203E; return
      if 0x00–0x7F: emit char32_t(byte); return
      else: emit U+FFFD (or error); return
    if state == Katakana:
      if 0xA1–0xDF: emit char32_t(0xFF61 + byte - 0xA1); return
      else: emit U+FFFD (or error); return

  state == Lead_Byte:
    iso2022jp_lead_ = byte; state = Trail_Byte; continue loop
    (Note: Trail_Byte is a transient inner state handled below)

  Actually use:
  state == Lead_Byte:
    if byte == 0x1B: emit U+FFFD (or error); state = Escape; continue
    if 0x21–0x7E: iso2022jp_lead_ = byte; state = Trail_Byte; continue
    else: emit U+FFFD (or error); state = ASCII; continue

  state == Trail_Byte (use iso2022jp_state_ == 6 or handle inline):
    if 0x21–0x7E:
      pointer = (iso2022jp_lead_ - 0x21) * 94 + (byte - 0x21)
      cp = tables::shift_jis[pointer]   (same as jis0208 lookup)
      if cp == 0: emit U+FFFD (or error)
      else: emit cp
      state = Lead_Byte; return
    else: emit U+FFFD (or error); state = ASCII; return

  state == Escape:
    if byte == 0x24 or byte == 0x28:
      iso2022jp_lead_ = byte; state = Escape_Middle; continue loop
    else: emit U+FFFD (or error); state = ASCII; re-process byte (put back)
    Note: "put back" is hard with input_iterator. Instead:
      --current is NOT available for input iterators.
      Emit U+FFFD; state = ASCII; let next load() call see the byte
      → Use a "pending byte" flag: store byte in pending_[0], set has_pending_=true

  state == Escape_Middle (iso2022jp_lead_ is 0x24 or 0x28):
    if iso2022jp_lead_ == 0x28:
      if byte == 0x42: state = ASCII; continue loop  (ESC ( B)
      if byte == 0x4A: state = Roman; continue loop  (ESC ( J)
      if byte == 0x49: state = Katakana; continue loop  (ESC ( I)
      else: emit U+FFFD; state = ASCII; has_pending_=true; pending_[0]=byte; continue
    if iso2022jp_lead_ == 0x24:
      if byte == 0x40 or byte == 0x42: state = Lead_Byte; continue loop
      else: emit U+FFFD; state = ASCII; has_pending_=true; pending_[0]=byte; continue
```

**Simplification for the WHATWG non-error (replacement) variant:**
- Error bytes → emit U+FFFD; return
- No error tracking needed

**For the or_error variant:**
- Error bytes → emit `std::unexpected(whatwg_error::invalid_byte)`

**Note on `has_pending_`**: The existing `has_pending_` flag and
`pending_[2]` array are already in the iterator for Big5/GB18030 use.
For ISO-2022-JP, reuse `has_pending_` and `pending_[0]` for the
"byte put-back" case.

**State after escape sequences**: ISO-2022-JP escape sequences are
silent — they change `iso2022jp_state_` but produce no output. The
`load()` loop continues until it finds an actual codepoint byte to emit.

**Simplification**: Use 6 values for `iso2022jp_state_`:
```
0 = ASCII
1 = Roman
2 = Katakana
3 = Lead_Byte   (next byte is JIS X 0208 lead)
4 = Trail_Byte  (iso2022jp_lead_ holds the lead; next byte is trail)
5 = Escape      (just saw 0x1B)
6 = Escape_Middle (saw 0x1B then 0x24 or 0x28; iso2022jp_lead_ = that byte)
```

### WHATWG ISO-2022-JP encoder algorithm

Source: https://encoding.spec.whatwg.org/#iso-2022-jp-encoder

The encoder state (in `iso2022jp_state_` of the encode iterator):
```
0 = ASCII (initial, and final state expected at stream end)
1 = Roman
2 = JIS0208
```

**Algorithm** for `load()` — called each time the buffer is exhausted:

```
if current_ == end_:
  if iso2022jp_state_ == 0: done_ = true; return
  if iso2022jp_state_ != 0:
    buf_[0..2] = ESC ( B; len_ = 3; iso2022jp_state_ = 0; pos_ = 0; return

cp = static_cast<char32_t>(*current_); ++current_;

if cp < 0x80:
  new_state = ASCII (maybe Roman for U+00A5, U+203E)
  Actually:
    if cp == U+00A5 or cp == U+203E → target Roman state
    else → target ASCII state
  
  if iso2022jp_state_ != target_state:
    emit ESC ( J (Roman: 3 bytes) or ESC ( B (ASCII: 3 bytes)
    set iso2022jp_state_ = target_state
    append the actual byte to buf_
    (total len_ = 3 or 4)
  else:
    buf_[0] = ASCII byte (or 0x5C for U+00A5, 0x7E for U+203E); len_ = 1
  pos_ = 0; return

if cp in U+FF61–U+FF9F: WHATWG spec says encode as "?" (unmapped) for
  ISO-2022-JP; these are EUC-JP half-width katakana, not in ISO-2022-JP

search tables::shift_jis[0..8835] for cp:
  if found at pointer P:
    lead  = (P / 94) + 0x21
    trail = (P % 94) + 0x21
    if iso2022jp_state_ != JIS0208:
      emit ESC $ B (3 bytes) + lead + trail; len_ = 5
      iso2022jp_state_ = JIS0208
    else:
      emit lead + trail; len_ = 2
    pos_ = 0; return

not found → unmapped:
  emit '?' (non-error) or unmapped_codepoint error
  if iso2022jp_state_ != ASCII:
    emit ESC ( B + '?'; len_ = 4; iso2022jp_state_ = ASCII
  else:
    buf_[0] = '?'; len_ = 1
  pos_ = 0; return
```

**Important edge case**: When the input is exhausted and
`iso2022jp_state_ != ASCII`, the encoder must emit `ESC ( B` to return
to ASCII. The current `load()` design calls `done_ = true` when
`current_ == end_`. For ISO-2022-JP, add a check at the top of `load()`:
if `current_ == end_` AND `iso2022jp_state_ != 0`, emit the final
`ESC ( B` instead of marking done, then on the next `load()` call mark
done.

### Implementation order

1. **Add `codec::iso_2022_jp`** to the enum in `whatwg_decode_view.hpp`
   (after `euc_jp`).

2. **Resize encode buffers** in `whatwg_encode_view.hpp`:
   - `whatwg_encode_view::iterator`: `char buf_[4]{}` → `char buf_[8]{}`
   - `whatwg_encode_or_error_view::iterator`: `result_t buf_[4]{}` → `result_t buf_[8]{}`
   This is safe — existing tests will continue to pass.

3. **Add state fields** to all four iterator classes in both view headers:
   - In `whatwg_decode_view<C,R>::iterator`: add `int iso2022jp_state_{0}` and
     `unsigned char iso2022jp_lead_{0}`
   - In `whatwg_decode_or_error_view<C,R>::iterator`: same two fields
   - In `whatwg_encode_view<C,R>::iterator`: add `int iso2022jp_state_{0}`
   - In `whatwg_encode_or_error_view<C,R>::iterator`: add `int iso2022jp_state_{0}`

4. **Create `include/beman/transcode/detail/iso2022jp.hpp`** (optional
   helper). The decode/encode logic can live directly in the `load()`
   dispatch arms instead of a separate header, since the algorithm is
   tightly coupled to the iterator state. However, for testability,
   creating a separate header is preferred.

   Alternative: embed the state machine logic directly in the `load()`
   dispatch arms, accessing `iso2022jp_state_`, `iso2022jp_lead_`,
   `has_pending_`, `pending_` directly. This avoids a helper header but
   makes the dispatch arms larger.

   **Recommendation**: embed directly in load(). The ISO-2022-JP state
   machine is 30–50 lines; splitting it out requires passing many
   iterator-internal fields by reference.

5. **Add `#include <beman/transcode/detail/tables/shift_jis.hpp>`** to
   `whatwg_decode_view.hpp` if not already there (it is — EUC-JP added
   it via `euc_jp.hpp`). No new table needed.

6. **Update `include/beman/transcode/CMakeLists.txt`**: No new header
   files if the logic is embedded in the dispatch arms (no iso2022jp.hpp).

7. **Create test files**:
   - `tests/beman/transcode/iso2022jp_decode.test.cpp`
   - `tests/beman/transcode/iso2022jp_encode.test.cpp`
   Register in `tests/beman/transcode/CMakeLists.txt`.

8. **Test cases to write**:

   Decode:
   - ASCII passthrough: `0x41` → U'A' (initial ASCII state)
   - JIS X 0208 via ESC $ B: `1B 24 42 21 21` → U+3000 (jis0208 pointer 0)
     (pointer 0: lead=0x21, trail=0x21 → (0x21-0x21)*94+(0x21-0x21)=0 → tables::shift_jis[0])
   - JIS X 0208 4E00: `1B 24 42 30 6C` → U+4E00 (一)
     (pointer 1485: lead_offset=15 → 0x21+15=0x30, trail_offset=75 → 0x21+75=0x6C)
   - Switch to Roman then back: `1B 28 4A 5C` → U+00A5 (¥)
   - Switch to ASCII: `1B 28 42 41` → U'A'
   - Switch to Katakana (ESC ( I): `1B 28 49 A1` → U+FF61
   - Invalid JIS X 0208 lead byte (out of 0x21–0x7E) → U+FFFD
   - Invalid JIS X 0208 trail byte → U+FFFD
   - Lone ESC at end-of-input → U+FFFD
   - Mixed: ASCII then JIS0208 sequence then ASCII
   - or_error variants (invalid_byte errors)
   - Pipe syntax + consteval (ASCII only, for consteval)

   Encode:
   - ASCII 'A' (initial ASCII state, no escape needed) → `0x41`
   - U+00A5 (needs Roman state): first time → `ESC ( J 0x5C` (4 bytes)
   - U+203E (needs Roman state): `ESC ( J 0x7E`
   - U+3000 (JIS X 0208, pointer 0): first time → `ESC $ B 0x21 0x21` (5 bytes)
   - U+4E00 (pointer 1485): if already in JIS0208 state → `0x30 0x6C` (2 bytes)
   - U+1F600 (unmapped): → `'?'` or `unmapped_codepoint` error
   - End of stream while in JIS0208 state: final `ESC ( B` emitted
   - Mixed: 'A' + U+3000 + 'B' → `0x41 ESC$B 0x21 0x21 ESC(B 0x42`
   - Pipe syntax + consteval (ASCII only)

### Known tricky parts

1. **"Put-back" byte for Escape errors**: When in Escape or Escape_Middle
   state and the next byte doesn't match, the spec requires re-processing
   that byte. Since `input_iterator` doesn't support decrement, use the
   existing `has_pending_` / `pending_[0]` fields as a one-byte look-ahead
   buffer. At the top of `load()`, check `has_pending_` first.

2. **End-of-stream in Lead_Byte state**: If `current_ == end_` while in
   Lead_Byte state (state=3), emit U+FFFD and mark done.

3. **Encode: tracking when to emit escape sequences**: The encode iterator
   field `iso2022jp_state_` (0=ASCII, 1=Roman, 2=JIS0208) must persist
   across `load()` calls. When the input is exhausted but state != ASCII,
   emit the return-to-ASCII escape before marking done.

4. **JIS X 0208 pointer in encode**: The pointer formula for ISO-2022-JP
   uses: `lead = (P / 94) + 0x21`, `trail = (P % 94) + 0x21`. (Note:
   EUC-JP uses `+ 0xA1`; ISO-2022-JP uses `+ 0x21`.) The table lookup
   uses the same `tables::shift_jis[P]` where P is in `[0, 8835]`.

5. **JIS X 0208 pointer in decode**: lead byte is in `0x21–0x7E` (94
   values starting at 0x21). Trail is also `0x21–0x7E`. Pointer =
   `(lead - 0x21) * 94 + (trail - 0x21)`. This is the same range
   `[0, 8835]` as EUC-JP jis0208 but accessed with `0x21` offset
   instead of `0xA1` offset.

### No new Python tooling needed

ISO-2022-JP does not require a new table. No changes to
`tools/generate_tables.py` or `tools/tests/test_generate.py`.

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Includes: angle brackets only, full path from include root
- Test files: include the header under test **twice** (idempotent check)
- Functions: out-of-line in headers with full qualification
- `constexpr` everything that can be
- License: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Build Commands

```bash
make test      # build + run ALL tests: C++ (ctest) + Python (pytest)
make lint      # clang-format + gersemi + ruff + codespell + mypy + gitleaks
make compile   # build only
make coverage  # gcovr coverage report
make pytest    # Python tool tests only
make mypy      # mypy type check only
```

## TDD Process

1. Branch: `git checkout -b step27-iso-2022-jp`
2. Write failing tests referencing `codec::iso_2022_jp` (RED) → commit → push both
3. Implement (GREEN):
   - Add enum value
   - Resize encode buffers
   - Add state fields to iterators
   - Embed decode/encode logic in dispatch arms
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — all decoder states and encoder transitions covered
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- Decoder: ASCII, Roman, Katakana, Lead_Byte, Trail_Byte, Escape, Escape_Middle all covered.
- Decoder error paths: invalid escape, invalid lead, invalid trail.
- Encoder: ASCII output, Roman-state escape, JIS0208-state escape, end-of-stream escape.
- Encoder error path: unmapped codepoint.
- The `has_pending_` "put-back" path for escape errors should be covered.
