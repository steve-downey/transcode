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
bibliography:
  - ../docs/whatwg/source.bib
  - ../docs/wpt/source.bib
  - ../docs/prior-art.bib
---

<!-- markdownlint-disable MD013 MD056 MD060 -->

## Abstract

> "It seems like a really messy situation and you're proposing to inherit that mess into the C++ standard." — Jan Schultke [@schultke-quote]

C++ has no way to convert text between character encodings.
Programs that must — mail gateways, HTML parsers, anything reading a legacy database — hand-roll the state machine or link ICU.

<!-- TODO(sdowney): answer Jan's objection here, in two or three sentences.
     The shape of the argument, for you to make in your own words: the mess is
     already inherited. The bytes arrive whether or not the standard can read
     them, and every C++ program that reads them today writes the loop in the
     Before column below. What the standard would be inheriting is not the mess
     but one pinned, tested description of it. Say whether you concede any part
     of the objection — the 39 codec tables are a real maintenance burden. -->

We propose separate WHATWG decode and encode adaptors, eager bulk decode and encode helpers, a convenience composed transcoder, and an `iconv`-based adaptor for encodings outside the WHATWG set.
The views are lazy and composable: they decode byte sequences to Unicode scalar values, encode scalar values back to bytes, and report errors either by WHATWG replacement or through `expected`.
The bulk helpers produce a container or fill an output iterator, for callers who want a result and not a pipeline.

Codec semantics follow the WHATWG Encoding Standard [@whatwg-encoding].
Where WHATWG does not reach, the optional `iconv` adaptor reaches the platform's own converter.

## Context and Motivation

IANA established a registry of character set names, but the specifications behind the names were often loose, and interoperability suffered.
C introduced multibyte encoding functions and POSIX standardized `iconv` to convert between them.
However, `iconv` is implementation-defined where it matters, differing between glibc, musl, and the various BSD and macOS implementations, and it depends on how the system was configured.
The C++ standard library later added `std::codecvt`.
It was deprecated in C++17 and removed in C++26.

The WHATWG Encoding Standard came later, and specifies the legacy encodings that are actually used on the web, which is a different set from the registry's.
It says exactly what to do with a malformed sequence.
C++ has since added `std::text_encoding`, which names an encoding but does not convert anything.

Everyone should use Unicode, and specifically UTF-8.
Most of the world's stored text predates that advice.
Postel's law applies: we must be conservative in what we emit, but liberal in what we accept.
Backend C++ systems process a great deal of data in legacy encodings, web interchange has not fully migrated, and email still generates and routes large volumes of non-UTF-8 text.

Worse, the data is frequently malformed.
Mail and web pages lie about their declared encoding, or concatenate text in mismatched encodings into one payload.
Error handling must therefore be explicit and composable, and it must be possible to choose the strategy at the call site.
Aborting on the first malformed byte is not available to a mail gateway.

## Comparison Table

### Decoding UTF-8 to Unicode code points

::: cmptable

#### Before: manual loop

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

#### After: `whatwg_decode`

```cpp
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  return input
    | whatwg_decode<codec::utf_8>
    | std::ranges::to<std::vector>();
}
```

:::

### Transcoding Shift_JIS to UTF-8

::: cmptable

#### Before: `iconv`

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

#### After: decode then encode

```cpp
std::string shift_jis_to_utf8(
    std::string_view input) {
  return input
    | whatwg_decode<codec::shift_jis>
    | whatwg_encode<codec::utf_8>
    | std::ranges::to<std::string>();
}
```

:::

### Converting a null-terminated C string

::: cmptable

#### Before: `mbstowcs`

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

#### After: `views::null_term`

```cpp
auto from_cstring(const char* s) {
  return views::null_term(s)
    | whatwg_decode<codec::utf_8>;
  // Returns lazy view of char32_t
  // No global state
  // Portable Unicode scalars
  // Single pass, errors yield U+FFFD
}
```

:::

### Processing with error visibility

::: cmptable

#### Before: manual tracking

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

#### After: `_or_error`

```cpp
for (auto r : input
    | whatwg_decode_or_error<codec::utf_8>) {
  if (r.has_value()) {
    process(*r);
  } else {
    log_warning(r.error());
    process(U'\xFFFD');
  }
}
```

:::

### Bulk conversion to owned storage

::: cmptable

#### Before: manual collection loop

```cpp
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  std::vector<char32_t> result;
  for (char32_t cp : input
      | whatwg_decode<codec::utf_8>) {
    result.push_back(cp);
  }
  return result;
}
```

#### After: `decode_to`

```cpp
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  return decode_to<codec::utf_8>(input);
}
```

