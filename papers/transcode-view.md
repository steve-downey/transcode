---
title: "Transcoding Text Views"
subtitle: "Range adapters for character encoding conversion"
document: D4246R0
date: today
audience:
  - SG16
  - SG9
  - LEWGI
  - LEWG
author:
  - name: Steve Downey
    email: <sdowney@gmail.com>
    email: <sdowney2@bloomberg.net>
toc: true
toc-depth: 2
---

# Abstract

We propose a set of range adaptor views for transcoding text between character encodings, providing both WHATWG-compliant codecs for web interoperability and an `iconv`-based adaptor for broader encoding support.

Character encoding conversion is a fundamental operation when processing text from external sources — network protocols, file formats, legacy databases, and user input. The current standard library offers no direct support for this operation. Existing solutions require manual buffer management, explicit error handling at every step, and careful attention to encoding-specific edge cases. This proposal provides lazy, composable range views that decode byte sequences to Unicode scalar values and encode Unicode back to bytes, with well-defined error handling semantics.

The design follows the WHATWG Encoding Standard for codec semantics, ensuring compatibility with web platform behavior. For encodings not covered by WHATWG, an optional `iconv`-based adaptor provides access to the platform's native transcoding capabilities.

# Comparison Table

## Decoding UTF-8 to Unicode code points

::: cmptable

### Before (manual loop)
```cpp
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  std::vector<char32_t> result;
  size_t i = 0;
  while (i < input.size()) {
    unsigned char b = input[i];
    char32_t cp; int extra;
    if (b < 0x80) { cp = b; extra = 0; }
    else if ((b & 0xE0) == 0xC0)
      { cp = b & 0x1F; extra = 1; }
    else if ((b & 0xF0) == 0xE0)
      { cp = b & 0x0F; extra = 2; }
    else if ((b & 0xF8) == 0xF0)
      { cp = b & 0x07; extra = 3; }
    else { result.push_back(U'\xFFFD');
           ++i; continue; }
    if (i + extra >= input.size()) {
      result.push_back(U'\xFFFD'); break;
    }
    for (int j = 0; j < extra; ++j) {
      unsigned char c = input[++i];
      if ((c & 0xC0) != 0x80) {
        cp = U'\xFFFD'; break;
      }
      cp = (cp << 6) | (c & 0x3F);
    }
    // Missing: overlong, surrogate checks
    result.push_back(cp);
    ++i;
  }
  return result;
}
```

### After
```cpp
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  return input
    | std::views::transcode<codec::utf_8>
    | std::ranges::to<std::vector>();
}
```

:::

## Transcoding Shift_JIS to UTF-8

::: cmptable

### Before (iconv)
```cpp
std::string shift_jis_to_utf8(
    std::string_view input) {
  iconv_t cd = iconv_open("UTF-8",
                          "SHIFT_JIS");
  if (cd == (iconv_t)-1)
    throw std::runtime_error("iconv_open");

  std::string result;
  result.resize(input.size() * 4);

  char* inbuf = const_cast<char*>(
                  input.data());
  size_t inleft = input.size();
  char* outbuf = result.data();
  size_t outleft = result.size();

  while (inleft > 0) {
    size_t r = iconv(cd, &inbuf, &inleft,
                     &outbuf, &outleft);
    if (r == (size_t)-1) {
      if (errno == E2BIG) {
        size_t used = outbuf - result.data();
        result.resize(result.size() * 2);
        outbuf = result.data() + used;
        outleft = result.size() - used;
      } else if (errno == EILSEQ) {
        // Skip invalid byte
        ++inbuf; --inleft;
        // Append replacement char
        *outbuf++ = '\xEF';
        *outbuf++ = '\xBF';
        *outbuf++ = '\xBD';
        outleft -= 3;
      } else {
        iconv_close(cd);
        throw std::runtime_error("iconv");
      }
    }
  }
  result.resize(outbuf - result.data());
  iconv_close(cd);
  return result;
}
```

### After
```cpp
std::string shift_jis_to_utf8(
    std::string_view input) {
  return input
    | std::views::transcode<codec::shift_jis>
    | std::views::transcode<codec::utf_8>
    | std::ranges::to<std::string>();
}
```

:::

## Converting a null-terminated C string

::: cmptable

