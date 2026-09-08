::: wording

## Transcoding pipelines [transcode.pipeline] {-}

```cpp
template<codec From, codec To> inline constexpr $unspecified$ transcode;
```

[#]{.pnum} *Remarks*: The name `transcode<From, To>` denotes a range adaptor object. Given a subexpression `E` that models `legacy_byte_range`, `transcode<From, To>(E)` and `E | transcode<From, To>` are each expression-equivalent to `E | whatwg_decode<From> | whatwg_encode<To>`. An `E` of array type is ill-formed.

[#]{.pnum} The composition is the specification: the pipeline decodes and re-encodes one element at a time, and it is lazy because its parts are. A scalar value `To` cannot represent is an encoding error, reported as `whatwg_encode` reports one; a byte sequence `From` does not allow is a decoding error, and the U+FFFD it decodes to is what gets encoded.

```cpp
template<decode_codec From, encode_codec To>
constexpr $see below$ pluggable_transcode(From from, To to);
```

[#]{.pnum} *Returns*: A range adaptor object over `from` and `to`.  Given a subexpression `E` that models `legacy_byte_range`, `pluggable_transcode(from, to)(E)` and `E | pluggable_transcode(from, to)` are each expression-equivalent to `E | decode(from) | encode(to)`.

[#]{.pnum} *Remarks*: This is `transcode` ([transcode.pipeline]) with codecs supplied as values rather than named by enumerators.

:::