:::

There is a principled reason not to propose `decode_to` / `encode_to` /
`decode_into` / `encode_into`.  The views satisfy the range concepts that
`std::ranges::to` and `std::ranges::copy` require, so the standard algorithms
already do the job, and a second spelling should have to pay for itself.
It does not pay in speed: there is no measurable difference between a named
`decode_to` and the equivalent `view | ranges::to` composition.

However, we propose them.  Converting a buffer to a container is the operation
users come for, and `decode_to<codec::utf_8>(input)` is what they look for
first.  Withholding the name does not stop anyone from wanting it; it only
moves the question from the library to every code review that touches the
library, and to every committee discussion of this paper.  Each helper is a
one-line forward to the pipeline it names, so the cost is four names and the
wording that specifies them.

The named helpers and the pipeline are the same operation, and the paper
specifies them that way:

```cpp
decode_to<C>(r)         is  r | whatwg_decode<C> | ranges::to<vector<char32_t>>()
encode_to<C, Cont>(r)   is  r | whatwg_encode<C> | ranges::to<Cont>()
decode_into<C>(r, out)  is  ranges::copy(r | whatwg_decode<C>, out)
encode_into<C>(r, out)  is  ranges::copy(r | whatwg_encode<C>, out)
```

<!-- TODO(sdowney): this ## Motivation and the ## Context and Motivation above
     are two motivation sections with the comparison table wedged between them.
     Decide the order you want: history and the case for the facility up top,
     then the table, then Status Quo / Why WHATWG / Why Forward-Only as design
     rationale — or fold Status Quo up into the first section and rename this
     one ## Design Rationale. -->

## Motivation

### The Status Quo

C++ has no standard facility for character encoding conversion. Programs must choose between:

- **`mbstowcs`/`wcstombs`**: Depends on global locale state, uses non-portable `wchar_t`, provides no error recovery, and supports only the locale's encoding.

- **`iconv`**: POSIX-only, requires manual buffer management, handle cleanup, and retry loops.
  Error handling requires checking `errno` and manually advancing past invalid bytes.

- **`std::codecvt`**: Deprecated in C++17, removed in C++26.
  Was never widely used due to its complexity and poor integration with streams.

- **ICU, Boost.Text, or other libraries**: Capable but heavyweight dependencies that may not be appropriate for all projects.

None of these integrate with the ranges library.
Converting between encodings requires extracting data from one container, passing it through a conversion API, and collecting into another container — losing the composability that makes ranges effective.

### Why WHATWG?

The WHATWG Encoding Standard defines precise behavior for every legacy encoding encountered on the web.
It gives exact byte-to-scalar mappings for all byte values, including every error case, and the Web Platform Tests [@wpt-encoding] pin that behavior across all major browsers.

Targeting it means a C++ program decodes a page the way the browser that fetched it did, and parses HTML and JSON with the same error handling.

### Why Forward-Only Iterators?

Legacy multibyte encodings like Shift_JIS, Big5, and GB18030 are fundamentally forward-only.
Unlike UTF-8, where lead and continuation bytes occupy disjoint ranges, legacy encodings have overlapping byte values that can serve as either lead or trail bytes depending on context.

Consider Shift_JIS: the byte `0x81` is valid both as a lead byte starting a two-byte sequence and as a trail byte completing one.
Given only a pointer into the middle of a byte stream, it is impossible to determine whether you are at a character boundary without scanning from the beginning.

Even with a known character boundary, stepping backward is impossible without external state.
The byte sequence `0x81 0x81 0x81 0x40` could decode as either two ideographic commas followed by '@', or one comma followed by an ideographic space — depending on how many `0x81` bytes preceded it from the string's start.

This is not a limitation of the implementation but an inherent property of these encodings.
The WHATWG state machines are designed as forward-only decoders, resetting their state at each character boundary.
Bidirectional iteration would require caching all byte offsets during forward traversal.

Therefore, the proposed decode, encode, and `iconv` transcode views all model `input_range`.
That is the strongest category that can be implemented uniformly across all supported encodings, especially for stateful multibyte codecs.
The WHATWG decode and encode views preserve stronger properties when the codec and base range permit it: const-qualified iteration is supported for const-compatible bases, `common_range` and `borrowed_range` propagate from the base when implementable, and indexed codecs lift to `random_access_range`.
The eager bulk helpers are layered on top of those pipelines when callers want collected results.

## Design

### Overview

The proposal adds four user-facing adaptor families:

- **`whatwg_decode_view<C>` / `whatwg_decode<C>`**: Decodes a byte range to `char32_t` (Unicode scalar values).
  Decode errors are replaced with U+FFFD.

