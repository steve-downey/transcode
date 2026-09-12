::: wording

## Eager transcoding [transcode.string] {-}

```cpp
constexpr bool has_whatwg_encoder(codec encoding) noexcept;
```

[#]{.pnum} *Returns*: `true` if the WHATWG Encoding Standard defines an encoder for `encoding`, and `false` otherwise.

```cpp
optional<string> transcode_string(span<const char> src, codec from, codec to);
```

[#]{.pnum} *Returns*: `nullopt` if the WHATWG Encoding Standard defines no encoder for `to`; otherwise, the bytes of `src`, decoded as `from` and re-encoded as `to`. Errors are substituted rather than reported: a byte sequence `from` does not allow decodes to U+FFFD, and a scalar value `to` cannot represent encodes to `'?'`.

[#]{.pnum} *Remarks*: This is `transcode` ([transcode.pipeline]) run to completion into a `string`, which is what a caller who wants the whole result and not a view writes.  `src | transcode<From, To> | ranges::to<string>()` is the same thing with the codecs known at compile time.

```cpp
optional<string> transcode_string(span<const char> src, string_view from_label,
                                  string_view to_label);
```

[#]{.pnum} *Returns*: `transcode_string(src, *get_encoding(from_label), *get_encoding(to_label))`, and `nullopt` if either label names no encoding ([transcode.codec.label]) or the target encoding has no WHATWG encoder.

[#]{.pnum} *Remarks*: This is the overload a program uses when the encodings are a runtime choice -- a `Content-Type` header, a command-line option -- which is what labels are for.

:::
