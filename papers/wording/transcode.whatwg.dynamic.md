::: wording

## Codecs for an encoding chosen at run time [transcode.whatwg.dynamic] {-}

[#]{.pnum} *Remarks*: `whatwg::dynamic_decoder` is a decoder, as a codec object, for an encoding chosen at run time.  Each operation behaves as the same operation of `whatwg::decoder<E>` does, where `E` is the encoding it was constructed with.  It models `text_decoder`, `flushable_text_decoder` and `bom_aware_text_decoder` ([transcode.codec.reqs]) whatever the encoding, so the decode adaptor over it ([transcode.codec.decode]) yields what `whatwg::decode<E>` does.  It does not model `single_byte_text_decoder`.  It is equality comparable; two objects are equal if they decode the same encoding and are in the same state.

```cpp
constexpr dynamic_decoder() noexcept;
```

[#]{.pnum} *Effects*: Constructs a decoder for `encoding::utf_8`.

```cpp
constexpr explicit dynamic_decoder(whatwg::encoding e) noexcept;
```

[#]{.pnum} *Effects*: Constructs a decoder for `e`, in its initial state.

```cpp
constexpr whatwg::encoding encoding() const noexcept;
```

[#]{.pnum} *Returns*: The encoding this decoder decodes.

```cpp
template<input_iterator I, sentinel_for<I> S>
constexpr decode_result decode_one(I& current, S end);
```

[#]{.pnum} *Effects*: Equivalent to `d.decode_one(current, end)`, where `d` is the `whatwg::decoder<E>` this object behaves as.

```cpp
template<input_iterator I, sentinel_for<I> S>
constexpr decode_result decode_first(I& current, S end);
```

[#]{.pnum} *Effects*: Equivalent to `d.decode_first(current, end)` if the `whatwg::decoder<E>` this object behaves as has `decode_first`, and to `d.decode_one(current, end)` otherwise.

```cpp
constexpr optional<decode_result> flush();
```

[#]{.pnum} *Effects*: Equivalent to `d.flush()` if the `whatwg::decoder<E>` this object behaves as has `flush`.

[#]{.pnum} *Returns*: The result of `d.flush()` if there is one, and `nullopt` otherwise.

[#]{.pnum} *Remarks*: `whatwg::dynamic_encoder` is an encoder, as a codec object, for an encoding chosen at run time.  Each operation behaves as the same operation of `whatwg::encoder<E>` does, where `E` is the encoding it was constructed with.  It models `text_encoder` and `flushable_text_encoder` ([transcode.codec.reqs]) whatever the encoding, so the encode adaptor over it ([transcode.codec.encode]) yields what `whatwg::encode<E>` does.  It does not model `single_byte_text_encoder`.  It is equality comparable.

```cpp
constexpr dynamic_encoder() noexcept;
```

[#]{.pnum} *Effects*: Constructs an encoder for `encoding::utf_8`.

```cpp
constexpr explicit dynamic_encoder(whatwg::encoding e) noexcept;
```

[#]{.pnum} *Preconditions*: `whatwg::has_encoder(e)` is `true` ([transcode.string]).

[#]{.pnum} *Effects*: Constructs an encoder for `e`, in its initial state.

```cpp
constexpr whatwg::encoding encoding() const noexcept;
```

[#]{.pnum} *Returns*: The encoding this encoder encodes.

```cpp
constexpr encode_result encode_one(char32_t scalar);
```

[#]{.pnum} *Effects*: Equivalent to `e.encode_one(scalar)`, where `e` is the `whatwg::encoder<E>` this object behaves as.

```cpp
constexpr optional<encode_result> flush();
```

[#]{.pnum} *Effects*: Equivalent to `e.flush()` if the `whatwg::encoder<E>` this object behaves as has `flush`.

[#]{.pnum} *Returns*: The result of `e.flush()` if there is one, and `nullopt` otherwise.

:::