- **`whatwg_encode_view<C>` / `whatwg_encode<C>`**: Encodes a `char32_t` range to bytes.
  Encode failures are replaced with `'?'`.

- **`whatwg_decode_or_error_view<C>` / `whatwg_decode_or_error<C>`** and **`whatwg_encode_or_error_view<C>` / `whatwg_encode_or_error<C>`**:
  Error-reporting variants yielding `expected<T, whatwg_error>`.

- **`iconv_transcode_view` / `iconv_transcode(from, to, buf)`** and **`iconv_transcode_or_error_view` / `iconv_transcode_or_error(from, to, buf)`**:
  Runtime-selected transcoding through the platform `iconv` implementation, using caller-provided staging storage.

Four eager bulk helpers, each specified as the pipeline it names:

- **`decode_to<C>(r)` / `encode_to<C, Container>(r)`**: Collect the decoded or encoded result into a container.

- **`decode_into<C>(r, out)` / `encode_into<C>(r, out)`**: Write the result through an output iterator.

And three utility entry points:

- **`transcode<From, To>`**: Convenience composition of `whatwg_decode<From>` followed by `whatwg_encode<To>`.

- **`transcode_string(source, from, to)`**: Eager convenience helper for runtime-selected WHATWG transcoding by codec or by label.

- **`null_term_view` / `views::null_term`**: Adapts a pointer to a null-terminated string into a range.
  This enables `views::null_term(cstr) | whatwg_decode<codec::utf_8>`.

### Codec Enumeration

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

The codec is a template parameter and not a runtime value, so there is no dispatch overhead, and transcoding can run at compile time.

### Label Lookup for Runtime Codec Selection

The WHATWG Encoding Standard §4.2 defines over 200 string labels that map to canonical encodings.
For example, `"shift_jis"`, `"sjis"`, `"x-sjis"`, `"ms_kanji"`, and `"csshiftjis"` all refer to the same Shift_JIS codec.
This mapping is essential for:

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

### Error Handling

Two WHATWG error handling strategies are provided:

1. **Replacement mode** (`whatwg_decode`, `whatwg_encode`, and `transcode`): Invalid input sequences are replaced with U+FFFD on decode or `'?'` on encode.
  This follows the WHATWG replacement model and is appropriate for most text processing where halting on errors is undesirable.

2. **Inspection mode** (`whatwg_decode_or_error`, `whatwg_encode_or_error`): Each output element is `expected<T, whatwg_error>`.
  This allows callers to log errors, apply custom replacement, or abort processing.

For WHATWG codecs, the error enumeration provides specific failure reasons:

```cpp
enum class whatwg_error {
  invalid_byte,        // Byte not valid in this position
  truncated_sequence,  // End of input mid-character
  overlong_encoding,   // UTF-8 overlong sequence
  surrogate_code_point,// UTF-8 encoded surrogate
  out_of_range,        // Code point > U+10FFFF
  unmapped_codepoint,  // Encode: no mapping exists
};
```

#### Relationship to `utf_transcoding_error` (P2728)

P2728 (beman.utf_view) defines `utf_transcoding_error` for its `_or_error` views.
The two enumerations are distinct types.  Most values correspond directly:

| `whatwg_error` | `utf_transcoding_error` (P2728) | Notes |
|---|---|---|
| `truncated_sequence` | `truncated_utf8_sequence` | Same concept; `whatwg_error` applies to all codecs, not just UTF-8 |
| `overlong_encoding` | `overlong` | Identical semantics |
| `surrogate_code_point` | `encoded_surrogate` | Identical semantics for UTF-8 and UTF-32 |
| `out_of_range` | `out_of_range` | Identical semantics |
| `invalid_byte` | `invalid_utf8_leading_byte` + `unexpected_utf8_continuation_byte` | P2728 distinguishes two invalid-byte cases; WHATWG conflates them because its spec produces U+FFFD for both without differentiating |
| `unmapped_codepoint` | *(no equivalent)* | See below |
| *(not needed)* | `unpaired_high_surrogate` / `unpaired_low_surrogate` | UTF-16 code-unit-level concepts; this library operates on byte streams with explicit endianness (`codec::utf_16be`/`utf_16le`), so there is no paired/unpaired distinction at the byte level |

`unmapped_codepoint` is the reason reusing `utf_transcoding_error` directly is not possible.
UTF-8, UTF-16, and UTF-32 can encode every Unicode scalar value, so P2728 never needs to report "no mapping exists for this code point."
Legacy codecs cannot: windows-1252 has no representation for U+4E2D (中), Shift-JIS has no representation for U+00E9 (é), and so on for most of the Unicode range.
`whatwg_encode_or_error<codec::windows_1252>` must be able to report this condition through the same error type as `whatwg_encode_or_error<codec::utf_8>`, so the error type must accommodate it uniformly across every codec.

