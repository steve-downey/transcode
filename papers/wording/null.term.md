```cpp
struct null_sentinel_t {
  template<input_iterator I>
    requires requires(I i) {
      { *i == 0 };
    }
  friend constexpr bool operator==(const I& it, null_sentinel_t);
};
```

```cpp
template<std::contiguous_iterator I>
class null_term_view : public ranges::view_interface<null_term_view<I>> {
  I $ptr$; // exposition only

public:
  constexpr null_term_view() = default;
  constexpr explicit null_term_view(I ptr);

  constexpr I begin() const;
  constexpr null_sentinel_t end() const;
};
```
