# P6-Step 6: UTF-32 Input Validation, Specified and Uniform

**Branch:** `p6-step6-utf32-validation`
**Depends on:** Step 0
**Findings:** S-01, and C-08 and D-15 below, both new
**Read first:** `docs/plans/phase6-index.md`

> This step replaces `p6-step6-scalar-precondition.md`, which proposed the
> opposite.  The reasoning that changed is in the phase index under
> "S-01, resolved."

---

## Goal

Stop calling a surrogate a precondition violation, because the standards do
define what happens to one.  Then do that definition **once**, ahead of codec
dispatch, so all forty codecs agree instead of only UTF-8 being right.

## Why

### The precondition was attributed to the wrong layer

WHATWG's encoder algorithms do take scalar values, and the Encoding Standard
enforces it with a normative assertion in *process an item* (§4.1):

> Assert: encoderDecoder is not an encoder instance or **item is not a
> surrogate**.

and two lines above,

> Assert: encoderDecoder is not an encoder instance or **mode is not
> "replacement"**.

So an encoder can never run in replacement mode; the U+FFFD branch is
unreachable on the encode path, and encoder error modes are only `"fatal"` and
`"html"`.  Read alone, that supports a precondition.

But the substitution is specified anyway -- one layer up, and by *three*
different specifications, all invoking Infra's *convert a string into a scalar
value string* (Infra §4.7, "replace any surrogates with U+FFFD"):

- **Web IDL** §3.2.12, for `TextEncoder.encode(optional USVString input)`;
- **the Encoding Standard itself** §7.6, for `TextEncoderStream`, which
  deliberately takes a `DOMString` so a pair split across chunks can be
  rejoined, and whose note says "lone surrogates will be replaced with U+FFFD";
- **HTML**, in *create an entry*, which converts form names and values itself.

URL's *percent-encode after encoding* takes a `scalar value string` as a typed
precondition.  There is no WHATWG-specified path that hands a surrogate to an
encoder.

So the earlier framing -- either the encoder defines it, or it is a JavaScript
binding artifact -- was a false choice.  It is neither: the substitution is a
cross-cutting Infra convention that every entry point applies, and the codec
below never sees the case.  A C++ library that offers the entry point owns the
conversion, exactly as `TextEncoderStream` and HTML do.

### And the encoder-layer reading gives an answer nobody wants

If the UTF-8 encoder algorithm really were total over code points, the answer
it gives for a surrogate is not U+FFFD.  Encoding §8.1.2 selects by range, and
U+D800 falls inside the `U+0800 to U+FFFF` row -- so the algorithm would emit
`ED A0 80`.  That is CESU-8.  Unicode D92 says any UTF-8 sequence that would
map to U+D800–U+DFFF is **ill-formed**, and C10 says such a sequence "must
never be generated."  A value above U+10FFFF matches no row at all and the
algorithm has nowhere to go.

A conforming library therefore has exactly three options for a surrogate --
refuse it, report it, or substitute U+FFFD -- and emitting the "natural"
encoding is not among them.

### The implementation already does the right thing

`detail/utf8_encode.hpp:21-25` returns `surrogate_code_point` for U+D800–DFFF
and `out_of_range` above U+10FFFF, and `whatwg_encode_view.hpp:506-517`
substitutes `EF BF BD` in replacement mode.  That is defined, conformant, and
matches P2728, which specifies the same substitution and attributes it to the
**UTF-32 decode step** rather than to a UTF-8 encode error -- `char32_t` in,
bytes out, is two operations, and WHATWG only specifies the second.
`docs/p2728-alignment.md` already maps `surrogate_code_point` to P2728's
`encoded_surrogate`.

So the code has been right and the wording wrong, and the fix is to say what
the code does rather than to make the code match a precondition that the
library's own conformance suite deliberately violates.

### C-08: the diagnosis is not uniform (new)

The surrogate and out-of-range checks live **inside** the
`if constexpr (C == codec::utf_8)` branch.  Every other codec goes straight to
its table, and its miss path is:

```cpp
this->emit_error(whatwg_error::unmapped_codepoint, {'?'});
```

at `whatwg_encode_view.hpp:499`, `:524`, `:535`, `:546`, `:557` and `:568`.

So `whatwg_encode_or_error<codec::windows_1252>` given U+D800 reports
`unmapped_codepoint`, whose table entry in `error.hpp:35-37` reads "the
encoding has no representation for the Unicode scalar value being encoded."
Every clause of that is wrong: it is not about representation, and U+D800 is
not a Unicode scalar value.  The same input is diagnosed correctly by one codec
and misdiagnosed by the other thirty-nine.

This is a defect under *any* answer to S-01.  Under a precondition it is
undiagnosed-but-inconsistent; under defined behaviour it is simply the wrong
error.  It is fixed here because the fix -- validate once, above dispatch -- is
the same layering the standards use.

### D-15: the documentation says `'?'` and UTF-8 says otherwise (new)

Three places claim an encode failure yields `'?'`:

- `error.hpp:79` and the `transcode_error_kind` table at `:86-89`
- `encode_view.hpp:229`
- `papers/transcode-view.md:371` — "Encode failures are replaced with `'?'`",
  with no codec qualifier

`whatwg_encode<codec::utf_8>` emits `EF BF BD`.  Both outputs are defensible --
`'?'` for a character a legacy encoding cannot represent, U+FFFD for input that
was never well-formed UTF-32 -- but the docs have to say which is which, and
after this step the distinction is exactly the layer boundary.

## What to change

### Validate above dispatch