<!-- TODO(sdowney): pick a number and use it everywhere.  The enum in Codec
     Enumeration lists 40 entries; this paragraph said 39 and the P2728 table
     still says "39 WHATWG encodings".  WHATWG's own count depends on whether
     replacement and x-user-defined are counted. -->

The `iconv` adaptors use a separate `iconv_error` enumeration:

```cpp
enum class iconv_error {
  invalid_sequence,
  incomplete_sequence,
  output_full,
};
```

### Concepts

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

Raw arrays are explicitly rejected to prevent silent inclusion of null terminators.
Use `views::null_term` for null-terminated strings or wrap counted byte buffers in `span`.

The `char8_t` type is not accepted because it implies UTF-8 encoding, conflicting with the codec parameter.
Similarly, `wchar_t` is rejected due to its platform-dependent size.

### Constexpr Support

All views are `constexpr`-enabled.
Decoding and encoding tables are compile-time constants.
This enables:

```cpp
constexpr auto decoded = []() consteval {
    constexpr char bytes[] = {'\xC3', '\xA9'};
    std::span<const char> sp(bytes, 2);
    return *(sp | whatwg_decode<codec::utf_8>).begin();
}();
static_assert(decoded == U'é');
```

### Pipe Syntax

The views support both function call and pipe syntax:

```cpp
// Function call
auto v1 = whatwg_decode<codec::utf_8>(bytes);

// Pipe
auto v2 = bytes | whatwg_decode<codec::utf_8>;

// Composition
auto v3 = bytes
  | whatwg_decode<codec::shift_jis>
  | whatwg_encode<codec::utf_8>;

// Convenience closure
auto v4 = bytes | transcode<codec::shift_jis, codec::utf_8>;
```

### Range Properties and Const Support

All of the views are valid `input_range`s, but the WHATWG views preserve stronger range properties when possible instead of flattening everything to the weakest category.

- **Const iteration**: `whatwg_decode`, `whatwg_encode`, and their `_or_error` variants support `begin() const` and `end() const` when the underlying range supports compatible const iteration.
  This includes owning views after construction.

- **`common_range` and `borrowed_range`**: These properties are preserved from the base range when the codec allows it.

- **Indexed codecs**: Single-byte and other indexed codecs can provide `random_access_range` behavior for both regular and `_or_error` WHATWG views.
  Stateful codecs such as UTF-8 and ISO-2022-JP remain forward-only or input-only where required by their decoding model.

### User-Facing Examples

The separate decode and encode entry points make the pipeline direction explicit:

```cpp
std::vector<char> utf8 = {'H', 'i', '\xE2', '\x82', '\xAC'};

auto code_points = utf8 | whatwg_decode<codec::utf_8>;
auto round_trip  = code_points | whatwg_encode<codec::utf_8>;
```

Collecting into owned storage uses `std::ranges::to`, or the named helper:

```cpp
std::string bytes = "caf\xE9";
auto scalars = bytes
  | whatwg_decode<codec::windows_1252>
  | std::ranges::to<std::vector<char32_t>>();

auto utf8 = scalars
  | whatwg_encode<codec::utf_8>
  | std::ranges::to<std::string>();

// The same two conversions, named:
auto scalars2 = decode_to<codec::windows_1252>(bytes);
auto utf8_2   = encode_to<codec::utf_8, std::string>(scalars2);
```

Writing into an output iterator uses `std::ranges::copy`, or `decode_into` and
`encode_into`:

```cpp
std::vector<char32_t> scalars;
std::ranges::copy(bytes | whatwg_decode<codec::utf_8>,
                  std::back_inserter(scalars));

std::string encoded;
encode_into<codec::utf_8>(scalars, std::back_inserter(encoded));
```

The convenience `transcode<From, To>` adaptor is useful when the intermediate Unicode range is not otherwise needed:

```cpp
std::string latin1 = "caf\xE9";
auto utf8 = latin1
  | transcode<codec::windows_1252, codec::utf_8>
  | std::ranges::to<std::string>();
```

Null-terminated inputs compose through `views::null_term`:

```cpp
const char* cstr = "hello";
std::u32string scalars = views::null_term(cstr)
  | whatwg_decode<codec::utf_8>
  | std::ranges::to<std::u32string>();
```

Runtime labels can be resolved through WHATWG's alias table before dispatch:

```cpp
if (auto utf8 = transcode_string(bytes, "shift_jis", "utf-8")) {
  consume(*utf8);
}
```

