::: wording

## Error types [transcode.errors] {-}

```cpp
enum class whatwg_error {
  invalid_byte,
  truncated_sequence,
  overlong_encoding,
  surrogate_code_point,
  out_of_range,
  unmapped_codepoint,
};
```

[#]{.pnum} *Remarks*: An operation that fails to decode or to encode reports one of these values.  The enumerators have the following meanings:

- [#.#]{.pnum} `invalid_byte` -- the input holds a byte the encoding does not allow in that position.
- [#.#]{.pnum} `truncated_sequence` -- the input ends in the middle of a sequence.
- [#.#]{.pnum} `overlong_encoding` -- the sequence encodes a value that a shorter sequence also encodes.
- [#.#]{.pnum} `surrogate_code_point` -- the sequence encodes a surrogate code point, which is not a Unicode scalar value.
- [#.#]{.pnum} `out_of_range` -- the sequence encodes a value greater than the largest Unicode scalar value.
- [#.#]{.pnum} `unmapped_codepoint` -- the encoding has no representation for the Unicode scalar value being encoded.

```cpp
enum class iconv_error {
  invalid_sequence,
  incomplete_sequence,
  output_full,
};
```

[#]{.pnum} *Remarks*: An `iconv` conversion that fails reports one of these values, which are the three failures POSIX `iconv` distinguishes.  The enumerators have the following meanings:

- [#.#]{.pnum} `invalid_sequence` -- the input is not valid in the source encoding, or has no representation in the destination encoding (`EILSEQ`).
- [#.#]{.pnum} `incomplete_sequence` -- the input ends in the middle of a multibyte sequence (`EINVAL`).
- [#.#]{.pnum} `output_full` -- the conversion has no room left to write its result (`E2BIG`).

```cpp
enum class transcode_error_kind {
  replacement,
  expected,
};
```

[#]{.pnum} *Remarks*: A view's error kind says how it reports a failure of the codec it drives.  The enumerators have the following meanings:

- [#.#]{.pnum} `replacement` -- a failure to decode yields U+FFFD REPLACEMENT CHARACTER and a failure to encode yields `'?'`, and the view's value type is the codec's own.
- [#.#]{.pnum} `expected` -- the view's value type is `expected<T, whatwg_error>`, and a failure yields an `unexpected` holding the error that occurred.

:::
