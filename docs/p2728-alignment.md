<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->

# Tracking P2728: UTF Transcoding alignment

This document records how `beman.transcode` relates to WG21
[P2728 "Unicode in the Library, Part 1: UTF Transcoding"][p2728] (Eddie Nolan;
reference implementation [`beman.utf_view`][utf_view]), and what we should do to
**follow that model as closely as possible while tracking its evolution**.

Latest revision reviewed: **P2728R17** (2026-11-18), design-approved by LEWG
on 2026-11-19. When a new revision lands,
update the [Revision history](#revision-history-of-the-model) and
[Action items](#action-items) sections below.

[p2728]: https://isocpp.org/files/papers/P2728R17.html
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

## The model in brief (P2728R17)

- **One view template:** `to_utf_view<V, K, charT>`, where `V`'s value type is
  a `code-unit` (`char8_t`/`char16_t`/`char32_t`), `charT` is the target
  code-unit type, and `K` is a `to_utf_view_kind` constant.
- **Error handling is a template parameter, not separate classes:**
  `enum class to_utf_view_kind : unspecified { replacement, expected }` —
  modeled on `subrange_kind`, and named `_kind` rather than `_error_kind` so
  that later kinds need not be about errors. `replacement` substitutes U+FFFD;
  `expected` yields `expected<charT, utf_transcoding_error>`.
- **`std::utf_transcoding_error`** — eight enumerators
  (`truncated_utf8_sequence`, `unpaired_high_surrogate`, `unpaired_low_surrogate`,
  `unexpected_utf8_continuation_byte`, `overlong`, `encoded_surrogate`,
  `out_of_range`, `invalid_utf8_leading_byte`); **unspecified underlying type**.
- **Range adaptor objects:** `views::to_utf<charT>` / `to_utf_or_error<charT>`;
  `to_utf8/16/32` and their `_or_error` forms are defined in the synopsis as
  copies of those, after `views::keys`/`views::pairwise`. **No CTAD:** the tag
  types and the deduction guide are gone, following `elements_view`,
  `adjacent_view` and `adjacent_transform_view`.
- **Iterator:** `expected<inplace_vector<charT, N>, utf_transcoding_error> buf_`,
  plus `buf_index_` and `to_increment_`; three iterators (`begin_`/`current_`/
  `end_`) for the bidirectional case; a converting constructor from
  `iterator<!Const>`; *simple-view* `begin()`/`end()`.
  `iterator_concept` goes up to `bidirectional_iterator`, and to
  `random_access_iterator` for UTF-32→UTF-32. **`iterator_category` is
  always `input_iterator_tag`, present only over a forward base,** because
  `operator*` returns a prvalue.
- **`base()` and `base_code_units()`, forward ranges only.** `base()` is the
  start of the input subsequence that produced the current element;
  `base_code_units()` is `subrange(current_, next(current_, to_increment_))`,
  that subsequence.
- **Code-unit cast adaptors:** `as_char`, `as_wchar_t`, `as_char8_t`,
  `as_char16_t`, `as_char32_t`, restricted to same-width integral and
  `std::byte` sources, and returning the range itself (not a `transform_view`)
  when it already has the target element type, which keeps contiguity.
- `null_term`; rejects raw arrays (string-literal null-terminator trap);
  does not cache `begin()`; double-transcode elision in the adaptor, for the
  replacement kind only; `<ranges>` includes `<expected>`; feature macro
  `__cpp_lib_unicode_transcoding`.

## Revision history of the model

The evolution we are tracking, most recent first:

| Rev | Change | Relevance here |
|-----|--------|----------------|
| **R17** | `iterator_category` always `input_iterator_tag` (prvalue `operator*`); converting constructor from `iterator<!Const>`; *simple-view* `begin()`/`end()`; non-const `empty()` and its missing constraint; sentinel `==` tests the iterator's base for forwardness; no elision for `_or_error`; `utf_transcoding_error` in `std::`; `reserve_hint` result unspecified; `K`/`charT` parameter names; code-unit adaptors keep contiguity; "input subsequence" defined in a General subclause. | `iterator_category` applies here directly; the rest is wording or the iterator's const shape. |
| **R16** | Removed the `to_utf_tag` types and the deduction guide (no CTAD, after `elements_view`); `to_utfN` defined as copies of `to_utf<charN_t>`; code-unit adaptor rule stated by width, with a standard-guarantee table and a typical-platform table; buffer sized by target type. | Confirms we need no CTAD machinery. |
| **R15** | Reverted SIMD support, with a design discussion of why; renamed `to_utf_view_error_kind` → `to_utf_view_kind`; added `it.base_code_units()` for forward ranges; random access for UTF-32→UTF-32; code-unit adaptors restricted to same-width integral and `std::byte` sources. | `base_code_units()` is now an iterator member — see [base-position](#base-position). |
| **R14** | Loosened the wording to admit chunked (SIMD) implementations over forward ranges, respecifying `base()` and equality positionally; design discussion of `base_code_units()`. | Superseded by R15's revert. |
| **R13** | Both enums given **unspecified underlying type** (precedent `memory_order`/`launch`/`assertion_kind`); remove `base()` for input-not-forward; add `as_char`/`as_wchar_t` (reverse-direction casts); wording fixes. | Latest polish to match. |
| **R12** | Removed the iterator's **back-pointer to parent view**, restoring the three-iterator design → **borrowedness** (precedent: `views::adjacent<N>`). | Borrowed-range correctness. |
| **R11** | `std::nontype` → **`std::constant_wrapper`** (P3948R1) for the compile-time error-kind. | CTAD plumbing. |
| **R10** | Replaced separate `to_utfX_view` classes with a **single `to_utf_view`** + tag constructors; replaced `bool OrError` with the **`to_utf_view_error_kind` enum**; stopped caching `begin()`; reject `charN_t` arrays; double-transcode elision in CPOs. | The big structural consolidation. |

### Design intent from committee review

The SG9 review at Brno (2026-06-10, on R14) settled several of the questions
the Kona review left open:

- **`base()` for forward ranges**, at the price of decoding a code point twice
  or keeping an extra iterator: unanimous consent.
- **`base_code_units()` for forward ranges**, as pure syntactic sugar for
  `subrange(it.base(), next(it).base())`: strong consensus (1/4/0/1/0). For
  input ranges it was rejected unanimously, since it would need a cache with
  lifetime problems.
- **Change the error-kind enum to allow more options later**: unanimous
  consent. This produced the `to_utf_view_kind` rename.
- **Borrowed when the base is borrowed**, at the price of three iterators:
  strong consensus.
- **Leave implementations free to buffer ahead for SIMD**: strong consensus
  *against*.
- Action item: UTF-32→UTF-32 random access when the base is.

SG16 forwarded R15 to LEWG by unanimous consent (2026-10-14), asking that the
code-unit adaptors be gated on representability of the code points rather than
on the width of the types.

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
  was discussed then as a future bolt-on. Brno later adopted it as an iterator
  member for forward ranges (see above), so the guidance that it would not be
  a new iterator member no longer holds.
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
  (`expected<char32_t, whatwg::error>`) — and **no** error-query member on the
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
  (`bulk_transcode.hpp`, `whatwg/bulk.hpp`, `iconv_bulk.hpp`) — the structure SG9 is drifting
  toward for SIMD/throughput.
- **Strongest iterator category per codec** — `decode_view` and `encode_view`
  over a single-byte codec model `random_access_range` rather than a uniform
  floor.
- **Same-codec transcode is not a no-op** (it sanitizes) — README §6.2 mirrors
  P2728 §6.2.

## Where it diverges, and how to track

### 1. Collapse the `_view` / `_or_error_view` pairs into one template — **done**

Closed in P5-Step 3c, which made each of the four families one template,
parameterized on `transcode_error_kind { replacement, expected }` (in
`error.hpp`), with the `_or_error` spellings kept as alias templates and the
closure objects selecting the kind.  At that point the templates were:

    whatwg_decode_view<C, R, E>       whatwg_encode_view<C, R, E>
    decode_view<Codec, R, E>          encode_view<Codec, R, E>

Since the SG16 review response the enum is `error_mode`, and the WHATWG view
templates are gone: `whatwg::decode<C>` and `whatwg::encode<C>` are the
codec-object adaptors over `whatwg::decoder<C>` and `whatwg::encoder<C>`, so
the two generic templates are all that remain.

The enum was named `_kind` and left open for the reason SG9 gave: a future kind
that also surfaces the offending input bytes should be addable without a third
class template. Errors stay in the `value_type`; no error-query member was added
to any iterator.

What the pairs had been, for the record:

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

### 2. `iterator_category` of a prvalue iterator (R17) — **done**

`decode_view` and `encode_view` iterators return their elements by value, so
they meet at most the C++17 input-iterator requirements. Their
`iterator_category` was `iterator_concept` (forward) on the generic views and
`random_access_iterator_tag` on the single-byte random-access views, which told
C++17 algorithms they could take multiple passes through references that do not
exist. It is now `input_iterator_tag`, present only over a forward base, as in
R17 and as for `iota_view` and `enumerate_view`. `iterator_concept` is
unchanged.

### <a id="base-position"></a>3. `base()` position and `base_code_units()` (R15, Brno) — **done**

Our decode iterator decodes eagerly, so `base()` returned the position *after*
the current element, where P2728's returns the start of it. P2728 also now has
`base_code_units()` as an iterator member for forward ranges, and no `base()`
for input ranges.

Now matched. Over a forward base, the `decode_view` and `encode_view`
iterators keep the position where the current element's input began
(`start_`, absent over an input-only base), `base()` returns it, and
`base_code_units()` is `subrange(start_, current_)`. Input-only iterators have
neither. The single-byte random-access iterators already pointed at the
current element; their `base_code_units()` is that one element.

A legacy codec has cases UTF does not, settled as follows:

- **Input that yields nothing** (`is_empty`: an ISO-2022-JP escape, a byte
  order mark through `decode_first`) counts as input to the element that
  follows it. Successive elements' inputs are therefore adjacent, and
  `base_code_units()` is `subrange(it.base(), next(it).base())`, which is the
  identity Brno polled. Such input after the last element belongs to none, and
  `base()` of an iterator equal to `end()` is the end of the base range.
- **An element yielded without consuming input** — the second of Big5's two
  scalar values for `88 62`, or a second `flush()` result — has an empty
  `base_code_units()`.
- **Encode:** every byte of one scalar value's encoding has that scalar value
  as its `base_code_units()`; bytes yielded by `flush()` have an empty range at
  the end of the base.

The iconv views are not changed: their `base()` is how far iconv has consumed,
which runs ahead of the output being yielded, since iconv converts a buffer at
a time and does not say which input produced which output byte.

### 4. Unspecified underlying type for the error enums (R13)

`whatwg::error` currently has the implicit `int`. Match R13 by not depending on a
fixed underlying type for the error / error-kind enums.

### 5. Verify `base()` semantics

Checked: the decode `iterator::base()` returned the next boundary, not the start
of the current code point. Now tracked as [base-position](#base-position).

### Intentional, non-convergent differences (do not "fix" toward P2728)

- The uniform `buf_`/`to_increment_` small-buffer iterator and **bidirectional**
  traversal assume *stateless* UTF-N. The repo's CJK codecs (ISO-2022-JP,
  GB18030 replay) are inherently **stateful and forward-only**; bidirectional
  like P2728 is not achievable for them.
- **`empty()` is not forwarded to the base.** R17's `to_utf_view::empty()`
  returns `ranges::empty(base_)`, which holds for UTF because non-empty input
  always yields at least one element. A legacy codec can consume input and
  yield nothing — ISO-2022-JP input that is only `ESC ( B` decodes to an empty
  range — so our views keep `view_interface`'s `empty()`.
- **The iterator is not a `Const` template.** R17 has `iterator<bool Const>`,
  a converting constructor from `iterator<!Const>`, and *simple-view*
  `begin()`/`end()`. Our views use one iterator type for both, built from
  `detail::compatible_iterator_t`. The observable behavior is the same; revisit
  only if wording review asks for the standard's shape.
- `as_charN_t` cast adaptors are pointless for a byte-in/`char32_t`-out pipeline —
  unless added deliberately as the interop seam to feed `char`-stored UTF into a
  P2728-typed pipeline.
- **No unified error codes across backends.** P2728 has a single error enum,
  `utf_transcoding_error`, because it covers UTF only. Here `whatwg::error` and
  `iconv_error` stay separate, and there is no shared error code or coarse
  category spanning them. Two cases settled this:
  - `output_full` (`E2BIG`) is an artifact of iconv's fixed output buffer. It
    says nothing about the input data, and no other backend can produce it.
  - An unmapped code point is impossible for a UTF target, because every
    scalar value has a UTF encoding. The legacy encoders still need it.

  A unified scheme would make every client handle, forever, cases that cannot
  happen for its conversion. Doing it properly would take a paper of its own.
  This is about the error *codes*. Unifying the error *kind* (replacement vs
  `expected`, item 1 above) is a different axis, and it is done.

## Action items

- [x] Unify `whatwg_decode_view` + `whatwg_decode_or_error_view` (and the encode
      and pluggable counterparts) under one error-kind-parameterized template.
      Done in P5-Step 3c.
- [ ] Give the error / error-kind enums an unspecified underlying type. Nothing
      to change in the code — neither enum names an enum-base — so this is a
      constraint on the *wording*, which must not fix one either.
- [x] Make `iterator_category` `input_iterator_tag`, present only over a
      forward base (R17).
- [x] Return the start of the current element from `iterator::base()`, add
      `base_code_units()`, and drop `base()` for input-only bases
      ([base-position](#base-position)).
- [ ] Make the closures' `operator()` `static` (C++23) — small QoI.
- [ ] Update the paper's related-work citation and the README link from
      P2728R13 to R17, and add R14–R17 to the bibliography.

## Future bolt-ons that the model leaves open

- A SIMD/bulk "kind" or separate view, kept out of the lazy `operator++`.
