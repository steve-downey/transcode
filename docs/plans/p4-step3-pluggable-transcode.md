# P4-Step 3: Pluggable Transcode Pipeline

**Branch:** `p4-step3-pluggable-transcode`
**Depends on:** [p4-step1-pluggable-encode.md](p4-step1-pluggable-encode.md)
**Read first:** docs/plans/phase3-handoff.md and docs/plans/handoff-next.md

---

## Goal

Add a `transcode(decode_codec, encode_codec)` pipe adaptor that composes
decode and encode in a single pipeline for pluggable codecs.

## Context for Executing Agent

The WHATWG version is `transcode<From, To>` in
`include/beman/transcode/detail/transcode_view.hpp` (lines 14-39).  It
composes `whatwg_decode<From>` and `whatwg_encode<To>`:

```cpp
template <codec From, codec To>
struct transcode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const {
        return std::forward<R>(r) | whatwg_decode<From> | whatwg_encode<To>;
    }
    // pipe operator| ...
};
template <codec From, codec To>
inline constexpr transcode_closure<From, To> transcode{};
```

The pluggable version takes two codec *objects*:

```cpp
auto result = input | transcode(my_decoder{}, my_encoder{});
```

### Implementation

```cpp
template <decode_codec From, encode_codec To>
struct pluggable_transcode_closure {
    From from_;
    To to_;
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const {
        return std::forward<R>(r) | decode(from_) | encode(to_);
    }
    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const pluggable_transcode_closure& self) {
        return self(std::forward<R>(r));
    }
};

template <decode_codec From, encode_codec To>
constexpr pluggable_transcode_closure<From, To> transcode(From from, To to) {
    return {from, to};
}
```

This overloads the existing `transcode` name.  The WHATWG version is a variable
template (`transcode<From, To>` with enum NTTPs); the pluggable version is a
function (`transcode(from_obj, to_obj)` with codec objects).  These don't
conflict because the WHATWG version requires `<>` template arguments.

## Deliverables

- Modify `include/beman/transcode/detail/transcode_view.hpp`:
  - Add `pluggable_transcode_closure` struct
  - Add `transcode(From, To)` factory function
- `tests/beman/transcode/pluggable_transcode.test.cpp` — tests

## Procedure

1. Create branch from `main`
2. Add `pluggable_transcode_closure` and factory function to `transcode_view.hpp`
3. Include `<beman/transcode/decode_view.hpp>` and `<beman/transcode/encode_view.hpp>`
   in transcode_view.hpp (needed for `decode()` and `encode()` calls)
4. Write test: `input | transcode(table_codec<latin1>{}, table_codec<utf8>{})` —
   but note `table_codec` may not implement a full UTF-8 encoder; use a simpler
   test case where decode and encode use the same single-byte table (identity
   round-trip)
5. Write consteval test
6. `make test` + `make lint`
7. Update `handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff

Step 3 done.  The pluggable codec API surface is now complete (decode views,
encode views, bulk ops, transcode pipeline).