### Before (mbstowcs)
```cpp
std::wstring from_cstring(const char* s) {
  // Assumes locale is set correctly
  std::setlocale(LC_ALL, "");
  size_t len = std::mbstowcs(nullptr, s, 0);
  if (len == (size_t)-1)
    throw std::runtime_error("mbstowcs");
  std::wstring result(len, L'\0');
  std::mbstowcs(result.data(), s, len + 1);
  return result;
  // Problems:
  // - Global locale state
  // - wchar_t is not portable
  // - No error recovery
  // - Two passes required
}
```

### After
```cpp
auto from_cstring(const char* s) {
  return std::views::null_term(s)
    | std::views::transcode<codec::utf_8>;
  // Returns lazy view of char32_t
  // No global state
  // Portable Unicode scalars
  // Single pass, errors yield U+FFFD
}
```

:::

## Processing with error visibility

::: cmptable

### Before
```cpp
// No standard way to detect errors
// during transcoding. Either:
// 1. Errors are silently replaced
// 2. Exceptions thrown mid-stream
// 3. Custom state machine required

bool has_errors = false;
std::vector<char32_t> result;
// ... complex manual decoding with
// error tracking interspersed ...
if (has_errors) {
  log_warning("Invalid UTF-8 detected");
}
```

### After
```cpp
for (auto r : input
    | std::views::transcode_or_error<
        codec::utf_8>) {
  if (r.has_value()) {
    process(*r);
  } else {
    log_warning(r.error());
    process(U'\xFFFD');
  }
}
```

:::

# Motivation

## The Status Quo

C++ has no standard facility for character encoding conversion. Programs must choose between:

- **`mbstowcs`/`wcstombs`**: Depends on global locale state, uses non-portable `wchar_t`, provides no error recovery, and supports only the locale's encoding.

- **`iconv`**: POSIX-only, requires manual buffer management, handle cleanup, and retry loops. Error handling requires checking `errno` and manually advancing past invalid bytes.

- **`std::codecvt`**: Deprecated in C++17, removed in C++26. Was never widely used due to its complexity and poor integration with streams.

- **ICU, Boost.Text, or other libraries**: Capable but heavyweight dependencies that may not be appropriate for all projects.

None of these integrate with the ranges library. Converting between encodings requires extracting data from one container, passing it through a conversion API, and collecting into another container — losing the composability that makes ranges effective.

## Why WHATWG?

The WHATWG Encoding Standard defines precise behavior for every legacy encoding encountered on the web. Its specifications are:

- **Complete**: Defines exact mappings for all byte values, including error cases.
- **Tested**: The Web Platform Tests provide thousands of test vectors.
- **Interoperable**: All major browsers implement identical behavior.
- **Practical**: Designed for real-world data, not theoretical purity.

Using WHATWG semantics means C++ programs can process web content identically to browsers, parse HTML and JSON files with the same error handling, and benefit from years of specification refinement.

## Why Forward-Only Iterators?

Legacy multibyte encodings like Shift_JIS, Big5, and GB18030 are fundamentally forward-only. Unlike UTF-8, where lead and continuation bytes occupy disjoint ranges, legacy encodings have overlapping byte values that can serve as either lead or trail bytes depending on context.

Consider Shift_JIS: the byte `0x81` is valid both as a lead byte starting a two-byte sequence and as a trail byte completing one. Given only a pointer into the middle of a byte stream, it is impossible to determine whether you are at a character boundary without scanning from the beginning.

Even with a known character boundary, stepping backward is impossible without external state. The byte sequence `0x81 0x81 0x81 0x40` could decode as either two ideographic commas followed by '@', or one comma followed by an ideographic space — depending on how many `0x81` bytes preceded it from the string's start.

This is not a limitation of the implementation but an inherent property of these encodings. The WHATWG state machines are designed as forward-only decoders, resetting their state at each character boundary. Bidirectional iteration would require caching all byte offsets during forward traversal.

Therefore, `transcode_view` provides only `input_range` — the strongest category that can be implemented correctly for all supported encodings.

# Design

## Overview

The proposal adds two primary view adaptors:

- **`transcode_view<C>`**: Decodes a byte range to `char32_t` (Unicode scalar values), or encodes a `char32_t` range to bytes. Errors are replaced with U+FFFD (decode) or '?' (encode).

- **`transcode_or_error_view<C>`**: Same transformation, but yields `expected<T, transcode_error>` so callers can inspect errors.

And one utility view:

- **`null_term_view`**: Adapts a pointer to a null-terminated string into a range, enabling `views::null_term(cstr) | views::transcode<codec::utf_8>`.

## Codec Enumeration

