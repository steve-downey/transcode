::: wording

## iconv adaptors [transcode.iconv]{- .sref} {-}

```cpp
const R& base() const& noexcept;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return $base$;
```

```cpp
R base() &&;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return move($base$);
```

```cpp
const R& base() const& noexcept;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return $base$;
```

```cpp
R base() &&;
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return move($base$);
```

```cpp
explicit iconv_transcode_view(R base, IconvFns fns, const char* from, const char* to,
                              span<char> buf);
```

[#]{.pnum} *Effects*: Initializes the view with `std::move(base)`, `std::move(fns)`, `from`, `to` and `buf`.  No conversion descriptor is opened: `begin` opens one.

```cpp
$iterator$ begin();
```

[#]{.pnum} *Returns*: An `$iterator$` over `$base$` holding a conversion descriptor opened by `$fns$.open($to$, $from$)`.

[#]{.pnum} *Remarks*: Each call opens a descriptor, so a view converts once per `begin`. The iterator owns the descriptor and closes it, which is what makes the adaptor leak-free where the POSIX interface is not.

```cpp
default_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `default_sentinel`.

```cpp
explicit iconv_transcode_or_error_view(R base, IconvFns fns, const char* from,
                                       const char* to, span<char> buf);
```

[#]{.pnum} *Effects*: Initializes the view with `std::move(base)`, `std::move(fns)`, `from`, `to` and `buf`.

```cpp
$iterator$ begin();
```

[#]{.pnum} *Returns*: An `$iterator$` over `$base$` holding a conversion descriptor opened by `$fns$.open($to$, $from$)`.

```cpp
default_sentinel_t end() const;
```

[#]{.pnum} *Returns*: `default_sentinel`.

```cpp
inline iconv_functions make_real_iconv_fns() noexcept;
```

[#]{.pnum} *Returns*: An `iconv_functions` whose members are the implementation's `iconv_open`, `iconv` and `iconv_close`.

```cpp
inline $see below$ iconv_transcode(const char* from, const char* to, span<char> buf);
```

[#]{.pnum} *Returns*: A range adaptor object.  Given a subexpression `E` that models `legacy_byte_range`, `iconv_transcode(from, to, buf)(E)` and `E | iconv_transcode(from, to, buf)` are each expression-equivalent to an `iconv_transcode_view` ([transcode.iconv]{- .sref}) over `E`, `make_real_iconv_fns()`, `from`, `to` and `buf`.

[#]{.pnum} *Remarks*: `buf` is the caller's, and every iterator the adaptor produces converts into it.  Two ranges adapted with the same buffer must not be iterated at the same time.

```cpp
inline $see below$ iconv_transcode_or_error(const char* from, const char* to,
                                         span<char> buf);
```

[#]{.pnum} *Returns*: `iconv_transcode(from, to, buf)` with the errors reported rather than skipped: the view it adapts to has value type `expected<char, iconv_error>`.

```cpp
template<typename Container = string, typename IconvFns, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to,
                             IconvFns fns);
```

[#]{.pnum} *Returns*: A `Container` holding the bytes of `source` converted from `from` to `to` by `fns`.  Input the conversion does not accept is skipped, as it is by `iconv_transcode_view` ([transcode.iconv]{- .sref}).  The result is empty when the conversion descriptor cannot be opened -- which is what `iconv_open` failing means, and is not distinguishable here from an empty input.

```cpp
template<typename Container = string, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to);
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return iconv_transcode_to<Container>(forward<R>(source), from, to,
                                     make_real_iconv_fns());
```

```cpp
template<typename IconvFns, legacy_byte_range R, output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to, Output output,
                            IconvFns fns);
```

[#]{.pnum} *Effects*: Converts the bytes of `source` from `from` to `to` by `fns` and writes them through `output`.

[#]{.pnum} *Returns*: The value of `output` after the last byte written.

```cpp
template<legacy_byte_range R, output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to,
                            Output output);
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return iconv_transcode_into(forward<R>(source), from, to, move(output),
                            make_real_iconv_fns());
