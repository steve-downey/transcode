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

[#]{.pnum} *Remarks*: An operation that fails to decode or to encode reports one of these values.  The enumerators have the meanings in the following table.

| Constant | Meaning |
|---|---|
| `invalid_byte` | the input holds a byte the encoding does not allow in that position. |
| `truncated_sequence` | the input ends in the middle of a sequence. |
| `overlong_encoding` | the sequence encodes a value that a shorter sequence also encodes. |
| `surrogate_code_point` | a byte sequence decodes to, or a UTF-32 input code unit holds, a surrogate code point, which is not a Unicode scalar value. |
| `out_of_range` | a byte sequence decodes to, or a UTF-32 input code unit holds, a value greater than the largest Unicode scalar value. |
| `unmapped_codepoint` | the encoding has no representation for the Unicode scalar value being encoded. |
: [Enum class `whatwg_error`]{#transcode.errors.whatwg}

```cpp
enum class iconv_error {
  invalid_sequence,
  incomplete_sequence,
  output_full,
  open_failed,
  system_error,
};
```

[#]{.pnum} *Remarks*: An `iconv` conversion that fails reports one of these values, which include the three failures POSIX `iconv` distinguishes and failures at the boundary of that model.  The enumerators have the meanings in the following table.

| Constant | Meaning |
|---|---|
| `invalid_sequence` | the input is not valid in the source encoding, or has no representation in the destination encoding (`EILSEQ`). |
| `incomplete_sequence` | the input ends in the middle of a multibyte sequence (`EINVAL`). |
| `output_full` | the conversion has no room left to write its result (`E2BIG`). |
| `open_failed` | the conversion descriptor could not be opened; `iconv_open` uses `EINVAL` for an unsupported conversion pair, so this condition is kept distinct from an incomplete input sequence. |
| `system_error` | the conversion failed with an error POSIX does not specify for `iconv`. |
: [Enum class `iconv_error`]{#transcode.errors.iconv}

```cpp
enum class transcode_error_kind {
  replacement,
  expected,
};
```

[#]{.pnum} *Remarks*: A view's error kind says how it reports a failure of the codec it drives.  The enumerators have the meanings in the following table.

| Constant | Meaning |
|---|---|
| `replacement` | a failure to decode yields U+FFFD REPLACEMENT CHARACTER.  Ill-formed UTF-32 input is first replaced with U+FFFD and then encoded normally; when an encoder cannot represent a scalar value, the encode view yields `'?'`. The view's value type is the codec's own. |
| `expected` | the view's value type is `expected<T, whatwg_error>`, and a failure yields an `unexpected` holding the error that occurred. |
: [Enum class `transcode_error_kind`]{#transcode.errors.kind}

:::
