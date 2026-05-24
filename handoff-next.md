# Handoff: beman.transcode — Step 20 (UTF-8 Encoder)

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

**116 C++ tests + 33 Python tests pass** (`make test`). Steps 0–19
complete. On `main`.

### What Step 19 Built

Step 19 created the encode direction for all 28 single-byte codecs:

- **`include/beman/transcode/whatwg_encode_view.hpp`** — new file
  containing `whatwg_encode_view<C, R>`,
  `whatwg_encode_or_error_view<C, R>`, their closures and global pipe
  adapters. Dispatch covers all 28 single-byte codec enum values.
  `iso_8859_8_i` shares the `iso_8859_8` table.

- **`detail/single_byte.hpp`** extended with `single_byte_encode_one()`
  and `single_byte_encode_result`. The encoder does a linear scan of
  the 128-entry table (O(128) per codepoint).

- **`detail/concepts.hpp`** extended with `unicode_scalar_range` —
  `input_range<R>` with `range_value_t == char32_t`, also rejects
  raw arrays.

- **`detail/error.hpp`** extended with `whatwg_error::unmapped_codepoint`.

- **Test files** (already exist — APPEND to them for step 20):
  - `tests/beman/transcode/whatwg_encode.test.cpp`
  - `tests/beman/transcode/whatwg_encode_or_error.test.cpp`

- **Test executables** already registered in
  `tests/beman/transcode/CMakeLists.txt`.

### Current iterator design — IMPORTANT for step 20

The `whatwg_encode_view::iterator` currently uses a simple
`char value_` field because single-byte codecs produce exactly
one byte per codepoint:

```cpp
class iterator {
    base_iter current_;
    base_sent end_;
    char      value_{};   // ← single byte only
    bool      done_{false};
    ...
};
```

**Step 20 must refactor this** to support UTF-8's variable-length
output (1–4 bytes per codepoint). Use a buffer design:

```cpp
class iterator {
    base_iter current_;
    base_sent end_;
    char      buf_[4]{};  // encoded bytes for current codepoint
    int       len_{0};    // how many valid bytes in buf_
    int       pos_{0};    // index of next byte to yield
    bool      done_{false};
    ...
};

constexpr char operator*() const { return buf_[pos_]; }
constexpr iterator& operator++() {
    if (++pos_ < len_)
        return *this;    // still bytes left from current codepoint
    load();              // advance to next codepoint
    return *this;
}
```

For single-byte codecs `load()` sets `buf_[0]`, `len_=1`, `pos_=0`.
For UTF-8 `load()` fills up to 4 bytes. The same iterator class works
for both after this refactor.

The same refactor applies to `whatwg_encode_or_error_view::iterator`,
which currently uses `result_t value_`. After the refactor it uses
`result_t buf_[4]` with the same `len_`/`pos_` logic but yielding
`expected<char, whatwg_error>`.

## What To Do Next — Step 20

**Branch:** `step20-utf8-encoder`

**Read the checklist:** `docs/plans/phase2-checklist.md`

Note: the old plan file `docs/plans/step18-utf8-encoder.md` describes
this step (it was written as step 18 before step 19 was inserted). The
plan is still accurate for the implementation except:
- `whatwg_encode_view.hpp` already exists — do not create it fresh
- `unicode_scalar_range` already exists in `detail/concepts.hpp`
- Test files and their CMake registration already exist
- Need to **refactor** the iterator to use the buffer design first,
  then add UTF-8 arms

### Implementation order

1. Refactor `whatwg_encode_view::iterator` and
   `whatwg_encode_or_error_view::iterator` to buffer design
   (single-byte arms still work with `len_=1`)

2. Create `include/beman/transcode/detail/utf8_encode.hpp`:

   ```cpp
   struct utf8_encode_result {
       char         bytes[4]{};
       int          count{};
       whatwg_error error{};
       bool         is_error{false};
   };

   constexpr utf8_encode_result utf8_encode_one(char32_t cp);
   ```

   WHATWG UTF-8 encoder:
   - U+0000–U+007F → 1 byte
   - U+0080–U+07FF → 2 bytes
   - U+0800–U+FFFF (non-surrogate) → 3 bytes
   - U+10000–U+10FFFF → 4 bytes
   - Surrogates (U+D800–U+DFFF) → error (`surrogate_code_point`)
   - > U+10FFFF → error (`out_of_range`)

3. Add `#include <beman/transcode/detail/utf8_encode.hpp>` to
   `whatwg_encode_view.hpp`

4. Add dispatch arms in `whatwg_encode_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::utf_8) {
       auto r = detail::utf8_encode_one(static_cast<char32_t>(*current_));
       ++current_;
       if (r.is_error) {
           // substitute U+FFFD encoded as UTF-8: {0xEF, 0xBF, 0xBD}
           buf_[0] = '\xEF'; buf_[1] = '\xBF'; buf_[2] = '\xBD';
           len_ = 3;
       } else {
           for (int i = 0; i < r.count; ++i) buf_[i] = r.bytes[i];
           len_ = r.count;
       }
       pos_ = 0;
   }
   ```

   And in `whatwg_encode_or_error_view::iterator::load()`:
   ```cpp
   } else if constexpr (C == codec::utf_8) {
       auto r = detail::utf8_encode_one(static_cast<char32_t>(*current_));
       ++current_;
       if (r.is_error) {
           buf_[0] = std::unexpected(r.error);
           len_ = 1;
       } else {
           for (int i = 0; i < r.count; ++i)
               buf_[i] = static_cast<char>(r.bytes[i]);
           len_ = r.count;
       }
       pos_ = 0;
   }
   ```

5. Add `detail/utf8_encode.hpp` to
   `include/beman/transcode/CMakeLists.txt` HEADERS FILES list.

6. Append UTF-8 tests to the existing test files:
   - `whatwg_encode.test.cpp`: ASCII, 2-byte, 3-byte, 4-byte, surrogate
     replaced with U+FFFD bytes, consteval
   - `whatwg_encode_or_error.test.cpp`: surrogate → unexpected,
     out-of-range → unexpected, valid codepoint → has_value()

7. Add negative compile test (`whatwg_encode_reject_char_range_fail.cpp`)
   verifying that `vector<char>` is rejected as input because it does
   not satisfy `unicode_scalar_range`. Register in
   `tests/beman/transcode/CMakeLists.txt` with PASS_REGULAR_EXPRESSION.

### Note on the `replacement` and `x_user_defined` encode codecs

Those three algorithmic codecs are NOT yet in the encode view's dispatch.
Step 20 adds only `codec::utf_8`. The `replacement` and `x_user_defined`
encode directions are deferred to a later step (or may be skipped as
they are niche). Do not add them in step 20.

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

1. Branch: `git checkout -b step20-utf8-encoder`
2. Write failing tests (RED) → commit → push both remotes
3. Implement (GREEN): refactor iterator, `utf8_encode.hpp`,
   add UTF-8 arms, add negative compile test
4. `make test` (all pass) + `make lint` (clean) → commit → push both
5. `make coverage` — new code should be covered; check `utf8_encode.hpp`
   lines specifically
6. Merge to main + push both
7. Mark checklist `[x]`

## Coverage Notes

- `utf8_encode.hpp` will have two dead-code lines: the `cp > 0x10FFFF`
  guard after the F0–F4 lead-byte check, and the `extra==1 && cp < 0x80`
  guard. These are defensive and unreachable. Do not add tests for them.
  (Same situation as `utf8.hpp` in the decoder.)
- All other lines in `utf8_encode.hpp` and the new iterator buffer logic
  should be covered.