```cpp
enum class codec {
  // Unicode
  utf_8,
  utf_16be,
  utf_16le,

  // WHATWG special
  replacement,     // Always yields single U+FFFD
  x_user_defined,  // Maps 0x80-0xFF to U+F780-U+F7FF

  // Single-byte legacy
  ibm866, iso_8859_2, iso_8859_3, iso_8859_4,
  iso_8859_5, iso_8859_6, iso_8859_7, iso_8859_8,
  iso_8859_8_i, iso_8859_10, iso_8859_13, iso_8859_14,
  iso_8859_15, iso_8859_16, koi8_r, koi8_u, macintosh,
  windows_874, windows_1250, windows_1251, windows_1252,
  windows_1253, windows_1254, windows_1255, windows_1256,
  windows_1257, windows_1258, x_mac_cyrillic,

  // CJK multibyte
  gbk,
  gb18030,
  big5,
  shift_jis,
  euc_jp,
  iso_2022_jp,
  euc_kr,
};
```

The codec is a template parameter, not a runtime value, enabling complete elimination of dispatch overhead and allowing `constexpr` transcoding at compile time.

## Label Lookup for Runtime Codec Selection

The WHATWG Encoding Standard §4.2 defines over 200 string labels that map to canonical encodings. For example, `"shift_jis"`, `"sjis"`, `"x-sjis"`, `"ms_kanji"`, and `"csshiftjis"` all refer to the same Shift_JIS codec. This mapping is essential for:

- Parsing `charset` attributes in HTML `<meta>` tags
- Parsing `charset` parameters in `Content-Type` headers
- Web-compatible encoding sniffing algorithms

A `get_encoding` function provides runtime codec selection:

```cpp
constexpr std::optional<codec> get_encoding(std::string_view label) noexcept;
```

The function performs ASCII case-insensitive matching and strips leading/trailing ASCII whitespace, per WHATWG §4.2:

```cpp
static_assert(get_encoding("UTF-8") == codec::utf_8);
static_assert(get_encoding("utf-8") == codec::utf_8);
static_assert(get_encoding("  utf-8  ") == codec::utf_8);
static_assert(get_encoding("shift_jis") == codec::shift_jis);
static_assert(get_encoding("SJIS") == codec::shift_jis);
static_assert(get_encoding("x-sjis") == codec::shift_jis);
static_assert(get_encoding("unknown") == std::nullopt);
```

This enables runtime dispatch when parsing external metadata:

```cpp
std::string_view charset = parse_content_type(headers);
if (auto c = get_encoding(charset)) {
    // Use std::visit or switch to dispatch to appropriate view
}
```

## Error Handling

Two error handling strategies are provided:

1. **Replacement mode** (`transcode_view`): Invalid input sequences are replaced with U+FFFD (decode) or '?' (encode). This follows the WHATWG "replacement" error mode and is appropriate for most text processing where halting on errors is undesirable.

2. **Inspection mode** (`transcode_or_error_view`): Each output element is `expected<T, transcode_error>`, allowing callers to log errors, apply custom replacement, or abort processing.

The error enumeration provides specific failure reasons:

```cpp
enum class transcode_error {
  invalid_byte,        // Byte not valid in this position
  truncated_sequence,  // End of input mid-character
  overlong_encoding,   // UTF-8 overlong sequence
  surrogate_code_point,// UTF-8 encoded surrogate
  out_of_range,        // Code point > U+10FFFF
  unmapped_codepoint,  // Encode: no mapping exists
};
```

## Concepts

Two concepts constrain the input ranges:

```cpp
template <typename R>
concept legacy_byte_range =
    ranges::input_range<R> &&
    !is_array_v<remove_cvref_t<R>> &&
    (same_as<range_value_t<R>, char> ||
     same_as<range_value_t<R>, signed char> ||
     same_as<range_value_t<R>, unsigned char> ||
     same_as<range_value_t<R>, byte>);

template <typename R>
concept unicode_scalar_range =
    ranges::input_range<R> &&
    !is_array_v<remove_cvref_t<R>> &&
    same_as<range_value_t<R>, char32_t>;
```

Raw arrays are explicitly rejected to prevent silent inclusion of null terminators. Use `views::null_term` or wrap in `span`.

The `char8_t` type is not accepted because it implies UTF-8 encoding, conflicting with the codec parameter. Similarly, `wchar_t` is rejected due to its platform-dependent size.

## Constexpr Support

