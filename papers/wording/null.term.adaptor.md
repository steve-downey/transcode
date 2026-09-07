::: wording

## `views::null_term` [null.term.adaptor]{- .sref} {-}

```cpp
inline constexpr $unspecified$ null_term;
```

[#]{.pnum} *Remarks*: The name `views::null_term` denotes a range adaptor object.  Given a subexpression `E`, the expression `views::null_term(E)` is expression-equivalent to `null_term_view(E)` when `E` has pointer type, and to `null_term_view(static_cast<const T*>(E))` when `E` has array type with element type `T`.  For any other type it is ill-formed, so that a range carrying no terminator is a diagnosed error and not a silent one.

:::
