::: wording

## Encoders [transcode.whatwg.encoders] {-}

[#]{.pnum} *Remarks*: `whatwg::encoder<C>` is the encoder, as a codec object, for the encoding `C`.  `$whatwg-encode-codec$<C>` is `true` for every encoding but `utf_16be`, `utf_16le`, `replacement` and `x_user_defined`, for which the Encoding Standard defines no encoder, and `$random-access-encode-codec$<C>` is `true` when `C` is one of the Encoding Standard's single-byte encodings other than `x_user_defined`.  It models `text_encoder` ([transcode.codec.reqs]) and the encode adaptor over it ([transcode.codec.encode]) yields what `whatwg::encode<C>` does.  It models `flushable_text_encoder` when `C` is `encoding::iso_2022_jp`, which owes an escape sequence returning to ASCII after the last scalar value.  It models `single_byte_text_encoder` when `C` names a single-byte encoding.  It is equality comparable.

```cpp
constexpr encode_result encode_one(char32_t scalar);
```

[#]{.pnum} *Effects*: Encodes `scalar`, which is a Unicode scalar value, as the encoding `C` says, updating the encoder's state if `C` has one.

[#]{.pnum} *Returns*: The encoded bytes, or, if `C` has no representation for `scalar`, a result with `is_error`.  An error from `iso_2022_jp` carries the bytes that replace `scalar`: an escape sequence returning to ASCII if the output was not in it, then `'?'`.

```cpp
constexpr optional<unsigned char> encode_byte(char32_t scalar) const;
```

[#]{.pnum} *Constraints*: `C` models `$random-access-encode-codec$`.

[#]{.pnum} *Returns*: The byte `scalar` encodes to in the encoding `C`, or `nullopt` if `C` has no representation for it.  Present for the single-byte encodings.

```cpp
constexpr optional<encode_result> flush();
```

[#]{.pnum} *Constraints*: `C == encoding::iso_2022_jp` is `true`.

[#]{.pnum} *Effects*: Returns the encoder to its initial state.

[#]{.pnum} *Returns*: `nullopt` if the output is already in ASCII.  Otherwise the escape sequence that returns it there.

:::