All views are `constexpr`-enabled. Decoding and encoding tables are compile-time constants. This enables:

```cpp
constexpr auto decoded = []() consteval {
    constexpr char bytes[] = {'\xC3', '\xA9'};
    std::span<const char> sp(bytes, 2);
    return *(sp | transcode<codec::utf_8>).begin();
}();
static_assert(decoded == U'é');
```

## Pipe Syntax

The views support both function call and pipe syntax:

```cpp
// Function call
auto v1 = transcode_view<codec::utf_8>(bytes);

// Pipe
auto v2 = bytes | views::transcode<codec::utf_8>;

// Composition
auto v3 = bytes
    | views::transcode<codec::shift_jis>  // decode to char32_t
    | views::transcode<codec::utf_8>;     // encode to UTF-8 bytes
```

# Design Decisions

## Byte-Like Types, Not Character Types

The `legacy_byte_range` concept accepts `char`, `signed char`, `unsigned char`, and `std::byte` — the types that represent raw octets in C++. It explicitly rejects:

- **`char8_t`**: Implies UTF-8 encoding, which conflicts with explicit codec selection. If you have `char8_t` data, you already know it's UTF-8 — wrapping it in `transcode<codec::shift_jis>` would be a logic error.

- **`wchar_t`**: Platform-dependent width (16-bit on Windows, 32-bit on POSIX) makes it unsuitable as an interchange format. It also implies "already decoded" rather than "raw bytes."

- **`char16_t` / `char32_t`**: These are Unicode code unit types, not byte types. A range of `char16_t` is not a byte stream to be decoded — it's already decoded data that might need re-encoding.

This design reflects the reality of where encoded text comes from: network sockets (`recv` returns bytes), file I/O (`read` returns bytes), legacy APIs (return `char*`), and modern buffer types (`std::byte`). The transcoding views meet data where it lives rather than requiring conversion to a specific character type first.

```cpp
// All of these work:
std::vector<char> v1;
std::vector<unsigned char> v2;
std::vector<std::byte> v3;
std::span<const signed char> s;

v1 | transcode<codec::utf_8>;  // OK
v2 | transcode<codec::utf_8>;  // OK
v3 | transcode<codec::utf_8>;  // OK
s  | transcode<codec::utf_8>;  // OK

// These are rejected at compile time:
std::u8string u8s;
u8s | transcode<codec::utf_8>;  // Error: char8_t implies UTF-8
```

## Template Parameter vs Runtime Selection

The codec is a template parameter rather than a constructor argument. This was chosen because:

1. **Performance**: The compiler can inline codec-specific logic and eliminate dispatch overhead.
2. **Constexpr**: Runtime codec selection would prevent compile-time transcoding.
3. **Type safety**: Different codecs have different error characteristics that callers may want to handle differently.

Runtime selection can be layered on top via `variant` or `any` if needed.

## char32_t as the Interchange Type

Unicode scalar values are represented as `char32_t`, not `char8_t[]` sequences. This simplifies composition — a `char32_t` stream can be fed to any encoder without reparsing — and makes individual code point inspection trivial.

The tradeoff is that UTF-32 is space-inefficient, but since these are lazy views, no intermediate storage is created unless explicitly collected.

## Separation of Decode and Encode

Rather than a single "transcode from A to B" adaptor, decode and encode are separate operations composed via pipes. This enables:

- Processing Unicode without re-encoding: `bytes | decode<utf_8> | filter(...)`
- Encoding to multiple targets: `codepoints | tee(encode<utf_8>(...), encode<shift_jis>(...))`
- Clear mental model: decode produces `char32_t`, encode consumes `char32_t`

## No BOM Handling

BOM (Byte Order Mark) detection and handling is explicitly out of scope for these views. BOMs are a framing concern, not a transcoding concern. A separate `bom_filter` view or preprocessing step should handle BOM detection if needed.

## No Normalization

Unicode normalization (NFC, NFD, NFKC, NFKD) is orthogonal to encoding. A `normalize_view<form>` is a natural future addition but not part of this proposal.

# Testing

## Web Platform Tests

The implementation is validated against the WHATWG Web Platform Tests (WPT) for text encoding. These tests provide:

- Decode tests for each codec with representative byte sequences
- Error handling tests verifying U+FFFD replacement behavior
- Edge cases for truncated sequences, overlong UTF-8, surrogate encoding
- BOM handling (tested but BOM is stripped before our views)

Test vectors are extracted from WPT and converted to C++ data structures, enabling automated verification that the implementation matches browser behavior.

