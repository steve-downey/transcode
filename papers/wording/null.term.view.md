::: wording

## Class template `null_term_view` [null.term.view] {-}

```cpp
constexpr null_term_view() = default;
```

[#]{.pnum} *Remarks*: The resulting view is singular. It may be assigned to and destroyed, but iterating it results in undefined behavior because the iterator returned by `begin()` has no reachable terminator.

```cpp
constexpr explicit null_term_view(I ptr);
```

[#]{.pnum} *Preconditions*: There is an iterator `i` reachable from `ptr` by a finite sequence of applications of `++` such that `*i == 0`, and every element from `ptr` through `i`, inclusive, is within the lifetime of a single object.

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
