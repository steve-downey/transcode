::: wording

## Range requirements [transcode.reqs] {-}

```cpp
template<typename R>
concept legacy_byte_range = ranges::range<R> && !is_array_v<remove_cvref_t<R>> &&
                            $legacy-byte-type$<remove_cv_t<ranges::range_value_t<R>>>;
```

[#]{.pnum} *Remarks*: An array type does not satisfy `legacy_byte_range`, so that the terminating null character of a string literal or of a character array is not transcoded along with the rest of the array.  `views::null_term` adapts a null-terminated character sequence, and `span` a counted buffer.

```cpp
template<typename R>
concept unicode_scalar_range =
    ranges::input_range<R> && !is_array_v<remove_cvref_t<R>> &&
    same_as<remove_cv_t<ranges::range_value_t<R>>, char32_t>;
```

[#]{.pnum} *Remarks*: `unicode_scalar_range` constrains the type of a range and not its values.  That each element of the range is a Unicode scalar value is a precondition of every operation that encodes it, not a property this concept can require.

:::