## Negative Compile Tests

Every concept constraint has a corresponding negative compile test — a `.cpp` file that must fail to compile with a specific diagnostic. This ensures:

- Raw arrays are rejected with a clear error message
- `char8_t` ranges are rejected (wrong for explicit codec selection)
- `wchar_t` ranges are rejected (non-portable width)
- Non-input ranges are rejected

## Consteval Tests

Every operation that should be `constexpr` has a `consteval` test verifying compile-time evaluation:

```cpp
constexpr auto result = []() consteval {
    constexpr char bytes[] = {'\xE2', '\x82', '\xAC'};
    std::span<const char> sp(bytes, 3);
    return *(sp | transcode<codec::utf_8>).begin();
}();
static_assert(result == U'€');
```

## Roundtrip Tests

For codecs where encode is defined, roundtrip tests verify that `encode(decode(bytes)) == bytes` for all valid inputs, and that `decode(encode(codepoints)) == codepoints` for all mapped code points.

# Impact on the Standard

This proposal adds new headers and does not modify existing standard library components. It is a pure extension.

## Feature Test Macro

```cpp
#define __cpp_lib_transcode_view 202XXXL
```

## Headers

```cpp
#include <transcode>   // transcode_view, transcode_or_error_view
#include <null_term>   // null_term_view
```

Or combined into `<ranges>` additions.

# Acknowledgements

- Zach Laine for `Boost.Text` and extensive work on Unicode in C++.
- Tom Honermann and SG16 for guidance on text encoding issues.
- The WHATWG for the Encoding Standard.
- The Web Platform Tests project for comprehensive test vectors.

# Relationship to Other Proposals

## P2728 Unicode in the Library

[@P2728R6] proposes transcoding views for UTF-8, UTF-16, and UTF-32 that operate on the strict Unicode character types (`char8_t`, `char16_t`, `char32_t`). This proposal overlaps in the UTF portions but differs in scope and philosophy:

| Aspect | P2728 | This Proposal |
|--------|-------|---------------|
| **Input types** | `char8_t`, `char16_t`, `char32_t` | `char`, `signed char`, `unsigned char`, `std::byte` |
| **UTF-16 model** | `char16_t` code units (native order) | Byte stream with explicit BE/LE |
| **Encodings** | UTF-8, UTF-16, UTF-32 only | 39 WHATWG encodings including legacy CJK |
| **Focus** | Strict Unicode processing | Web/legacy interoperability |
| **Use case** | Internal Unicode manipulation | External data ingestion |

The UTF-16 distinction is particularly important. P2728 operates on `char16_t`, which represents UTF-16 code units already in the platform's native byte order. This is appropriate for in-memory Unicode processing where the data originated from C++ code.

This proposal instead decodes UTF-16 from raw byte streams:

```cpp
// P2728: char16_t input (native byte order, already code units)
std::u16string s = u"Hello";
auto codepoints = s | std::uc::to_utf32;

// This proposal: byte input with explicit byte order
std::vector<std::byte> network_data = read_socket();
auto codepoints = network_data | transcode<codec::utf_16be>;
```

When UTF-16 data arrives from external sources — network protocols, file formats, binary blobs — it arrives as bytes with a specific byte order determined by the protocol or BOM, not by the platform. The `codec::utf_16be` and `codec::utf_16le` variants decode these byte streams correctly regardless of the host's native endianness.

The proposals are complementary:

- **P2728** is ideal when working with well-typed Unicode data within an application. Its strict typing catches encoding mismatches at compile time.

- **This proposal** is necessary when ingesting data from external sources — network protocols, file formats, databases — where bytes arrive as `char*` or `std::byte*` and the encoding is determined by metadata or heuristics, not the C++ type system.

A typical workflow might use this proposal to decode legacy-encoded input to `char32_t`, process it, then use P2728's facilities for UTF-to-UTF conversions within the application, and finally use this proposal again to encode output for a legacy system.

The UTF-8 decode/encode in this proposal follows WHATWG semantics (replacement character on error), which may differ slightly from P2728's error handling. Applications requiring strict UTF validation should prefer P2728 for that portion of the pipeline.

# References

- WHATWG Encoding Standard: <https://encoding.spec.whatwg.org/>
- Web Platform Tests: <https://github.com/nickvidal/nickvidal/web-platform-tests>
- Beman.Transcode reference implementation: <https://github.com/bemanproject/transcode>