For encodings outside the WHATWG set, the `iconv` adaptor provides a runtime-selected escape hatch:

```cpp
std::array<char, 256> buffer{};

std::vector<char> utf32le = input
  | iconv_transcode("UTF-8", "UTF-32LE", std::span(buffer))
  | std::ranges::to<std::vector>();
```

## Design Decisions

### Byte-Like Types, Not Character Types

The `legacy_byte_range` concept accepts `char`, `signed char`, `unsigned char`, and `std::byte` — the types that represent raw octets in C++.
It explicitly rejects:

- **`char8_t`**: Implies UTF-8 encoding, which conflicts with explicit codec selection.
  If you have `char8_t` data, you already know it's UTF-8 — passing it to `whatwg_decode<codec::shift_jis>` would be a logic error.

- **`wchar_t`**: Platform-dependent width (16-bit on Windows, 32-bit on POSIX) makes it unsuitable as an interchange format. It also implies "already decoded" where what is wanted is "raw bytes."

- **`char16_t` / `char32_t`**: These are Unicode code unit types. A range of `char16_t` is already decoded data that might need re-encoding.

This design reflects where encoded text comes from: network sockets (`recv` returns bytes), file I/O (`read` returns bytes), legacy APIs (return `char*`), and modern buffer types (`std::byte`).
The views take the bytes as they arrive, with no conversion step first.

Rejecting `char8_t` will inconvenience anyone holding a `u8string` that is not
in fact UTF-8, which does happen; `u8string` is sometimes just the string type
a codebase settled on. However, accepting it would mean accepting a type whose
whole purpose is to assert an encoding, and then ignoring what it asserts.
Such a caller can say `as_bytes` and get what they want, visibly.

```cpp
// All of these work:
std::vector<char> v1;
std::vector<unsigned char> v2;
std::vector<std::byte> v3;
std::span<const signed char> s;

v1 | whatwg_decode<codec::utf_8>;  // OK
v2 | whatwg_decode<codec::utf_8>;  // OK
v3 | whatwg_decode<codec::utf_8>;  // OK
s  | whatwg_decode<codec::utf_8>;  // OK

// These are rejected at compile time:
std::u8string u8s;
u8s | whatwg_decode<codec::utf_8>;  // Error: char8_t implies UTF-8
```

### Template Parameter vs Runtime Selection

The codec is a template parameter rather than a constructor argument.
This was chosen because:

1. **Performance**: The compiler can inline codec-specific logic and eliminate dispatch overhead.
2. **Constexpr**: Runtime codec selection would prevent compile-time transcoding.
3. **Type safety**: Different codecs have different error characteristics that callers may want to handle differently.

There is a real cost to this. The codec a program needs is very often not known
until run time: it arrives in a `Content-Type` header or a `<meta>` tag, and a
template parameter cannot be spelled from a string. However, runtime selection
layers cleanly on top of a compile-time set, through `variant`, through `any`,
or through the `transcode_string` and `get_encoding` entry points proposed here.
The reverse does not hold. A runtime-dispatched design cannot be recovered into
a `constexpr` one.

### char32_t as the Interchange Type

Unicode scalar values are represented as `char32_t`, and not as `char8_t[]` sequences. This simplifies composition — a `char32_t` stream can be fed to any encoder without reparsing — and makes individual code point inspection trivial.

UTF-32 is the wasteful choice. A `char32_t` per scalar is four bytes where UTF-8 would often use one — but these are lazy views, so nothing is stored unless a caller collects it, and a single code point is trivial to inspect. All other interchange types are worse.

### Separation of Decode and Encode

Rather than a single primary "transcode from A to B" adaptor, decode and
encode are separate operations composed via pipes. This enables:

- Processing Unicode without re-encoding: `bytes | whatwg_decode<codec::utf_8> | filter(...)`
- Encoding to multiple targets: `codepoints | tee(whatwg_encode<codec::utf_8>(...), whatwg_encode<codec::shift_jis>(...))`
- Clear mental model: decode produces `char32_t`, encode consumes `char32_t`

The `transcode<From, To>` convenience closure is layered on top of this composition model for the common case where the intermediate `char32_t` stream is not observed.
The eager bulk helpers are layered on the same decode and encode semantics, but package the common collection patterns directly.

### Leading BOM Stripping for UTF Decoders

The WHATWG UTF decoders strip a leading BOM when it matches the selected codec.
This matches browser behavior and the Web Platform Tests [@wpt-encoding] for UTF-8, UTF-16LE, and UTF-16BE.

