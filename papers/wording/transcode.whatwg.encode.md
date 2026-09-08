::: wording

## Encoding views [transcode.whatwg.encode]{- .sref} {-}

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
constexpr explicit whatwg_encode_view(R base);
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
template<codec C> inline constexpr $unspecified$ whatwg_encode;
```

[#]{.pnum} *Remarks*: The name `whatwg_encode<C>` denotes a range adaptor object.  Given a subexpression `E` that models `unicode_scalar_range`, the expression `whatwg_encode<C>(E)` and `E | whatwg_encode<C>` are each expression-equivalent to a `whatwg_encode_view<C, views::all_t<decltype((E))>>` over `E`.  An `E` of array type is ill-formed.

```cpp
template<codec C> inline constexpr $unspecified$ whatwg_encode_or_error;
```

[#]{.pnum} *Remarks*: `whatwg_encode_or_error<C>` is `whatwg_encode<C>` with `transcode_error_kind::expected`: the view it adapts to has value type `expected<char, whatwg_error>`, and an encoding error is the error rather than `'?'`.

### Class `whatwg_encode_view::iterator` [transcode.whatwg.encode.iterator]{- .sref} {-}

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
