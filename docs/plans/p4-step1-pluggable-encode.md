# P4-Step 1: Pluggable Encode View

**Branch:** `p4-step1-pluggable-encode`
**Depends on:** None (first step in Phase 4)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/phase4-index.md

---

## Goal

Add `encode(codec)` and `encode_or_error(codec)` pipe adaptors for any type
satisfying the `encode_codec` concept.  This closes the biggest single gap
in the pluggable codec API surface.

## Context for Executing Agent

The pluggable decode view already exists at `include/beman/transcode/decode_view.hpp`.
It has two variants:
- `decode_view<Codec, R>` (general, input/forward range)
- `random_access_decode_view<Codec, R>` (for `random_access_decode_codec_type`)

And two factory functions:
- `decode(codec)` → returns the appropriate view
- `decode_or_error(codec)` → returns the `_or_error` variant

The encode view must mirror this structure.

### Key files to reference

- `include/beman/transcode/decode_view.hpp` — the pattern to follow (structure,
  CPO closures at lines 307-355, factory functions at lines 355-360)
- `include/beman/transcode/whatwg_encode_view.hpp` — the WHATWG encode view
  (lines 370-574 for class templates, lines 448-468 for CPO pattern, lines
  956-964 for the dispatcher that selects random_access vs general)
- `include/beman/transcode/detail/codec_concepts.hpp` — the `encode_codec`
  concept (line 29) and `encode_result` type
- `include/beman/transcode/detail/codec_result.hpp` — `encode_result` struct:
  `{ std::array<unsigned char, 8> bytes; int count; bool is_error; }`
- `include/beman/transcode/detail/table_codec.hpp` — `table_codec` provides
  both `decode_one` and `encode_one`, making it the test subject

### Key differences from decode_view

- **Input type**: `unicode_scalar_range` (range of `char32_t`), not `legacy_byte_range`
- **Output type**: `char` (bytes), not `char32_t`
- **Internal buffering**: one `char32_t` may produce 1-8 output bytes, so the
  iterator needs `buf_[8]`, `len_`, `pos_` fields
- **Concept**: constrained on `encode_codec`, not `decode_codec`

### The encode_codec concept

```cpp
template <typename C>
concept encode_codec = std::semiregular<C> &&
    requires(C& c, char32_t cp) {
        { c.encode_one(cp) } -> std::same_as<encode_result>;
    };
```

## Deliverables

- `include/beman/transcode/encode_view.hpp` — new file containing:
  - `encode_view<Codec, R>` class template (forward/input range)
  - `encode_or_error_view<Codec, R>` class template (yields `expected<char, decode_error>`)
  - `encode_closure<Codec>` struct with `operator()` and pipe `operator|`
  - `encode_or_error_closure<Codec>` struct
  - `encode(codec)` factory function returning `encode_closure`
  - `encode_or_error(codec)` factory function
- `tests/beman/transcode/encode_view.test.cpp` — runtime tests using `table_codec`
- `tests/beman/transcode/encode_view_reject.cpp` — negative compile test:
  a type that does NOT satisfy `encode_codec` should fail with a clear diagnostic
- Update `include/beman/transcode/transcode.hpp` to include the new header
- Update `tests/beman/transcode/CMakeLists.txt` for the new test targets

## Constraints

- Functions defined out-of-line in the header (body after class), per coding rules
- Include guards, angle-bracket includes, SPDX license header
- `constexpr` everything — the encode view should work at compile time
- The `_or_error` view uses `std::expected<char, decode_error>` (where
  `decode_error` is the alias for `whatwg_error`) to match the decode_or_error
  pattern

## Procedure

1. Create branch `p4-step1-pluggable-encode` from `main`
2. Create `include/beman/transcode/encode_view.hpp`:
   - Include guard: `INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP`
   - Include: `<beman/transcode/detail/codec_concepts.hpp>`,
     `<beman/transcode/detail/concepts.hpp>`, `<expected>`, `<ranges>`, etc.
   - Define `encode_view<Codec, R>` with internal byte buffer
   - Define `encode_or_error_view<Codec, R>`
   - Define `encode_closure<Codec>` and `encode_or_error_closure<Codec>`
   - Define `encode(Codec)` and `encode_or_error(Codec)` factory functions
3. Implement `encode_view::iterator::load()`:
   - Call `codec_.encode_one(*current_++)` to get `encode_result`
   - Copy `result.bytes[0..count]` into internal buffer
   - If `result.is_error`, emit `'?'` (replacement byte, matching WHATWG behavior)
4. Implement `encode_or_error_view::iterator::load()`:
   - Same but yield `std::unexpected(decode_error::unmapped_codepoint)` on error
5. Add `#include <beman/transcode/encode_view.hpp>` to `transcode.hpp`
6. Write runtime test in `tests/beman/transcode/encode_view.test.cpp`:
   - Test: `U"ABC" | encode(latin1_codec{})` produces `"ABC"`
   - Test: codepoint > 0xFF with table_codec yields `'?'` (replacement)
   - Test: `_or_error` variant returns `unexpected` for unmapped codepoints
   - Test: round-trip: `"hello" | decode(codec{}) | encode(codec{})` == `"hello"`
7. Write negative compile test: struct with no `encode_one` should fail
8. Write consteval test using `constify()` from `tests/beman/transcode/test_utilities.hpp`
9. Register tests in `tests/beman/transcode/CMakeLists.txt`
10. Run `make test`
11. Run `make lint`
12. Update `docs/plans/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 2

Step 1 done, next read `docs/plans/p4-step2-pluggable-bulk.md`.
The new `encode(codec)` pipe adaptor is available for use in Step 2's
bulk operations.