When the codec is not yet known, a separate `sniff_encoding` helper can examine the byte stream for a BOM before dispatch:

```cpp
if (auto codec = sniff_encoding(bytes)) {
  // Dispatch according to the detected BOM.
}
```

Non-matching BOMs are not stripped.
For example, a UTF-8 decoder does not remove a UTF-16LE BOM prefix.

### No Normalization

Unicode normalization (NFC, NFD, NFKC, NFKD) is orthogonal to encoding.
A `normalize_view<form>` is a natural future addition but not part of this proposal.

## Methods

### Authoritative Source Acquisition

The reference implementation does not hand-transcribe encoding tables or conformance fixtures.
Instead, it stores pristine upstream source material for both the WHATWG Encoding Standard indexes [@whatwg-encoding] and the relevant Web Platform Tests (WPT) [@wpt-encoding] under version control, together with provenance metadata and checksums.

This serves three purposes:

- It keeps the implementation tied to the same normative and de facto-interoperability sources that define browser behavior.
- It makes source updates auditable during review, because regenerated artifacts can be compared against the exact upstream inputs that produced them.
- It avoids network access during normal builds and test runs; regeneration is an explicit maintenance step, and never a hidden part of compilation.

### Python Preprocessing for Tables and Conformance Vectors

The project uses small Python helpers to transform upstream WHATWG and WPT data into C++-friendly artifacts.
The generators are checked in and unit-tested; regeneration is a maintenance step rather than part of the build.

For codec tables, Python scripts download the WHATWG index files, record provenance, and generate checked-in lookup tables for single-byte and multibyte codecs.
This avoids manually maintaining large arrays of code points while keeping the generated results reviewable and deterministic.

For conformance tests, a separate Python generator parses the JavaScript-authored WPT fixtures and emits C++ headers containing byte sequences, expected Unicode scalar values, and descriptive labels.
That translation step is necessary because the authoritative tests are written for the web platform, in JavaScript.
By converting them once into ordinary C++ data, the test suite can run as native unit tests without embedding a JavaScript engine or reimplementing the WPT harness at runtime.

The generators are themselves unit-tested.
That keeps the extraction logic honest: if WPT uses surrogate pairs, replacement characters, BOM-sensitive cases, fatal-mode expectations, or codec-specific fixture shapes, those parsing rules are validated independently from the transcoder under test.

## Testing

### Web Platform Tests

The implementation is validated against the WHATWG Web Platform Tests (WPT) [@wpt-encoding] for text encoding.
These tests provide:

- Decode tests for each codec with representative byte sequences
- Error handling tests verifying U+FFFD replacement behavior
- Edge cases for truncated sequences, overlong UTF-8, surrogate encoding
- BOM handling (tested but BOM is stripped before our views)

Test vectors are extracted from WPT and converted to C++ data structures using the preprocessing pipeline described in Methods.
This enables automated verification that the implementation matches browser behavior while keeping the executable tests entirely within the native C++ test harness.

### Negative Compile Tests

Every concept constraint has a corresponding negative compile test — a `.cpp` file that must fail to compile with a specific diagnostic.
This ensures:

- Raw arrays are rejected with a clear error message
- `char8_t` ranges are rejected (wrong for explicit codec selection)
- `wchar_t` ranges are rejected (non-portable width)
- Non-input ranges are rejected

### Consteval Tests

Every operation that should be `constexpr` has a `consteval` test verifying compile-time evaluation:

```cpp
constexpr auto result = []() consteval {
    constexpr char bytes[] = {'\xE2', '\x82', '\xAC'};
    std::span<const char> sp(bytes, 3);
    return *(sp | whatwg_decode<codec::utf_8>).begin();
}();
static_assert(result == U'€');
```

### Roundtrip Tests

For codecs where encode is defined, roundtrip tests verify that `encode(decode(bytes)) == bytes` for all valid inputs, and that `decode(encode(codepoints)) == codepoints` for all mapped code points.

Bulk collection through `ranges::to` and output-iterator paths via `ranges::copy` are covered as part of the view tests to verify that the views satisfy the required range concepts.

## Impact on the Standard

This proposal adds new headers and does not modify existing standard library components. It is a pure extension.

### Feature Test Macro

```cpp
#define __cpp_lib_transcode_view 202XXXL
```

### Headers

```cpp
#include <transcode>   // whatwg_decode, whatwg_encode, transcode, iconv_transcode, transcode_string
#include <null_term>   // null_term_view
```

Or combined into `<ranges>` additions.

## Acknowledgements

- Zach Laine for `Boost.Text` and extensive work on Unicode in C++.
- Tom Honermann and SG16 for guidance on text encoding issues.
- The WHATWG for the Encoding Standard [@whatwg-encoding].
- The Web Platform Tests project [@wpt-encoding] for comprehensive test vectors.

