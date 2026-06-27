<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->

# Tracking P2728: UTF Transcoding alignment

This document records how `beman.transcode` relates to WG21
[P2728 "Unicode in the Library, Part 1: UTF Transcoding"][p2728] (Eddie Nolan;
reference implementation [`beman.utf_view`][utf_view]), and what we should do to
**follow that model as closely as possible while tracking its evolution**.

Latest revision reviewed: **P2728R13** (2026-05-26). When a new revision lands,
update the [Revision history](#revision-history-of-the-model) and
[Action items](#action-items) sections below.

[p2728]: https://isocpp.org/files/papers/P2728R13.html
[utf_view]: https://github.com/bemanproject/utf_view

## How the two relate

P2728 is **type-based** transcoding: the encoding is carried in the character
type (`char8_t` = UTF-8, `char16_t` = UTF-16, `char32_t` = UTF-32), and the
view transcodes UTF-N → UTF-M between those types. The compiler statically
tracks the encoding.

`beman.transcode` is **byte-based** transcoding: it decodes byte-like I/O
(`char`, `signed char`, `unsigned char`, `std::byte`) to `char32_t` code points,
where the codec is selected at compile time from the `codec` enum but
corresponds to a runtime/protocol fact (HTTP `Content-Type`, BOM sniff, DB
column metadata, an `iconv` label). It also wraps system `iconv`.

These are **complementary, non-overlapping** problems — the repository README
already says so. Therefore "follow the model as exactly as possible" means
matching P2728's **structural and error-handling patterns**, **not** copying its
`charN_t`-typed interface. The character-typed cast adaptors P2728 provides
(`as_char8_t`, etc.) are the natural *interop seam* between the two libraries,
not a feature gap.

## The model in brief (P2728R13)

- **One view template:** `to_utf_view<V, E, ToType>`, where `V`'s value type is
  a `code-unit` (`char8_t`/`char16_t`/`char32_t`), `ToType` is the target
  code-unit type, and `E` is a `to_utf_view_error_kind` constant.
- **Error handling is a template parameter, not separate classes:**
  `enum class to_utf_view_error_kind { replacement, expected }` — modeled on
  `subrange_kind`. `replacement` substitutes U+FFFD; `expected` yields
  `expected<ToType, utf_transcoding_error>`.
- **`utf_transcoding_error`** — eight enumerators
  (`truncated_utf8_sequence`, `unpaired_high_surrogate`, `unpaired_low_surrogate`,
  `unexpected_utf8_continuation_byte`, `overlong`, `encoded_surrogate`,
  `out_of_range`, `invalid_utf8_leading_byte`); **unspecified underlying type**.
- **CPOs:** `views::to_utf<ToType>` / `to_utf_or_error<ToType>`, plus the fixed
  `to_utf8/16/32` and their `_or_error` variants. CTAD uses
  `to_utf_tag_t<ToType>` plus a `std::constant_wrapper<E>` constructor tag.
- **Iterator:** small `inplace_vector<value_type, 4/sizeof(ToType)> buf_`, plus
  `buf_index_` and `to_increment_`; three iterators (`begin_`/`current_`/`end_`)
  for the bidirectional case; `base()` only for `forward_range`; up to
  `bidirectional_iterator`; borrowed when the base is.
- **Code-unit cast adaptors:** `as_char`, `as_wchar_t`, `as_char8_t`,
  `as_char16_t`, `as_char32_t` (thin `transform_view` casts).
- `null_term`; rejects raw arrays (string-literal null-terminator trap);
  does not cache `begin()`; double-transcode elision in the CPO; feature macro
  `__cpp_lib_unicode_transcoding`.

## Revision history of the model

The evolution we are tracking, most recent first:

| Rev | Change | Relevance here |
|-----|--------|----------------|
| **R13** | Both enums given **unspecified underlying type** (precedent `memory_order`/`launch`/`assertion_kind`); remove `base()` for input-not-forward; add `as_char`/`as_wchar_t` (reverse-direction casts); wording fixes. | Latest polish to match. |
| **R12** | Removed the iterator's **back-pointer to parent view**, restoring the three-iterator design → **borrowedness** (precedent: `views::adjacent<N>`). | Borrowed-range correctness. |
| **R11** | `std::nontype` → **`std::constant_wrapper`** (P3948R1) for the compile-time error-kind. | CTAD plumbing. |
| **R10** | Replaced separate `to_utfX_view` classes with a **single `to_utf_view`** + tag constructors; replaced `bool OrError` with the **`to_utf_view_error_kind` enum**; stopped caching `begin()`; reject `charN_t` arrays; double-transcode elision in CPOs. | The big structural consolidation. |

### Design intent from committee review

The SG9 review (Kona 2025, on R9) records the *reasoning* behind several of the
above changes — useful because it tells us which patterns are load-bearing:

- **The error-kind enum is meant to stay open-ended.** It is an enum named
  `_kind` (not a `bool`, not a type) specifically so future kinds beyond
  `{replacement, expected}` can be added (e.g. one that also surfaces the
  offending base code units).
- **Errors belong in the value type, never as an iterator member.** An earlier
  design had a `.success()`/`.error()` member on the iterator; SG9 disliked
  inventing novel iterator member functions, and that rejection is *the reason
  the `_or_error` view exists at all*.
- **`base()` / `base_code_units`.** `base()` is forward-only and points at the
  code unit at the **start of the current code point**. A `base_code_units`
  helper (`subrange(it.base(), ranges::next(it).base())`) for error reporting
  was discussed only as a **future bolt-on**, not in the paper — and would be
  expressed through the value/error, not a new iterator member.
- **Borrowedness mirrors the base range;** whether to use a smaller back-pointer
  form when the base is not borrowed is an ABI/stack-size QoI tradeoff
  (motivated by industrial users who avoid ranges over stack usage).
- **SIMD / eager read-ahead is in tension with lazy views.** The room leaned
  toward keeping bulk/SIMD a *separate* mechanism rather than baking read-ahead
  into the lazy `operator++`.
- **Iterator category should not be over-downgraded** (e.g. UTF-32 self-
  transcode should stay random access).

## Where `beman.transcode` already follows the model

- **`_or_error` paired views** with errors carried in the `value_type`
  (`expected<char32_t, whatwg_error>`) — and **no** error-query member on the
  iterator. This matches both the `_or_error` pattern and the explicit SG9
  preference that produced it.
- **U+FFFD** substitution on lossy decode.
- **`views::null_term`** mirrors `std::null_term`.
- **Raw arrays are rejected** with a targeted diagnostic (same null-terminator
  rationale as P2728 §10.1).
- **`enable_borrowed_range`** is specialized to mirror the base range, and the
  iterators hold `current_`/`end_` with **no parent back-pointer** — the R12
  direction.
- **`begin()` is not cached** (P2728 §10.3).
- **Lazy views are kept separate from bulk paths**
  (`detail/bulk_transcode.hpp`, `iconv_bulk.hpp`) — the structure SG9 is drifting
  toward for SIMD/throughput.
- **Strongest iterator category per codec** — single-byte codecs get a dedicated
  `random_access_whatwg_decode_view` rather than a uniform floor.
- **Same-codec transcode is not a no-op** (it sanitizes) — README §6.2 mirrors
  P2728 §6.2.

## Where it diverges, and how to track

### 1. Collapse the `_view` / `_or_error_view` pairs into one template (highest value)

The repository still exposes **separate** `whatwg_decode_view` and
`whatwg_decode_or_error_view` classes (and likewise for encode and the pluggable
`decode_view`/`encode_view`). That is P2728's **pre-R10** shape.

Track the model by unifying each pair into a single view template parameterized
on an **extensible error-kind enum** — the analog of `to_utf_view_error_kind`:

- Name it for extension (e.g. `decode_error_kind`/`transcode_kind`), with at
  least `{ replacement, expected }`, leaving room for future kinds.
- Give the enum an **unspecified underlying type** (R13).
- Select `E` in the lossy vs `_or_error` CPOs; keep the `_or_error` suffix.
- Use **`std::constant_wrapper`** + a tag type for CTAD (R11), and do **not** add
  a bare constructor lacking a corresponding deduction guide (SG9 kept this
  deliberately).
- Keep errors in the `value_type`; do not add an error-query iterator member.

### 2. Unspecified underlying type for the error enums (R13)

`whatwg_error` currently has the implicit `int`. Match R13 by not depending on a
fixed underlying type for the error / error-kind enums.

### 3. Verify `base()` semantics

Confirm the decode `iterator::base()` returns the position at the **start of the
current code point's bytes** (P2728's forward-range contract), not the next
boundary — the repo's decoder advances eagerly in `load()`, so this is worth a
test.

### Intentional, non-convergent differences (do not "fix" toward P2728)

- The uniform `buf_`/`to_increment_` small-buffer iterator and **bidirectional**
  traversal assume *stateless* UTF-N. The repo's CJK codecs (ISO-2022-JP,
  GB18030 replay) are inherently **stateful and forward-only**; bidirectional
  like P2728 is not achievable for them.
- `as_charN_t` cast adaptors are pointless for a byte-in/`char32_t`-out pipeline —
  unless added deliberately as the interop seam to feed `char`-stored UTF into a
  P2728-typed pipeline.

## Action items

- [ ] Unify `whatwg_decode_view` + `whatwg_decode_or_error_view` (and the encode
      and pluggable counterparts) under one error-kind-parameterized template.
- [ ] Give the error / error-kind enums an unspecified underlying type.
- [ ] Add a test pinning `iterator::base()` to the start-of-code-point byte
      position.
- [ ] Make the closures' `operator()` `static` (C++23) — small QoI.
- [ ] (Future, not now) A "which input bytes failed" capability for error
      reporting, expressed as an error-kind / error payload rather than a new
      iterator member.

## Future bolt-ons that the model leaves open

- A `base_code_units`-style accessor for error reporting (forward ranges only),
  as syntactic sugar over `subrange(it.base(), ranges::next(it).base())`.
- A SIMD/bulk "kind" or separate view, kept out of the lazy `operator++`.
