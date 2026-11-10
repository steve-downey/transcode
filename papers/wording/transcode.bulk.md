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
template<encoding C, legacy_byte_range R>
constexpr expected<vector<char32_t>, error> decode_to_or_error(R&& source);
```

[#]{.pnum} *Returns*: `decode_to_or_error(std::forward<R>(source), whatwg::decoder<C>{})` ([transcode.bulk]).

[#]{.pnum} *Remarks*: `whatwg::decode_to<C>(source)` with the errors reported rather than substituted: the whole text as scalar values, or the first error the Encoding Standard's decoder for `C` reports in it.  For an encoding chosen at run time, `decode_to_or_error(source, whatwg::dynamic_decoder(e))` ([transcode.whatwg.dynamic]).

```cpp
template<encoding C, typename Container = string, unicode_scalar_range R>
constexpr expected<Container, error> encode_to_or_error(R&& source);
```

[#]{.pnum} *Returns*: `encode_to_or_error<Container>(std::forward<R>(source), whatwg::encoder<C>{})` ([transcode.bulk]).

[#]{.pnum} *Remarks*: `whatwg::encode_to<C, Container>(source)` with the errors reported rather than substituted: the whole text as bytes, or the first scalar value that is not a Unicode scalar value or that `C` has no representation for.

[#]{.pnum} *Mandates:* The Encoding Standard defines an encoder for `C`.

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

```cpp
template<legacy_byte_range R, text_decoder Codec>
constexpr expected<vector<char32_t>, whatwg::error> decode_to_or_error(R&& source,
                                                                       Codec codec);
```

[#]{.pnum} *Returns*: Let `v` be `std::forward<R>(source) | decode_or_error(codec)` ([transcode.codec.decode]).  If an element of `v` holds an error, the error of the first such element; otherwise a `vector<char32_t>` holding the value of each element of `v`, in order.

[#]{.pnum} *Remarks*: This is `decode_to(source, codec)` with the errors reported rather than substituted: the whole text, or the first thing wrong with it.  The elements of `v` after the first error are not examined.  A codec for an encoding chosen at run time is `whatwg::dynamic_decoder` ([transcode.whatwg.dynamic]).

```cpp
template<typename Container = string, unicode_scalar_range R, text_encoder Codec>
constexpr expected<Container, whatwg::error> encode_to_or_error(R&& source,
                                                                Codec codec);
```

[#]{.pnum} *Returns*: Let `v` be `std::forward<R>(source) | encode_or_error(codec)` ([transcode.codec.encode]).  If an element of `v` holds an error, the error of the first such element; otherwise a `Container` holding the value of each element of `v`, in order.

[#]{.pnum} *Remarks*: This is `encode_to<Container>(source, codec)` with the errors reported rather than substituted.  The elements of `v` after the first error are not examined, and the encoder is not flushed past it.

:::
