# P4-Step 2: Pluggable Bulk Operations

**Branch:** `p4-step2-pluggable-bulk`
**Depends on:** [p4-step1-pluggable-encode.md](p4-step1-pluggable-encode.md)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Add `decode_to`, `encode_to`, `decode_into`, `encode_into` overloads that
accept pluggable codec objects (not just the WHATWG `codec` enum).

## Context for Executing Agent

The existing bulk operations in `include/beman/transcode/detail/bulk_transcode.hpp`
(lines 59-116) are template functions constrained on `codec C` (the WHATWG enum).
They use `whatwg_decode<C>` / `whatwg_encode<C>` internally.

The pluggable versions are additional overloads constrained on `decode_codec` /
`encode_codec` concepts, using `decode(codec)` / `encode(codec)` from Step 1.

### Signature difference

WHATWG:     `decode_to<C>(range)` — enum as NTTP, no runtime codec argument
Pluggable:  `decode_to(codec, range)` — codec object as first argument

This is intentional: the codec object may carry state (e.g., a custom table
pointer), while the WHATWG enum is a compile-time constant.

### Fast path opportunity

For codecs satisfying `random_access_decode_codec_type`, `decode_to` can use
the `decode_byte()` method directly instead of going through the view, matching
the WHATWG single-byte fast path in the existing code.

## Deliverables

- Modify `include/beman/transcode/detail/bulk_transcode.hpp`:
  - `decode_to(Codec, R&&) → std::vector<char32_t>` (constrained on `decode_codec`)
  - `encode_to(Codec, R&&) → Container` (constrained on `encode_codec`)
  - `decode_into(Codec, R&&, Output)` (constrained on `decode_codec`)
  - `encode_into(Codec, R&&, Output)` (constrained on `encode_codec`)
- `tests/beman/transcode/pluggable_bulk.test.cpp` — runtime and consteval tests

## Procedure

1. Create branch `p4-step2-pluggable-bulk` from `main`
2. Add four new function templates to `bulk_transcode.hpp` after the existing
   WHATWG overloads
3. Constrain with `decode_codec` / `encode_codec` concepts from `codec_concepts.hpp`
4. Implement `decode_to(codec, range)`:
   - If `random_access_decode_codec_type`, use direct `codec.decode_byte()` loop
   - Otherwise, iterate `range | decode(codec)` and collect
5. Implement `encode_to(codec, range)`:
   - Iterate `range | encode(codec)` and collect
6. Implement `decode_into` and `encode_into` with output iterators
7. Write tests using `table_codec` through all four operations
8. Write consteval tests using `constify()`
9. Run `make test` + `make lint`
10. Update `handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 3

Step 2 done, next read `docs/plans/p4-step3-pluggable-transcode.md`.