```

```cpp
template<typename Container = string, typename IconvFns, legacy_byte_range R>
expected<Container, iconv_error> iconv_transcode_to_or_error(R&& source,
                                                             const char* from,
                                                             const char* to,
                                                             IconvFns fns);
```

[#]{.pnum} *Returns*: A `Container` holding the converted bytes, or the first `iconv_error` the conversion reported.

[#]{.pnum} *Remarks*: This is the eager form of `iconv_transcode_or_error` ([transcode.iconv]{- .sref}): it stops at the first failure rather than skipping it, which is the difference between a conversion a program wants to know about and one it wants to get through.

```cpp
template<typename Container = string, legacy_byte_range R>
expected<Container, iconv_error> iconv_transcode_to_or_error(R&& source,
                                                             const char* from,
                                                             const char* to);
```

[#]{.pnum} *Returns*: Equivalent to:

```cpp
return iconv_transcode_to_or_error<Container>(forward<R>(source), from, to,
                                              make_real_iconv_fns());
```

### Classes `iconv_transcode_view::iterator` and `iconv_transcode_or_error_view::iterator` [transcode.iconv.iterator]{- .sref} {-}

```cpp
iterator($iterator$&&) noexcept;
```

[#]{.pnum} *Effects*: Takes over `other`'s conversion descriptor and leaves `other` holding none.

[#]{.pnum} *Remarks*: The iterator is move-only.  A conversion descriptor is an OS-owned handle that cannot be duplicated, so copying one would either close it twice or leak it.

```cpp
$iterator$& operator=($iterator$&&) noexcept;
```

[#]{.pnum} *Effects*: Closes the descriptor `*this` holds, if any, then takes over `other`'s and leaves `other` holding none.

[#]{.pnum} *Returns*: `*this`.

```cpp
~$iterator$();
```

[#]{.pnum} *Effects*: Closes the conversion descriptor, if `*this` holds one.

```cpp
char operator*() const;
```

[#]{.pnum} *Returns*: The converted byte at the current position of the output buffer.

```cpp
$iterator$& operator++();
```

[#]{.pnum} *Effects*: Advances to the next converted byte, converting more input when the buffer is exhausted; if no input and no unconverted bytes remain, makes `*this` equal to `default_sentinel`.

[#]{.pnum} *Returns*: `*this`.

[#]{.pnum} *Remarks*: Input the conversion does not accept is skipped one byte at a time -- what POSIX reports as `EILSEQ` -- so the range ends where the input does rather than at the first byte a converter refuses.  A program that needs to know *that* it happened uses `iconv_transcode_or_error` ([transcode.iconv]{- .sref}).

```cpp
void operator++(int);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
++*this;
```

```cpp
iterator($iterator$&&) noexcept;
```

[#]{.pnum} *Effects*: Takes over `other`'s conversion descriptor and leaves `other` holding none.

```cpp
$iterator$& operator=($iterator$&&) noexcept;
```

[#]{.pnum} *Effects*: Closes the descriptor `*this` holds, if any, then takes over `other`'s and leaves `other` holding none.

[#]{.pnum} *Returns*: `*this`.

```cpp
~$iterator$();
```

[#]{.pnum} *Effects*: Closes the conversion descriptor, if `*this` holds one.

```cpp
expected<char, iconv_error> operator*() const;
```

[#]{.pnum} *Returns*: The converted byte at the current position of the output buffer, or the `iconv_error` the conversion reported there.

```cpp
$iterator$& operator++();
```

[#]{.pnum} *Effects*: Advances to the next element, converting more input when the buffer is exhausted; if no input and no unconverted bytes remain, makes `*this` equal to `default_sentinel`.

[#]{.pnum} *Returns*: `*this`.

```cpp
void operator++(int);
```

[#]{.pnum} *Effects*: Equivalent to:

```cpp
++*this;
```

:::