## Prior Art

Tom Honermann's `text_view` library and paper [@P0244R2; @text-view] established an early range-based model for enumerating encoded text as code points. That work demonstrated the value of making encodings explicit in the type system, and the ergonomic cost of making iteration report decoding status.

Zach Laine's `Boost.Text` [@boost-text] showed how modern range-based text facilities can integrate naturally with C++ algorithms and pipelines. Its Unicode-oriented design, normalization support, and text abstractions helped clarify where a proposal aimed at external byte streams and legacy encodings should stay narrower.

JeanHeyd Meneide's `P1629R1` proposal and `ztd.text` implementation [@P1629R1; @ztd-text] explored a broader text and transcoding library design with both eager and lazy interfaces, explicit encoding objects, and strong attention to extensibility. This proposal is narrower, aimed at browser-compatible WHATWG transcoding, although the split between lazy views and eager bulk helpers comes from the same recognition that both styles are needed.

Henri Sivonen's `encoding_rs` [@encoding-rs] is the implementation prior art for WHATWG-compatible transcoding specifically. It targets the Encoding Standard directly, provides both streaming and bulk APIs, and validates against the web platform tests without giving up its fast paths. That is where the emphasis here on exact WHATWG semantics, WPT-derived conformance coverage, and eager helpers alongside the range adaptors comes from.

<!-- TODO(sdowney): the four prior-art paragraphs all run the same shape —
     "X's Y did Z, and that informed this."  Nothing here says where you think
     any of them went wrong.  One owned judgment would carry more than the
     other three paragraphs combined: text_view's ergonomics, or the breadth of
     ztd.text against the narrowness proposed here.  Your call which. -->


## Relationship to Other Proposals

### P2728 Unicode in the Library

[@P2728R13] proposes transcoding views for UTF-8, UTF-16, and UTF-32 that operate on the strict Unicode character types (`char8_t`, `char16_t`, `char32_t`). This proposal overlaps in the UTF portions but differs in scope and philosophy:

