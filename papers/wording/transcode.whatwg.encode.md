::: wording

## Encoding views [transcode.whatwg.encode] {-}

```cpp
constexpr const R& base() const& noexcept;
```

[#]{.pnum} *Returns*: `$base$`.

```cpp
constexpr R base() &&;
```

[#]{.pnum} *Returns*: `move($base$)`.

```cpp
constexpr explicit encode_view(R base);
```

[#]{.pnum} *Effects*: Initializes `$base$` with `std::move(base)`.

```cpp
constexpr $iterator$ begin();
```

[#]{.pnum} *Returns*: An `$iterator$` over `$base$`, positioned at the first byte of its first encoded element.

```cpp
constexpr $iterator$ begin() const;
```

[#]{.pnum} *Constraints*: `R` models `$const-iterator-compatible-range$` and `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` over `$base$`, positioned at the first byte of its first encoded element.

```cpp
constexpr $iterator$ end();
```

[#]{.pnum} *Constraints*: `R` models `forward_range` and `R` models `common_range`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has encoded every element of `$base$`.

```cpp
constexpr $iterator$ end() const;
```

[#]{.pnum} *Constraints*:

- [#.#]{.pnum} `const R` models `forward_range`,
- [#.#]{.pnum} `const R` models `common_range`,
- [#.#]{.pnum} `R` models `$const-iterator-compatible-range$`,
- [#.#]{.pnum} `R` models `$const-sentinel-compatible-range$`.

[#]{.pnum} *Returns*: An `$iterator$` that compares equal to an iterator that has encoded every element of `$base$`.

```cpp
constexpr default_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `default_sentinel`.

```cpp
template<whatwg::encoding C> inline constexpr $unspecified$ encode;
```

[#]{.pnum} *Remarks*: The name `whatwg::encode<C>` denotes a range adaptor object.  Given a subexpression `E` that models `unicode_scalar_range`, the expression `whatwg::encode<C>(E)` and `E | whatwg::encode<C>` are each expression-equivalent to a `whatwg::encode_view<C, views::all_t<decltype((E))>>` over `E`.  An `E` of array type is ill-formed.

```cpp
template<whatwg::encoding C> inline constexpr $unspecified$ encode_or_error;
```

[#]{.pnum} *Remarks*: `whatwg::encode_or_error<C>` is `whatwg::encode<C>` with `error_mode::report`: the view it adapts to has value type `expected<char, whatwg::error>`, and validation and encoding errors are reported rather than replaced.

### Class `whatwg::encode_view::iterator` [transcode.whatwg.encode.iterator] {-}

```cpp
constexpr iterator(base_iter current, base_sent end);
```

[#]{.pnum} *Effects*: Initializes `$current$` with `std::move(current)` and `$end$` with `std::move(end)`, then encodes the first element.

```cpp
constexpr auto operator*() const;
```

[#]{.pnum} *Returns*: The byte at the current position of the encoded element, or the error the encoding of that element produced.

```cpp
constexpr $iterator$& operator++();
```

[#]{.pnum} *Effects*: Advances to the next byte of the encoded element, encoding the next element of the base range when the current one is exhausted; if no element remains, makes `*this` equal to `end()`.  When `C` is a stateful encoding, exhausting the base range also emits whatever the encoding owes to return to its initial state.

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
