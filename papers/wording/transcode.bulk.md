::: wording

## Eager bulk conversion [transcode.bulk] {-}

```cpp
template<encoding C, legacy_byte_range R>
constexpr vector<char32_t> decode_to(R&& source);
```

[#]{.pnum} *Returns*: `forward<R>(source) | decode<C> | ranges::to<vector<char32_t>>()`.

[#]{.pnum} *Remarks*: Each of these is the pipeline it names, run to completion.  They add no behaviour to it: the errors are the decoding view's, substituted rather than reported ([transcode.whatwg.decode]), and a caller who wants them reported writes the pipeline with `whatwg::decode_or_error` instead.

```cpp
template<encoding C, typename Container = string, unicode_scalar_range R>
constexpr Container encode_to(R&& source);
```

[#]{.pnum} *Returns*: `forward<R>(source) | encode<C> | ranges::to<Container>()`.

[#]{.pnum} *Remarks*: `Container` is the caller's choice of result, defaulted to `string`.

```cpp
template<encoding C, legacy_byte_range R, output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Output output);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
ranges::copy(forward<R>(source) | decode<C>, output);
```

```cpp
template<encoding C, unicode_scalar_range R, output_iterator<char> Output>
constexpr void encode_into(R&& source, Output output);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
ranges::copy(forward<R>(source) | encode<C>, output);
```

```cpp
template<legacy_byte_range R>
constexpr vector<char32_t> decode_to(R&& source, encoding e);
```

[#]{.pnum} *Returns*: `whatwg::decode_to<E>(std::forward<R>(source))`, where `E` is the enumerator equal to `e`.

[#]{.pnum} *Remarks*: The encoding is examined once, not once per element.

```cpp
template<legacy_byte_range R>
constexpr optional<vector<char32_t>> decode_to(R&& source, string_view label);
```

[#]{.pnum} *Returns*: `nullopt` if `whatwg::get_encoding(label)` ([transcode.whatwg.label]) is `nullopt`, and otherwise `whatwg::decode_to(std::forward<R>(source), *whatwg::get_encoding(label))`.

```cpp
template<typename Container = string, unicode_scalar_range R>
constexpr optional<Container> encode_to(R&& source, encoding e);
```

[#]{.pnum} *Returns*: `nullopt` if the Encoding Standard defines no encoder for `e`, and otherwise `whatwg::encode_to<E, Container>(std::forward<R>(source))`, where `E` is the enumerator equal to `e`.

[#]{.pnum} *Remarks*: The encoding is examined once, not once per element.

```cpp
template<typename Container = string, unicode_scalar_range R>
constexpr optional<Container> encode_to(R&& source, string_view label);
```

[#]{.pnum} *Returns*: `nullopt` if `whatwg::get_encoding(label)` ([transcode.whatwg.label]) is `nullopt`, and otherwise `whatwg::encode_to<Container>(std::forward<R>(source), *whatwg::get_encoding(label))`.

```cpp
template<legacy_byte_range R, output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, encoding e, Output output);
```

[#]{.pnum} *Effects*: Equivalent to `whatwg::decode_into<E>(std::forward<R>(source), output)`, where `E` is the enumerator equal to `e`.

```cpp
template<unicode_scalar_range R, output_iterator<char> Output>
constexpr bool encode_into(R&& source, encoding e, Output output);
```

[#]{.pnum} *Effects*: If the Encoding Standard defines an encoder for `e`, equivalent to `whatwg::encode_into<E>(std::forward<R>(source), output)`, where `E` is the enumerator equal to `e`; otherwise none.

[#]{.pnum} *Returns*: Whether the Encoding Standard defines an encoder for `e`.

```cpp
template<legacy_byte_range R, text_decoder Codec>
constexpr vector<char32_t> decode_to(R&& source, Codec codec);
```

[#]{.pnum} *Returns*: `forward<R>(source) | decode(codec) | ranges::to<vector<char32_t>>()`.

[#]{.pnum} *Remarks*: The same four operations as the `whatwg::` forms over a codec supplied as a value ([transcode.codec.reqs]), for a codec chosen at run time or one the `whatwg::encoding` enumeration does not name.  As in every eager helper, the source comes first, then what to read or write it as, then any output.

```cpp
template<typename Container = string, unicode_scalar_range R, text_encoder Codec>
constexpr Container encode_to(R&& source, Codec codec);
```

[#]{.pnum} *Returns*: `forward<R>(source) | encode(codec) | ranges::to<Container>()`.

```cpp
template<legacy_byte_range R, text_decoder Codec, output_iterator<char32_t> Output>
constexpr void decode_into(R&& source, Codec codec, Output output);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
ranges::copy(forward<R>(source) | decode(codec), output);
```

```cpp
template<unicode_scalar_range R, text_encoder Codec, output_iterator<char> Output>
constexpr void encode_into(R&& source, Codec codec, Output output);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
ranges::copy(forward<R>(source) | encode(codec), output);
```

:::
