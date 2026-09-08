::: wording

## Decoding views [transcode.whatwg.decode]{- .sref} {-}

```cpp
constexpr const R& base() const& noexcept;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return $base$;
```

```cpp
constexpr R base() &&;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return move($base$);
```

```cpp
constexpr explicit whatwg_decode_view(R base);
```

[#]{.pnum} *Effects*: Initializes `$base$` with `std::move(base)`.

```cpp
constexpr $iterator$ begin();
```

[#]{.pnum} *Returns*: An `$iterator$` over `$base$`, positioned at its first decoded element.  Decoding the first element is part of forming the iterator: a view over an input range cannot say whether it is empty without reading it.

```cpp
constexpr $iterator$ begin() const;
```

[#]{.pnum} *Constraints*: `R` models `$const-iterator-compatible-range$` and `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` over `$base$`, positioned at its first decoded element.

```cpp
constexpr $iterator$ end();
```

[#]{.pnum} *Constraints*: `R` models `forward_range` and `R` models `common_range`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has decoded every element of `$base$`.

```cpp
constexpr $iterator$ end() const;
```

[#]{.pnum} *Constraints*:

- [#.#]{.pnum} `R` models `forward_range`,
- [#.#]{.pnum} `R` models `common_range`,
- [#.#]{.pnum} `R` models `$const-iterator-compatible-range$`,
- [#.#]{.pnum} `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has decoded every element of `$base$`.

```cpp
constexpr default_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `default_sentinel`.

```cpp
template<codec C> inline constexpr $unspecified$ whatwg_decode;
```

[#]{.pnum} *Remarks*: The name `whatwg_decode<C>` denotes a range adaptor object.  Given a subexpression `E` that models `legacy_byte_range`, the expression `whatwg_decode<C>(E)` and `E | whatwg_decode<C>` are each expression-equivalent to a `whatwg_decode_view<C, views::all_t<decltype((E))>>` over `E`.  An `E` of array type is ill-formed: a string literal carries a terminating null character that a decode would render, and `views::null_term` ([null.term.adaptor]{- .sref}) is how a null-terminated sequence becomes a range.

```cpp
template<codec C> inline constexpr $unspecified$ whatwg_decode_or_error;
```

[#]{.pnum} *Remarks*: `whatwg_decode_or_error<C>` is `whatwg_decode<C>` with `transcode_error_kind::expected`: the view it adapts to has value type `expected<char32_t, whatwg_error>`, and a decoding error is the error rather than U+FFFD.

### Class `whatwg_decode_view::iterator` [transcode.whatwg.decode.iterator]{- .sref} {-}

```cpp
constexpr iterator(base_iter current, base_sent end);
```

[#]{.pnum} *Effects*: Initializes `$current$` with `std::move(current)` and `$end$` with `std::move(end)`, then decodes the first element.  When `C` is a UTF encoding and that element is U+FEFF BYTE ORDER MARK, it is discarded and the next element decoded in its place: a byte order mark is how the encoding was chosen, not text the document contains.

```cpp
constexpr result_t operator*() const;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return $value$;
```

```cpp
constexpr $iterator$& operator++();
```

[#]{.pnum} *Effects*: Decodes the next element of the base range, or, if none remains, makes `*this` equal to `end()`.

[#]{.pnum} *Returns*: `*this`.

```cpp
constexpr $iterator$ operator++(int);
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

[#]{.pnum} *Effects*: Equivalent to:

```cpp
auto tmp = *this;
++*this;
return tmp;
```

```cpp
constexpr void operator++(int);
```

[#]{.pnum} *Constraints*: `ranges::forward_range<R>` is `false`.

[#]{.pnum} *Effects*: Equivalent to:

```cpp
++*this;
```

:::
