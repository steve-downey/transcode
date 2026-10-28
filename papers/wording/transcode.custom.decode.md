::: wording

## Class template `decode_view` [transcode.custom.decode] {-}

```cpp
constexpr const R& base() const& noexcept;
```

[#]{.pnum} *Returns*: `$base$`.

```cpp
constexpr R base() &&;
```

[#]{.pnum} *Returns*: `move($base$)`.

```cpp
constexpr explicit decode_view(R base, Codec codec = {});
```

[#]{.pnum} *Effects*: Initializes `$base$` with `std::move(base)` and `$codec$` with `codec`.

```cpp
constexpr $iterator$ begin();
```

[#]{.pnum} *Returns*: An `$iterator$` over `$base$` and a copy of `$codec$`, positioned at its first decoded element.

```cpp
constexpr $iterator$ begin() const;
```

[#]{.pnum} *Constraints*: `R` models `$const-iterator-compatible-range$` and `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` over `$base$` and a copy of `$codec$`, positioned at its first decoded element.

```cpp
constexpr $iterator$ end();
```

[#]{.pnum} *Constraints*: `R` models `forward_range` and `R` models `common_range`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has decoded every element of `$base$`.

```cpp
constexpr $iterator$ end() const;
```

[#]{.pnum} *Constraints*:

- [#.#]{.pnum} `const R` models `forward_range`,
- [#.#]{.pnum} `const R` models `common_range`,
- [#.#]{.pnum} `R` models `$const-iterator-compatible-range$`,
- [#.#]{.pnum} `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has decoded every element of `$base$`.

```cpp
constexpr default_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `default_sentinel`.

```cpp
template<text_decoder Codec> constexpr $see below$ decode(Codec codec = {});
```

[#]{.pnum} *Returns*: A range adaptor object over `codec`.  Given a subexpression `E` that models `legacy_byte_range`, `decode(codec)(E)` and `E | decode(codec)` are each expression-equivalent to a `decode_view<Codec, views::all_t<decltype((E))>>` over `E` and a copy of `codec`.  An `E` of array type is ill-formed.

```cpp
template<text_decoder Codec> constexpr $see below$ decode_or_error(Codec codec = {});
```

[#]{.pnum} *Returns*: `decode(codec)` with `error_mode::report`: the view it adapts to has value type `expected<char32_t, whatwg::error>`, and a decoding error is the error rather than U+FFFD.

### Class `decode_view::iterator` [transcode.custom.decode.iterator] {-}

```cpp
constexpr iterator(base_iter current, base_sent end, Codec codec);
```

[#]{.pnum} *Effects*: Initializes `$current$` with `std::move(current)`, `$end$` with `std::move(end)` and the iterator's own copy of the codec with `codec`, then decodes the first element.

```cpp
constexpr result_t operator*() const;
```

[#]{.pnum} *Returns*: `$value$`.

```cpp
constexpr $iterator$& operator++();
```

[#]{.pnum} *Effects*: Decodes the next element of the base range, or, if none remains, makes `*this` equal to `end()`.  A `decode_one` result with `is_empty` yields no element and decoding goes on.  When `Codec` models `flushable_text_decoder`, exhausting the base range calls `flush()`, and calls it again after each engaged result, yielding what it returns until it returns `nullopt` or a result with `is_empty`.

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
