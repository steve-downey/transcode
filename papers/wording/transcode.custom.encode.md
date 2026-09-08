::: wording

## Class template `encode_view` [transcode.custom.encode]{- .sref} {-}

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
constexpr explicit encode_view(R base, Codec codec = {});
```

[#]{.pnum} *Effects*: Initializes `$base$` with `std::move(base)` and `$codec$` with `codec`.

```cpp
constexpr $iterator$ begin();
```

[#]{.pnum} *Returns*: An `$iterator$` over `$base$` and a copy of `$codec$`, positioned at the first byte of its first encoded element.

```cpp
constexpr $iterator$ begin() const;
```

[#]{.pnum} *Constraints*: `R` models `$const-iterator-compatible-range$` and `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` over `$base$` and a copy of `$codec$`, positioned at the first byte of its first encoded element.

```cpp
constexpr $iterator$ end();
```

[#]{.pnum} *Constraints*: `R` models `forward_range` and `R` models `common_range`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has encoded every element of `$base$`.

```cpp
constexpr $iterator$ end() const;
```

[#]{.pnum} *Constraints*:

- [#.#]{.pnum} `R` models `forward_range`,
- [#.#]{.pnum} `R` models `common_range`,
- [#.#]{.pnum} `R` models `$const-iterator-compatible-range$`,
- [#.#]{.pnum} `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has encoded every element of `$base$`.

```cpp
constexpr default_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `default_sentinel`.

```cpp
template<encode_codec Codec> constexpr $see below$ encode(Codec codec = {});
```

[#]{.pnum} *Returns*: A range adaptor object over `codec`.  Given a subexpression `E` that models `unicode_scalar_range`, `encode(codec)(E)` and `E | encode(codec)` are each expression-equivalent to an `encode_view<Codec, views::all_t<decltype((E))>>` over `E` and a copy of `codec`.

```cpp
template<encode_codec Codec> constexpr $see below$ encode_or_error(Codec codec = {});
```

[#]{.pnum} *Returns*: `encode(codec)` with `transcode_error_kind::expected`: the view it adapts to has value type `expected<char, whatwg_error>`, and an encoding error is the error rather than `'?'`.

### Class `encode_view::iterator` [transcode.custom.encode.iterator]{- .sref} {-}

```cpp
constexpr iterator(base_iter current, base_sent end, Codec codec);
```

[#]{.pnum} *Effects*: Initializes `$current$` with `std::move(current)`, `$end$` with `std::move(end)` and the iterator's own copy of the codec with `codec`, then encodes the first element.

```cpp
constexpr result_t operator*() const;
```

[#]{.pnum} *Returns*: The byte at the current position of the encoded element, or the error the encoding of that element produced.

```cpp
constexpr $iterator$& operator++();
```

[#]{.pnum} *Effects*: Advances to the next byte of the encoded element, encoding the next element of the base range when the current one is exhausted; if no element remains, makes `*this` equal to `end()`.

[#]{.pnum} *Returns*: `*this`.

```cpp
constexpr $iterator$ operator++(int);
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

```cpp
constexpr void operator++(int);
```

[#]{.pnum} *Constraints*: `ranges::forward_range<R>` is `false`.

:::
