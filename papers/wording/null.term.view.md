::: wording

## Class template `null_term_view` [null.term.view]{- .sref} {-}

```cpp
constexpr explicit null_term_view(I ptr);
```

[#]{.pnum} *Effects*: Initializes `$ptr$` with `ptr`.

```cpp
constexpr I begin() const;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return $ptr$;
```

```cpp
constexpr null_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `null_sentinel_t()`.

:::
