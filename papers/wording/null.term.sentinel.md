::: wording

## Class `null_sentinel_t` [null.term.sentinel] {-}

```cpp
template<input_iterator I>
  requires requires(I i) {
    { *i == 0 };
  }
friend constexpr bool operator==(const I& it, null_sentinel_t);
```

[#]{.pnum} *Returns*: `*it == 0`.

:::
