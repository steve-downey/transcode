# P6-Step 1: `sniff_encoding` Requires a Forward Range

**Branch:** `p6-step1-sniff-forward-range`
**Depends on:** Step 0
**Finding:** C-01
**Read first:** `docs/plans/phase6-index.md`

---

## Goal

Make the docblock true.

## Why

`sniff_encoding` is constrained on `legacy_byte_range`, which requires only
`input_range`.  It then does `++it` at `sniff.hpp:44` and again at `:49` while
reading up to three bytes -- so on a single-pass range, two bytes are gone by
the time it returns.  Its own `\remarks` at lines 30-35 says:

> The mark itself is not consumed -- a decode view strips a leading U+FEFF for
> the UTF codecs on its own -- so the range can be passed on unchanged.

For `std::ranges::istream_view<char>`, sniffing a BOM and then decoding gives
you the payload minus two bytes, silently.  Sniffing a *non*-BOM stream, which
is the common case, costs the same two bytes off the front of the document.
That is data loss in exactly the stream-oriented use the function exists for.

Of the three fixes the review offers -- require forward, return an advanced
range, or make consumption part of the contract -- requiring forward is the one
that keeps the promise rather than retracting it.  Returning a replayable range
changes a one-word predicate into a two-value result for every caller, and
admitting consumption makes the composition in the paper (`sniff`, then decode
the same range) wrong.

## What to change

`include/beman/transcode/sniff.hpp`.  Add `std::ranges::forward_range<R>` to
the constraint on both the declaration (line 22-23) and the definition (line
36-37); they must stay identical or the declaration is not a declaration of the
definition.

The docblock needs one added sentence saying the range must be a forward range
*because* the mark is not consumed -- the constraint and the promise are the
same fact, and a reader who sees only one of them will wonder about the other.

Nothing else changes.  The body is already correct for a forward range.

## Tests

`tests/beman/transcode/sniff.test.cpp` and a new negative compile test.

1. **Runtime** — add cases over a forward-but-not-contiguous range, so the fix
   is exercised on something that is not a `span`.  A
   `std::forward_list<char>` holding `EF BB BF` and one holding two bytes
   that are not a BOM.  Confirm the range is intact afterwards by decoding it
   and comparing against the whole input.
2. **Negative compile** — `sniff_reject_input_range_fail.cpp`, feeding a
   genuinely single-pass range.  Hand-roll the input-only range rather than
   using `istream_view`, so the test does not depend on a stream and stays
   `constexpr`-clean; the existing `sniff_reject_char32_range_fail.cpp` is the
   shape to copy.  Register it in `tests/beman/transcode/CMakeLists.txt`
   following the block at lines 935-958: an `OBJECT` library with
   `EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true`, an `add_test` that
   invokes `${CMAKE_COMMAND} --build ... --target ...`, and a
   `PASS_REGULAR_EXPRESSION`.  The expression must name `forward_range`, not
   `legacy_byte_range` -- otherwise the test passes when the *wrong* constraint
   fires and tells you nothing.
3. **Consteval** — `constify(sniff_encoding(...))` for a BOM and a non-BOM
   case, per the `constify()` convention in
   `tests/beman/transcode/test_utilities.hpp`.

## Watch for

`views::null_term(ptr)` produces a `null_term_view` over a
`contiguous_iterator`, which satisfies `forward_range`, so the
`null_term | sniff` composition keeps working.  Check that it still compiles
before assuming so; a `null_term_view`'s `end()` is `null_sentinel_t`, and
`forward_range` only needs the iterator, not a common range.

## Done when

- `sniff_encoding` rejects a single-pass range with a diagnostic naming
  `forward_range`.
- The three test kinds above exist and pass.
- `make test` and `make lint` pass.
