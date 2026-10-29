::: wording

## Transcoding pipelines [transcode.pipeline] {-}

```cpp
template<encoding From, encoding To> inline constexpr $unspecified$ transcode;
```

[#]{.pnum} *Remarks*: `whatwg::transcode<From, To>` is `transcode(whatwg::decoder<From>{}, whatwg::encoder<To>{})` ([transcode.pipeline]), and so is `E | whatwg::decode<From> | whatwg::encode<To>` for a subexpression `E` that models `legacy_byte_range`.  An `E` of array type is ill-formed.

[#]{.pnum} The composition is the specification: the pipeline decodes and re-encodes one element at a time, and it is lazy because its parts are. A scalar value `To` cannot represent is an encoding error, reported as `whatwg::encode` reports one; a byte sequence `From` does not allow is a decoding error, and the U+FFFD it decodes to is what gets encoded.

[#]{.pnum} *Mandates:* The Encoding Standard defines an encoder for `To`.

:::
