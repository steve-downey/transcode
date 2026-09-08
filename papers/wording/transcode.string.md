::: wording

## Eager transcoding [transcode.string]{- .sref} {-}

```cpp
string transcode_string(span<const char> src, codec from, codec to);
```

[#]{.pnum} *Returns*: The bytes of `src`, decoded as `from` and re-encoded as `to`. Errors are substituted rather than reported: a byte sequence `from` does not allow decodes to U+FFFD, and a scalar value `to` cannot represent encodes to `'?'`.  The result is empty when `to` names an encoding the WHATWG Encoding Standard defines no encoder for.

[#]{.pnum} *Remarks*: This is `transcode` ([transcode.pipeline]{- .sref}) run to completion into a `string`, which is what a caller who wants the whole result and not a view writes.  `src | transcode<From, To> | ranges::to<string>()` is the same thing with the codecs known at compile time.

```cpp
optional<string> transcode_string(span<const char> src, string_view from_label,
                                  string_view to_label);
```

[#]{.pnum} *Returns*: `transcode_string(src, *get_encoding(from_label), *get_encoding(to_label))`, and `nullopt` if either label names no encoding ([transcode.codec.label]{- .sref}).

[#]{.pnum} *Remarks*: This is the overload a program uses when the encodings are a runtime choice -- a `Content-Type` header, a command-line option -- which is what labels are for.

:::
