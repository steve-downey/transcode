# P4-Step 6: Error Enum Coherence

**Branch:** `p4-step6-error-coherence`
**Depends on:** Steps 1-5 (all new APIs should exist before cleaning up naming)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Resolve the naming divergence between `whatwg_error`, `decode_error`, and
`iconv_error`.  Either unify, rename, or explicitly document why they differ.

## Context for Executing Agent

Three error types currently exist:

### whatwg_error (include/beman/transcode/detail/error.hpp)

```cpp
enum class whatwg_error {
    invalid_byte,
    truncated_sequence,
    overlong_encoding,
    surrogate_code_point,
    out_of_range,
    unmapped_codepoint,
};
```

Used by: `whatwg_decode_or_error`, `whatwg_encode_or_error`, and transitively
by pluggable codec `_or_error` views.

### decode_error (include/beman/transcode/detail/codec_result.hpp)

```cpp
using decode_error = whatwg_error;
```

This is just a type alias — `decode_error` IS `whatwg_error`.  Used in
`decode_result` and `encode_result` structs, and in the pluggable codec
concept signatures.

### iconv_error (include/beman/transcode/detail/error.hpp)

```cpp
enum class iconv_error {
    invalid_sequence,
    incomplete_sequence,
    output_full,
};
```

Used by: `iconv_transcode_or_error_view` and (after Step 4)
`iconv_transcode_to_or_error`.

### Why they differ

`whatwg_error` has 6 values reflecting WHATWG spec error conditions —
semantically rich (overlong? surrogate? out of range?).

`iconv_error` has 3 values reflecting POSIX `errno` semantics — the OS
doesn't distinguish WHY a byte is invalid, just that it IS (`EILSEQ`),
incomplete (`EINVAL`), or couldn't fit (`E2BIG`).

These are fundamentally different abstraction levels.  Full unification
would lose semantic specificity.

### The problem

The `decode_error` alias is confusing: it's named as if it's a generic error
type, but it's literally `whatwg_error`.  The pluggable codec path uses
`decode_error` in struct definitions (`decode_result`, `encode_result`),
which means pluggable codecs report WHATWG-specific error categories even
though they have nothing to do with WHATWG.

## Deliverables

- Decision documented in code comments in `detail/error.hpp`
- If `decode_error` alias is removed: update all references in
  `codec_result.hpp`, `decode_view.hpp`, `encode_view.hpp`, `table_codec.hpp`
- If renamed: consistent new name throughout
- Update README API matrix error row to reflect final state
- Update `papers/transcode-view.md` API matrix error row

## Procedure

1. Create branch `p4-step6-error-coherence` from `main`
2. Run `grep -rn "decode_error" include/` to find all uses of the alias
3. Evaluate: is `decode_error` used in any user-facing API (public function
   signatures, view value_types)?
4. Decision tree:
   a. If only internal (struct fields, concept requirements): rename to
      `codec_error` or just use `whatwg_error` directly
   b. If user-facing: consider whether `codec_error` is clearer than
      `decode_error` (since it's used for encode errors too)
5. Add a documentation comment block in `detail/error.hpp` explaining:
   - `whatwg_error`: WHATWG Encoding Standard error categories
   - `iconv_error`: POSIX iconv errno mappings
   - Why they are separate (different abstraction levels)
   - Which views use which error type
6. Update README API matrix (line ~210) error row
7. Update `papers/transcode-view.md` API matrix error row
8. Run `make test` + `make lint`
9. Update `handoff-next.md` — Phase 4 complete

## Verification

```bash
make test
make lint
```

## Handoff (Phase 4 Complete)

Phase 4 complete.  All API surface gaps from the README matrix are closed:
- Pluggable codecs: encode views, bulk operations, transcode pipeline
- iconv: bulk operations, null-terminated input
- Error enums: documented/unified

To verify the final state, check the README API Surface Matrix — all 🔴
cells from Phase 4 scope should now be ✅.
