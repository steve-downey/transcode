# P6-Step 7: U+FFFD Is Reserved, and the Cost Is Stated

**Branch:** `p6-step7-fffd-reservation`
**Depends on:** Step 0
**Finding:** S-03
**Read first:** `docs/plans/phase6-index.md`

---

## Goal

Say out loud that a custom single-byte codec cannot map a high byte to the
actual U+FFFD scalar, and put the question of whether that is acceptable in
front of SG16 rather than answering it in a header.

## Why

`random_access_decode_codec_type` requires `decode_byte(byte)` to return
"what `byte` alone decodes to, and U+FFFD if it decodes to nothing"
(`codec_concepts.hpp:35-43`).  The `_or_error` path then reads that value back
(`decode_view.hpp:278-280`):

```cpp
char32_t cp = codec_.decode_byte(byte);
if (cp == U'\xFFFD' && byte >= 0x80)
    return error_result(whatwg_error::invalid_byte);
```

So U+FFFD is doing double duty: it is both a scalar value a codec might
legitimately want to produce and the in-band signal that it produced nothing.
A custom codec whose byte `0xFE` really does mean U+FFFD reports an error
instead, in `_or_error` mode only -- the replacement-mode view returns the
value correctly, so the two modes disagree about the same byte.

The `byte >= 0x80` guard limits the damage to high bytes, which is why this has
not bitten anything: no WHATWG single-byte index maps a high byte to U+FFFD.
It is a real limit on the *extension* point, though, and the extension point is
part of what the paper proposes.

The review offers three fixes.  Returning a result type from `decode_byte`, or
adding a separate validity query, both change the concept that makes
`random_access_range` decoding possible -- and the whole reason
`decode_byte` returns a bare `char32_t` is that it is the cheap indexed path.
Paying a result type per byte on the random-access path to serve a mapping no
real codec has is the wrong trade to make unilaterally.

So: document it, cost it, and ask.  If SG16 wants the mapping, the concept
changes then, with a reason.

## What to change

No behaviour.  Three docblocks and a paper paragraph.

- `include/beman/transcode/codec_concepts.hpp`, on
  `random_access_decode_codec_type` — state that U+FFFD is reserved as the
  "decodes to nothing" signal for bytes `0x80` and above, and that a codec
  therefore cannot map such a byte to U+FFFD itself.  Say it as a requirement
  on the codec author, since that is who it constrains.
- `include/beman/transcode/decode_view.hpp`, on the `_or_error`
  `operator*` — a `\remarks` saying that a high byte decoding to U+FFFD is
  reported as `invalid_byte`, and cross-referencing the concept.  This is the
  place a reader finds the behaviour; it should not be the place they have to
  infer the rule.
- Note the asymmetry explicitly: replacement mode returns U+FFFD and
  `_or_error` mode reports `invalid_byte` for the same byte and the same codec.
  A reader who discovers that themselves will file it as a bug.

## Paper

`papers/transcode-view.md` has a "Questions for SG16" section.  Add this to it,
briefly: the reserved sentinel, the mapping it forecloses, the three fixes, and
the reason for not choosing one yet.  Step 8 owns the paper, so either land the
paragraph here and tell Step 8, or hand Step 8 the text -- but do not leave it
unwritten, because the point of this step is the question, not the docblock.

## Tests

A custom codec in `tests/beman/transcode/codec_concepts.test.cpp` (or
`decode_view.test.cpp`, wherever the existing custom-codec fixtures live) that
maps `0xFE` to U+FFFD, asserting the documented split: U+FFFD from the
replacement view, `invalid_byte` from the `_or_error` view.

This test pins current behaviour rather than desired behaviour, which is
unusual and worth a comment saying so.  If SG16 answers, the test changes with
the concept -- and it is the thing that will tell whoever makes that change
exactly what they are changing.

## Done when

- The reservation is stated on the concept and its consequence on the view.
- The mode asymmetry is documented, not merely present.
- A test pins both modes for a codec that maps a high byte to U+FFFD.
- The SG16 question is written down.
- `make test` and `make lint` pass.
