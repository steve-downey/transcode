::: wording

## Decoding views [transcode.whatwg.decode] {-}

```cpp
template<whatwg::encoding C> inline constexpr $unspecified$ decode;
```

[#]{.pnum} *Remarks*: `whatwg::decode<C>` is `decode(whatwg::decoder<C>{})` ([transcode.custom.decode]).  It is the codec-object adaptor over the library's decoder for the encoding `C`, the decoder as the Encoding Standard defines it.  The value type, the reporting of decoding errors, the laziness and the range properties of the view it adapts to are those of that adaptor.  An argument of array type is ill-formed: a string literal carries a terminating null character that a decode would render, and `views::null_term` ([null.term.adaptor]) is how a null-terminated sequence becomes a range.

```cpp
template<whatwg::encoding C> inline constexpr $unspecified$ decode_or_error;
```

[#]{.pnum} *Remarks*: `whatwg::decode_or_error<C>` is `decode_or_error(whatwg::decoder<C>{})` ([transcode.custom.decode]): it is `whatwg::decode<C>` with `error_mode::report`.  The view it adapts to has value type `expected<char32_t, whatwg::error>`, and a decoding error is the error rather than U+FFFD.

:::
