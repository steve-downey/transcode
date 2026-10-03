::: wording

## Class `null_sentinel_t` [null.term.sentinel] {-}

```cpp
template<input_iterator I>
friend constexpr bool operator==(const I& it, null_sentinel_t);
```

[#]{.pnum} *Constraints*: `requires(I i) { { *i == 0 }; }` is `true`.

[#]{.pnum} *Returns*: `*it == 0`.

:::