In `whatwg_encode_view`'s `load()`, test the `char32_t` **before** the
`if constexpr` codec chain:

- a surrogate or a value above U+10FFFF is not a scalar value;
- in `expected` mode, report `surrogate_code_point` or `out_of_range` --
  the real reason, for every codec;
- in `replacement` mode, substitute the **scalar value U+FFFD** and then encode
  *that* through the selected codec normally.

The second half is the part worth getting right.  Substituting the code point
and re-encoding is what Infra's conversion does, and it makes the outputs fall
out correctly on their own: UTF-8 gives `EF BF BD`, and windows-1252 gives
`'?'` because U+FFFD genuinely has no windows-1252 mapping -- an
`unmapped_codepoint` that is now *true*.  Do not special-case per codec; let
the layering produce the answer.

Then delete the checks from `detail/utf8_encode.hpp`, or keep them and make
them unreachable — but not both silently.  If they stay, say in a comment that
the caller has already validated.

`encode_view` (the pluggable-codec sibling) needs the same treatment, or an
explicit note saying a custom codec owns its own validation.  Decide and write
it down; the two views should not differ by accident.

### Say what is now guaranteed

- **Encode.**  Remove the precondition sentences from `concepts.hpp:38-41`,
  `codec_concepts.hpp:47-48`, `encode_view.hpp:32-33` and
  `papers/transcode-view.md:560-564`.  Replace with: the input is validated as
  UTF-32; a code unit that is not a Unicode scalar value yields U+FFFD in
  replacement mode and the corresponding error in `expected` mode.
- **Decode.**  Add the postcondition, which is now provable: *every element of
  the output is a Unicode scalar value*.  Encoding §4.1 asserts it, and the
  implementation upholds it -- `detail/utf8.hpp` enforces the 0xED/0x9F and
  0xF4/0x8F boundaries and `detail/utf16.hpp` errors on an unpaired surrogate.
  This is a guarantee worth stating outright rather than leaving to inference.
- **D-15.**  Correct the three `'?'` claims to distinguish an unmappable scalar
  value (`'?'`) from ill-formed UTF-32 input (U+FFFD, then encoded).

### The WPT test stays, re-attributed

`tests/beman/transcode/wpt_encoder_surrogates.test.cpp` keeps its assertions --
they were right.  What changes is the claim attached to them.

The vectors come from `docs/wpt/textencoder-utf16-surrogates.any.js`, whose own
first line is `// META: title=Encoding API: USVString surrogate handling when
encoding` and whose test names are prefixed `'USVString handling: '`.  It tests
the Web IDL coercion; per §4.1 it *cannot* test the encoder's surrogate
handling, because the encoder has none.

Add a comment saying the file exercises this library's UTF-32 validation, which
is the same algorithm at the same layer as the coercion WPT is testing -- so
the vectors are apt -- but that WPT is not evidence about a WHATWG *encoder*.

Note the split while you are there: `tools/generate_wpt_vectors.py`'s
`parse_js_string` performs the first half of Infra's conversion in Python
(combining `𝄞` into `0x1D11E`) and leaves the second half
(lone-surrogate replacement) to the C++ encoder.  One algorithm, two languages.
The consequence is that the astral vector tests nothing about surrogates at
all, and no C++ vector can exercise a pair split across chunks, because a
`char32_t` range has no code units to split.

## Tests

- U+D800 through `whatwg_encode_or_error<codec::windows_1252>` reports
  `surrogate_code_point`, not `unmapped_codepoint`.  This is C-08 and it fails
  on `main`.
- The same through `whatwg_encode<codec::windows_1252>` yields `'?'` -- via
  U+FFFD being unmappable, which is the correct route to the same byte.
- U+110000 through several codecs reports `out_of_range` uniformly.
- UTF-8 behaviour is unchanged: `EF BF BD` and `surrogate_code_point`.
- A genuine unmappable scalar value (U+4E2D to windows-1252) still reports
  `unmapped_codepoint`, so the two conditions stay distinguishable.
- Existing WPT surrogate cases keep passing untouched.

Add cases across at least one single-byte codec, one CJK multibyte codec and
ISO-2022-JP, since the last has state and the substitution happens before it.

## Done when

- Validation happens once, above codec dispatch, and every codec reports the
  same error for the same out-of-domain input.
- No document claims a precondition; the encode contract and the decode
  postcondition are both stated.
- The three `'?'` claims are corrected.
- The WPT file says which layer it tests.
- `make test`, `make lint` and `make coverage` pass.

## Hand off to Step 8

- The paper's Concepts rationale (lines ~555-564) argued for the precondition
  at length.  It needs rewriting, not deleting: the argument that
  `unicode_scalar_range` cannot prove its elements are scalar values is still
  true and still worth making -- what changes is the conclusion drawn from it.
- `papers/transcode-view.md:371` — the `'?'` claim.
- The WPT conformance claim in the Testing section should describe the
  surrogate vectors as derived from WPT's `USVString` test and exercising this
  library's UTF-32 validation, rather than as encoder conformance.  A WG21
  reviewer who knows the Encoding Standard will otherwise catch it.
- UTF-16BE/LE having no encoder is already implemented correctly
  (`detail/whatwg_encode_select.hpp:82-83` excludes them from
  `encodable_codec`), and matches Encoding §4.1 and §4.3.  State it as a
  deliberate conformance point rather than leaving it implicit.
- Before quoting P2728R14 wording in the paper, re-read
  <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p2728r14.html>
  directly.  The alignment claim here rests on it.
