::: wording

## Class template `encode_view` [transcode.codec.encode] {-}

```cpp
constexpr const R& base() const& noexcept;
```

[#]{.pnum} *Returns*: `$base$`.

```cpp
constexpr R base() &&;
```

[#]{.pnum} *Returns*: `move($base$)`.

[#]{.pnum} *Remarks*: `encode_view<Codec, R, E>` validates the `char32_t` elements of `R` as UTF-32, presents the resulting scalar values as the bytes `Codec` encodes them to, and encodes lazily.  When `E` is `error_mode::replacement`, its value type is `char`.  When `E` is `error_mode::report`, its value type is `expected<char, whatwg::error>` and an error yields an element holding the error.  Ill-formed UTF-32 is replaced with U+FFFD before `Codec::encode_one` is called in replacement mode and is reported without calling the codec in report mode.  A custom codec therefore receives only Unicode scalar values.  When `Codec::encode_one` fails and supplies bytes, the view in replacement mode yields those bytes as the replacement; if it supplies none, the replacement is `'?'`.  In report mode the bytes are ignored.

[#]{.pnum} The view models `random_access_range` when `Codec` models `single_byte_text_encoder` and `R` models `random_access_range`. When it does, a scalar value for which `Codec::encode_byte` returns `nullopt` is reported as `whatwg::error::unmapped_codepoint` in report mode and yields `'?'` in replacement mode.

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
template<text_encoder Codec> constexpr $see below$ encode(Codec codec = {});
```

[#]{.pnum} *Returns*: A range adaptor object over `codec`.  Given a subexpression `E` that models `unicode_scalar_range`, `encode(codec)(E)` and `E | encode(codec)` are each expression-equivalent to an `encode_view<Codec, views::all_t<decltype((E))>>` over `E` and a copy of `codec`.

```cpp
template<text_encoder Codec> constexpr $see below$ encode_or_error(Codec codec = {});
```

[#]{.pnum} *Returns*: `encode(codec)` with `error_mode::report`: the view it adapts to has value type `expected<char, whatwg::error>`, and an encoding or UTF-32 validation error is reported rather than replaced.

### Class `encode_view::iterator` [transcode.codec.encode.iterator] {-}

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

[#]{.pnum} *Effects*: Advances to the next byte of the encoded element, encoding the next element of the base range when the current one is exhausted; if no element remains, makes `*this` equal to `end()`.  When `Codec` models `flushable_text_encoder`, exhausting the base range calls `flush()`, and calls it again after each engaged result, yielding the bytes it returns until it returns `nullopt`.  An engaged result with `count == 0` whose `is_error` is false owes nothing and ends the view.  An error result's bytes are used as an `encode_one` error's are: in replacement mode a non-empty `bytes[0, count)` is yielded as the replacement and otherwise `'?'` is; in report mode one `unexpected` element is yielded either way.

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
