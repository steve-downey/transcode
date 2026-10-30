::: wording

## Transcoding pipelines [transcode.pipeline] {-}

```cpp
template<text_decoder From, text_encoder To>
constexpr $see below$ transcode(From from, To to);
```

[#]{.pnum} *Returns*: A range adaptor object over `from` and `to`.  Given a subexpression `E` that models `legacy_byte_range`, `transcode(from, to)(E)` and `E | transcode(from, to)` are each expression-equivalent to `E | decode(from) | encode(to)`.

[#]{.pnum} *Remarks*: `from` and `to` may be the library's codec objects (`whatwg::decoder`, `whatwg::encoder`) or a program's own, in any combination.  `whatwg::transcode<From, To>` ([transcode.pipeline]) is defined in terms of this function, with codecs named by enumerators rather than supplied as values.

```cpp
template<encoding From, encoding To> inline constexpr $unspecified$ transcode;
```

[#]{.pnum} *Remarks*: `whatwg::transcode<From, To>` is `transcode(whatwg::decoder<From>{}, whatwg::encoder<To>{})` ([transcode.pipeline]), and so is `E | whatwg::decode<From> | whatwg::encode<To>` for a subexpression `E` that models `legacy_byte_range`.  An `E` of array type is ill-formed.

[#]{.pnum} The composition is the specification: the pipeline decodes and re-encodes one element at a time, and it is lazy because its parts are. A scalar value `To` cannot represent is an encoding error, reported as `whatwg::encode` reports one; a byte sequence `From` does not allow is a decoding error, and the U+FFFD it decodes to is what gets encoded.

[#]{.pnum} *Mandates:* The Encoding Standard defines an encoder for `To`.

:::
