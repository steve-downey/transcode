::: wording

## Decoders [transcode.whatwg.decoders] {-}

[#]{.pnum} *Remarks*: `whatwg::decoder<C>` is the decoder, as a codec object, for the encoding `C` the Encoding Standard defines.  It models `text_decoder` ([transcode.codec.reqs]) for every encoding, and the decode adaptor over it ([transcode.codec.decode]) yields what `whatwg::decode<C>` does.  It models `single_byte_text_decoder` when `C` names an encoding that decodes one byte to one scalar value, `flushable_text_decoder` when `C` names an encoding whose decoding can leave a result owed after the last byte, and `bom_aware_text_decoder` when `C` names a UTF encoding: its `decode_first` discards one leading U+FEFF BYTE ORDER MARK from the text it decodes.  `$strips-leading-bom$<C>` is `true` when `C` is `utf_8`, `utf_16be` or `utf_16le`, and `$random-access-decode-codec$<C>` is `true` when `C` is `x_user_defined` or one of the Encoding Standard's single-byte encodings.  It is equality comparable.

```cpp
template<input_iterator I, sentinel_for<I> S>
constexpr decode_result decode_one(I& current, S end);
```

[#]{.pnum} *Effects*: Decodes one scalar value from `[current, end)` as the encoding `C` says, advancing `current` past the bytes it consumed.

[#]{.pnum} *Returns*: The scalar value or the error, or, for an escape sequence that produced no scalar value, a result with `is_empty`.

```cpp
template<input_iterator I, sentinel_for<I> S>
constexpr decode_result decode_first(I& current, S end);
```

[#]{.pnum} *Constraints*: `C` models `$strips-leading-bom$`.

[#]{.pnum} *Effects*: Decodes one scalar value as `decode_one(current, end)` does.  If that is U+FEFF, it is discarded and, unless the input is exhausted, the next scalar value decoded in its place.

[#]{.pnum} *Returns*: The scalar value or the error, or, for a U+FEFF discarded with no input after it, a result with `is_empty`.

```cpp
constexpr char32_t decode_byte(unsigned char byte) const;
```

[#]{.pnum} *Constraints*: `C` models `$random-access-decode-codec$`.

[#]{.pnum} *Returns*: `0xFFFD` if `byte` decodes to nothing in `C`, otherwise the scalar value `byte` decodes to.

```cpp
constexpr optional<decode_result> flush();
```

[#]{.pnum} *Constraints*: `same_as<detail::decode_state_t<C>, detail::no_decode_state>` is `false`.

[#]{.pnum} *Effects*: Decodes what the end of the input leaves owed, from an empty range.

[#]{.pnum} *Returns*: `nullopt` if nothing is owed.  Otherwise a scalar value still pending, or the error for a sequence the input ended in the middle of.

:::
