::: wording

## Transcoding pipelines [transcode.pipeline] {-}

```cpp
template<encoding From, encoding To> inline constexpr $unspecified$ transcode;
```

[#]{.pnum} *Remarks*: The name `whatwg::transcode<From, To>` denotes a range adaptor object.  Given a subexpression `E` that models `legacy_byte_range`, `whatwg::transcode<From, To>(E)` and `E | whatwg::transcode<From, To>` are each expression-equivalent to `E | whatwg::decode<From> | whatwg::encode<To>`. An `E` of array type is ill-formed.

[#]{.pnum} The composition is the specification: the pipeline decodes and re-encodes one element at a time, and it is lazy because its parts are. A scalar value `To` cannot represent is an encoding error, reported as `whatwg::encode` reports one; a byte sequence `From` does not allow is a decoding error, and the U+FFFD it decodes to is what gets encoded.

```cpp
template<text_decoder From, text_encoder To>
constexpr $see below$ transcode(From from, To to);
```

[#]{.pnum} *Returns*: A range adaptor object over `from` and `to`.  Given a subexpression `E` that models `legacy_byte_range`, `transcode(from, to)(E)` and `E | transcode(from, to)` are each expression-equivalent to `E | decode(from) | encode(to)`.

[#]{.pnum} *Remarks*: This is `whatwg::transcode` ([transcode.pipeline]) with codecs supplied as values rather than named by enumerators.

:::
