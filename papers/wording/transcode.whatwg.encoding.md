::: wording

## Encodings [transcode.whatwg.encoding] {-}

```cpp
enum class encoding {
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

[#]{.pnum} *Remarks*: Each enumerator denotes the encoding that the WHATWG Encoding Standard names correspondingly.

### Label lookup [transcode.whatwg.label] {-}

```cpp
constexpr optional<encoding> get_encoding(string_view label) noexcept;
```

[#]{.pnum} *Returns*: The `encoding` whose WHATWG label set contains `label`, and `nullopt` if no encoding has that label.  Leading and trailing ASCII whitespace is ignored and the comparison is ASCII case-insensitive, which is the Encoding Standard's "get an encoding" algorithm.

[#]{.pnum} *Remarks*: An encoding's labels are the alternative names a document may spell it with -- `latin1`, `iso-8859-1` and `windows-1252` are all labels of `whatwg::encoding::windows_1252` -- so a lookup that fails is a name no encoding claims, not an encoding that cannot be represented.

```cpp
constexpr optional<encoding> get_encoding(text_encoding::id id) noexcept;
```

[#]{.pnum} *Returns*: `get_encoding(label)` ([transcode.whatwg.label]) for the first of the name and the aliases of `std::text_encoding(id)` that is a label of an encoding, and `nullopt` if there is none, which includes `id::other` and `id::unknown`.

[#]{.pnum} *Remarks*: This is the Encoding Standard's "get an encoding" applied to the registered names, so `std::text_encoding::id::ISOLatin1` is `encoding::windows_1252`.

```cpp
constexpr optional<text_encoding::id> to_text_encoding_id(encoding e) noexcept;
```

[#]{.pnum} *Returns*: The `id` of the registered encoding whose name is the name of `e` in the Encoding Standard, and `nullopt` if that name has no registry entry, which includes `encoding::replacement`, `encoding::x_user_defined` and `encoding::x_mac_cyrillic`.

[#]{.pnum} *Remarks*: IANA registers the Encoding Standard's names, so `encoding::windows_1252` is `std::text_encoding::id::windows1252` and not one of its aliases.

### Byte order mark sniffing [transcode.whatwg.sniff] {-}

```cpp
template<legacy_byte_range R> constexpr optional<encoding> bom_sniff(R&& r) noexcept;
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

[#]{.pnum} *Returns*: The encoding `r` begins with a byte order mark for:

- [#.#]{.pnum} `whatwg::encoding::utf_8`, if `r` begins with `EF BB BF`;
- [#.#]{.pnum} `whatwg::encoding::utf_16be`, if `r` begins with `FE FF`;
- [#.#]{.pnum} `whatwg::encoding::utf_16le`, if `r` begins with `FF FE`;
- [#.#]{.pnum} `nullopt` otherwise.

[#]{.pnum} *Remarks*: The mark is not consumed: `r` can be passed on unchanged.

:::
