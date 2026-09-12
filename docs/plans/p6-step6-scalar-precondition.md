# P6-Step 6: The Scalar-Value Precondition Means What It Says

**Branch:** `p6-step6-scalar-precondition`
**Depends on:** Step 0
**Finding:** S-01
**Read first:** `docs/plans/phase6-index.md`

> [!WARNING]
> **This step is on hold. Its decision is under review.**
>
> The phase index chose "keep the precondition, drop the promises."  That may
> be backwards.  The WHATWG *encoder algorithm* takes a code point stream, but
> `TextEncoder` is declared `encode(optional USVString input)`, and the Web IDL
> DOMString-to-USVString conversion replaces a lone surrogate with U+FFFD
> *before* the encoder runs.  If that conversion is part of the facility this
> library is specifying, rather than an artifact of the JavaScript binding,
> then surrogate-to-U+FFFD is **defined behaviour**, the WPT vectors below are
> normative after all, and the precondition is the thing that is wrong.
>
> The cases may also split: a value above U+10FFFF is not a code point at all,
> so no conversion is defined for it, and `char32_t` can hold one where a JS
> string cannot.
>
> Do not execute this step until that is settled.  Everything below assumes the
> original decision and is retained only so the reasoning is not lost.

---

## Goal

Stop the library both requiring that a `char32_t` be a Unicode scalar value and
promising what happens when it is not.

## Why

Two places state the precondition.  `codec_concepts.hpp:47-48`:

> `c.encode_one(cp)` returns the encoded form of `cp`, or an error if the
> encoding has no representation for it.  `cp` is required to be a Unicode
> scalar value.

And `encode_view.hpp:32-33`:

> Each element of `R` is required to be a Unicode scalar value, which is a
> precondition and not a constraint.

The paper argues for this at length under "Concepts": `unicode_scalar_range` is
a type-level concept and "cannot prove that each `char32_t` object is a Unicode
scalar value," so the encode adaptors carry a semantic precondition "matching
WHATWG's encoder hooks."  That is a deliberate design position, not an
oversight, and it is the one being kept.

Meanwhile the implementation and the tests treat a surrogate as supported input
with a defined result -- replacement in one mode, `surrogate_code_point` or
`out_of_range` in the other.  A value cannot be both outside the contract and
guaranteed a particular answer.  A caller reading the tests learns a guarantee
the wording says they do not have; a caller reading the wording is surprised by
tests that depend on one.

Keeping the precondition and dropping the guarantee is the smaller change and
the one consistent with the stated design.  The alternative -- admitting
non-scalar input as supported erroneous input -- would mean every encoder
validating every element, which is the per-element cost the paper's
`char32_t`-as-interchange argument is written to avoid.

## What to change

Nothing in the *behaviour*.  This step changes what is promised, not what
happens.

- `include/beman/transcode/codec_concepts.hpp` — keep the precondition
  sentence; make sure it reads as a requirement on the caller and not as a
  description of a check.
- `include/beman/transcode/encode_view.hpp`,
  `include/beman/transcode/whatwg_encode_view.hpp` — same.  Wherever a
  `\returns` or `\remarks` currently says what a surrogate or an out-of-range
  value produces, remove that sentence.  The `whatwg_error` enumerators
  `surrogate_code_point` and `out_of_range` stay: they are reachable from
  *decoding*, where a byte sequence really can encode a surrogate, and their
  table entries in `error.hpp:29-34` are about decoding.
- Add one `\remarks` saying that whether the implementation detects a
  precondition violation is unspecified, so that the hardening the code does is
  not mistaken for a guarantee and not mistaken for a bug either.

## The WPT tests are the hard part

`tests/beman/transcode/wpt_encoder_surrogates.test.cpp` feeds `char32_t`
surrogates straight into `whatwg_encode<codec::utf_8>` and checks the round
trip against
`tests/beman/transcode/wpt_encoder_surrogates_vectors.hpp`, which is
**generated** from `docs/wpt/textencoder-utf16-surrogates.any.js` by
`tools/generate_wpt_vectors.py`.

Do not simply delete it.  A WPT-derived vector is the strongest evidence this
project has, and the paper's Testing section makes conformance to WPT a claim.
But the vectors do not apply to this API as written, and it is worth being
clear why rather than hand-waving: WPT is testing JavaScript's `TextEncoder`,
whose input is a JS string -- UTF-16 code units, in which a lone surrogate is
representable and therefore an input the web platform must define. This
library's input is `char32_t`, and a lone surrogate in a `char32_t` is a value
the caller was required not to construct. The two APIs have different input
domains, so the same vectors ask different questions.

Reframe the test instead of removing it:

- Keep the file and the generated vectors.
- Change what it asserts.  Not "the output equals WPT's expectation," which is
  a guarantee this library no longer makes, but "the output is well-formed
  UTF-8 and the encoder terminates" -- a hardening property, true regardless of
  the contract, and the property that actually matters for a caller who gets a
  bad value in from somewhere.
- Retag it out of the conformance set: `[hardening]` rather than
  `[wpt::encoder_surrogates]`, so the WPT tag continues to mean "we match the
  web platform here" without an exception the reader has to know about.
- Put the reasoning in a comment at the top of the file.  The next person to
  read it will otherwise re-derive it or, worse, "fix" it back.

Then check the rest.  `wpt_surrogates_utf8.test.cpp`,
`wpt_utf16_surrogates.test.cpp` and `whatwg_encode_or_error.test.cpp` need
reading before editing: the *decode* direction is unaffected -- a byte sequence
encoding a surrogate is genuinely invalid input and `surrogate_code_point` is
genuinely its error -- and only the encode direction is in scope here.  Do not
touch a decode test.

## Done when

- No normative sentence promises a result for a non-scalar `char32_t`.
- The precondition survives, in both places, and reads as a caller obligation.
- `wpt_encoder_surrogates.test.cpp` asserts a hardening property, is retagged,
  and carries the reasoning.
- Every decode-direction surrogate test is untouched, and the pull request says
  so explicitly.
- `make test` and `make lint` pass.

## Commit message

Enumerate every test that changed meaning and every sentence that was removed.
This diff subtracts guarantees and subtracts assertions, which is what a
regression looks like from a distance; the message is what distinguishes them.