| Aspect | P2728 | This Proposal |
| -------- | ------- | --------------- |
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
auto codepoints = network_data | whatwg_decode<codec::utf_16be>;
```

When UTF-16 data arrives from external sources — network protocols, file formats, binary blobs — it arrives as bytes with a specific byte order determined by the protocol or BOM, not by the platform. The `codec::utf_16be` and `codec::utf_16le` variants decode these byte streams correctly regardless of the host's native endianness.

The proposals are complementary:

- **P2728** is ideal when working with well-typed Unicode data within an application.
  Its strict typing catches encoding mismatches at compile time.

- **This proposal** is necessary when ingesting data from external sources — network protocols, file formats, databases — where bytes arrive as `char*` or `std::byte*`.
  Metadata or heuristics determine the encoding; the C++ type system does not.

A typical workflow might use this proposal to decode legacy-encoded input to `char32_t`, process it, then use P2728's facilities for UTF-to-UTF conversions within the application, and finally use this proposal again to encode output for a legacy system.

The UTF-8 decode/encode in this proposal follows WHATWG semantics (replacement character on error), which may differ slightly from P2728's error handling.
Applications requiring strict UTF validation should prefer P2728 for that portion of the pipeline.

### API Surface Comparison

The three implementation families support the same operations, where their
encoding models allow it.  The P2728R13 column shows the parallel design in the
proposed standard UTF transcoding views.

Legend: *yes* implemented, *n/a* the encoding model does not support this,
*no* not yet implemented.

| API | WHATWG | Pluggable codec | iconv | P2728R13 |
|-----|--------|-----------------|-------|----------|
| **Codec identity** | `codec::utf_8` enum | `my_codec{}` type | `"UTF-8"` string | `char8_t`/`char16_t`/`char32_t` |
| **Decode view** | `whatwg_decode<C>` | `decode(codec)` | `iconv_transcode(f,t,buf)` | `views::to_utf32` |
| **Decode or-error view** | `whatwg_decode_or_error<C>` | `decode_or_error(codec)` | `iconv_transcode_or_error(...)` | `views::to_utf32_or_error` |
| **Encode view** | `whatwg_encode<C>` | `encode(codec)` | *n/a* 1 | `views::to_utf8` / `to_utf16` |
| **Encode or-error view** | `whatwg_encode_or_error<C>` | `encode_or_error(codec)` | *n/a* 1 | `views::to_utf8_or_error` |
| **Transcode pipeline** | `transcode<From,To>` | `pluggable_transcode(f,t)` | `iconv_transcode(f,t,buf)` | compose via `\|` |
| **Bulk decode to container** | `decode_to<C>(r)` | `decode_to(codec,r)` | *n/a* 1 | `v\|ranges::to<>()` |
| **Bulk encode to container** | `encode_to<C,Cont>(r)` | `encode_to(codec,r)` | *n/a* 1 | `v\|ranges::to<>()` |
| **Bulk transcode to container** | *n/a* 2 | *n/a* 2 | `iconv_transcode_to(range,f,t)` | *n/a* 2 |
| **Bulk decode to output iter** | `decode_into<C>(r,out)` | `decode_into(codec,r,out)` | *n/a* 1 | `ranges::copy(v, out)` |
| **Bulk encode to output iter** | `encode_into<C>(r,out)` | `encode_into(codec,r,out)` | *n/a* 1 | `ranges::copy(v, out)` |
| **Bulk transcode to output iter** | *n/a* 2 | *n/a* 2 | `iconv_transcode_into(range,f,t,out)` | *n/a* 2 |
| **Null-terminated input** | `views::null_term(ptr)` | `views::null_term(ptr)` | `views::null_term(ptr)` | *n/a* 3 |
| **Runtime label lookup** | `get_encoding("utf-8")` | *n/a* 4 | *n/a*, string labels are the API | *n/a* 5 |
| **Runtime transcode** | `transcode_string(src,from,to)` | *n/a* 4 | `iconv_transcode_to(r,f,t)` | *n/a* 5 |
| **BOM sniffing** | `sniff_encoding(range)` | *n/a* 6 | *n/a* 6 | *n/a* 7 |
| **Error type** | `whatwg_error` | `whatwg_error` | `iconv_error` | `utf_transcoding_error` |
| **Output element type** | `char32_t` | `char32_t` | `char` (raw bytes) | `char32_t` |
| **Input element type** | `char`/`byte` (legacy) | `char`/`byte` (legacy) | `char` (any byte encoding) | `char8_t`/`char16_t`/`char32_t` |
| **constexpr** | yes | yes | *no*, POSIX userland library | yes |

**Notes on n/a entries:**

1. **iconv is a byte↔byte transcoder.**  It converts one byte encoding directly
to another without exposing an intermediate `char32_t` stage.  Encode (from
`char32_t` to bytes) and decode (from bytes to `char32_t`) as separate steps
are outside its model.

2. **WHATWG and pluggable codecs compose decode and encode.**  Bulk decode and
bulk encode are proposed as named helpers, each specified as the
`view | ranges::to<Container>()` or `ranges::copy(view, output)` pipeline it
names.  There is no separate bulk transcode helper, because transcode is
`decode | encode` composed with `|`.  iconv converts byte to byte in a single
pass and exposes that as a first-class bulk operation.

3. **P2728 operates on typed Unicode character types** (`char8_t`, `char16_t`,
`char32_t`); `views::null_term` produces a range of `char`.  Bridging the two
requires a reinterpret step that is outside both proposals.

4. **Pluggable codecs are identified by C++ type.**  Codec
selection happens at compile time through the type system; there is no runtime
name-to-codec registry by design.  Runtime transcode is similarly outside
the model: you compose `decode(codec_a{}) | encode(codec_b{})` at compile time.

5. **P2728 is type-based.**  Codec selection is
determined by the character types (`char8_t`, `char16_t`, `char32_t`), so
runtime label lookup and runtime transcode are outside its model.

6. **BOM sniffing is a property of the byte stream.**  Individual codecs know nothing about it.
`sniff_encoding()` examines the first bytes of a stream to detect UTF-8/16/32
BOMs and returns the appropriate `codec` enum value.  This is a WHATWG-specific
facility; pluggable codecs and iconv operate on already-identified encodings.

7. **P2728 operates on in-memory typed data** where the encoding is known from
the type.  BOM detection is an I/O concern; a separate endian-converting view
proposal addresses byte-order issues.

All four columns share the `_or_error` suffix for the error-surfacing variants,
`char32_t` as the interchange type, and composition with `|`.

<!-- TODO(sdowney): this is the end of the paper and it currently stops on a
     table.  Put the question to the room here.  Candidates: whether SG16 wants
     the named bulk helpers at all (the argument is above, and it is the one
     thing in this paper that is pure ergonomics); whether the iconv adaptor
     belongs in the same paper as the WHATWG views or should be split out. -->

## References

- WHATWG Encoding Standard [@whatwg-encoding]
- Web Platform Tests [@wpt-encoding]
- Beman.Transcode reference implementation: <https://github.com/bemanproject/transcode>
