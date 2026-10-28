::: wording

## Eager bulk conversion [transcode.bulk] {-}

```cpp
template<encoding C, legacy_byte_range R>
constexpr vector<char32_t> decode_to(R&& source);
```

[#]{.pnum} *Returns*: `forward<R>(source) | decode<C> | ranges::to<vector<char32_t>>()`.

[#]{.pnum} *Remarks*: Each of these is the pipeline it names, run to completion.  They add no behaviour to it: the errors are the decoding view's, substituted rather than reported ([transcode.whatwg.decode]), and a caller who wants them reported writes the pipeline with `whatwg::decode_or_error` instead. What they add is the name, for the case a caller wants the whole result rather than a view over it.

```cpp
template<encoding C, typename Container = string, unicode_scalar_range R>
constexpr Container encode_to(R&& source);
```

[#]{.pnum} *Returns*: `forward<R>(source) | encode<C> | ranges::to<Container>()`.

[#]{.pnum} *Remarks*: `Container` is the caller's choice of result, defaulted to `string` because that is what encoded bytes are usually wanted in.

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
template<text_decoder Codec, legacy_byte_range R>
constexpr vector<char32_t> decode_to(Codec codec, R&& source);
```

[#]{.pnum} *Returns*: `forward<R>(source) | decode(codec) | ranges::to<vector<char32_t>>()`.

[#]{.pnum} *Remarks*: The same four operations as the `whatwg::` forms over a codec supplied as a value ([transcode.custom.reqs]), for a codec chosen at run time or one the `whatwg::encoding` enumeration does not name.

```cpp
template<text_encoder Codec, typename Container = string, unicode_scalar_range R>
constexpr Container encode_to(Codec codec, R&& source);
```

[#]{.pnum} *Returns*: `forward<R>(source) | encode(codec) | ranges::to<Container>()`.

```cpp
template<text_decoder Codec, legacy_byte_range R, output_iterator<char32_t> Output>
constexpr void decode_into(Codec codec, R&& source, Output output);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
ranges::copy(forward<R>(source) | decode(codec), output);
```

```cpp
template<text_encoder Codec, unicode_scalar_range R, output_iterator<char> Output>
constexpr void encode_into(Codec codec, R&& source, Output output);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
ranges::copy(forward<R>(source) | encode(codec), output);
```

:::
