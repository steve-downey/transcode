::: wording

## Encodings [transcode.codec]{- .sref} {-}

```cpp
enum class codec {
  utf_8,
  replacement,
  x_user_defined,
  ibm866,
  iso_8859_2,
  iso_8859_3,
  iso_8859_4,
  iso_8859_5,
  iso_8859_6,
  iso_8859_7,
  iso_8859_8,
  iso_8859_8_i,
  iso_8859_10,
  iso_8859_13,
  iso_8859_14,
  iso_8859_15,
  iso_8859_16,
  koi8_r,
  koi8_u,
  macintosh,
  windows_874,
  windows_1250,
  windows_1251,
  windows_1252,
  windows_1253,
  windows_1254,
  windows_1255,
  windows_1256,
  windows_1257,
  windows_1258,
  x_mac_cyrillic,
  utf_16be,
  utf_16le,
  gbk,
  gb18030,
  big5,
  shift_jis,
  euc_jp,
  iso_2022_jp,
  euc_kr,
};
```

[#]{.pnum} *Remarks*: Each enumerator names the encoding of the same name in the WHATWG Encoding Standard, whose table of encodings is normative and is not restated here: an encoding is what that standard says it is, and a specification that copied the list would be a second one to keep current. `codec::replacement` is that standard's `replacement` encoding, which decodes any non-empty input to a single U+FFFD REPLACEMENT CHARACTER, and `codec::x_user_defined` its `x-user-defined`.

### Label lookup [transcode.codec.label]{- .sref} {-}

```cpp
constexpr optional<codec> get_encoding(string_view label) noexcept;
```

[#]{.pnum} *Returns*: The `codec` whose WHATWG label set contains `label`, and `nullopt` if no encoding has that label.  Leading and trailing ASCII whitespace is ignored and the comparison is ASCII case-insensitive, which is the Encoding Standard's "get an encoding" algorithm.

[#]{.pnum} *Remarks*: An encoding's labels are the alternative names a document may spell it with -- `latin1`, `iso-8859-1` and `windows-1252` are all labels of `codec::windows_1252` -- so a lookup that fails is a name no encoding claims, not an encoding that cannot be represented.

### Byte order mark sniffing [transcode.codec.sniff]{- .sref} {-}

```cpp
template<legacy_byte_range R> constexpr optional<codec> sniff_encoding(R&& r) noexcept;
```

[#]{.pnum} *Returns*: The encoding `r` begins with a byte order mark for:

- [#.#]{.pnum} `codec::utf_8`, if `r` begins with `EF BB BF`;
- [#.#]{.pnum} `codec::utf_16be`, if `r` begins with `FE FF`;
- [#.#]{.pnum} `codec::utf_16le`, if `r` begins with `FF FE`;
- [#.#]{.pnum} `nullopt` otherwise.

[#]{.pnum} *Remarks*: This is the byte order mark half of the Encoding Standard's "decide the fallback encoding" step: what to do when there is no mark is a question about the document and its transport, which a library cannot answer.  The mark itself is not consumed -- a decode view strips a leading U+FEFF for the UTF codecs on its own -- so the range can be passed on unchanged.

:::
