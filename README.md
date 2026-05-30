# beman.transcode

A C++23 header-only library for Unicode transcoding using ranges and views.
Provides two transcoding backends with a shared pipeline-oriented interface:

- **WHATWG codecs** — a compile-time-safe, constexpr-capable implementation of
  the [WHATWG Encoding Standard](https://encoding.spec.whatwg.org/) covering 39
  codecs (UTF-8, all ISO-8859 and Windows code pages, CJK multi-byte encodings
  including GB18030, Big5, Shift-JIS, EUC-JP/KR, ISO-2022-JP)
- **iconv range adaptor** — a C++ ranges wrapper around the POSIX/system `iconv`
  API, giving safe, composable access to whatever encodings the platform provides

Most C and C++ code doing encoding conversion today uses `iconv` (directly, or
through libraries that call it).  The iconv view gives that existing ecosystem a
zero-overhead ranges interface — no resource leaks, no manual buffer management,
composable with other range adaptors — while the WHATWG views provide a
portable, header-only alternative with standardized error semantics for the
codecs web and network protocols actually use.

Part of the [Beman Project](https://github.com/bemanproject), targeting C++29
standardization.

> "It seems like a really messy situation and you're proposing to inherit that
> mess into the C++ standard."
> — Jan Schultke, personal correspondence

## Design: Byte-Oriented I/O Transcoding

This library operates on **byte-like types** (`char`, `signed char`,
`unsigned char`, `std::byte`) — the types you get from files, sockets, and
memory-mapped I/O.  It deliberately does not use `char8_t`, `char16_t`, or
`char32_t` as input types.  This is a different, complementary use case from
the type-based encoding approach in [P2728](https://wg21.link/P2728) (Eddie
Nolan's Unicode Transcoding paper), which uses distinct character types to
statically track encoding at the type-system level.

The two approaches serve non-overlapping problems:

- **P2728 (type-based)**: transcoding between `std::u8string`, `std::u16string`,
  `std::u32string` — the encoding is carried in the type, and the compiler
  prevents mixing them
- **beman.transcode (byte-based)**: transcoding I/O byte streams whose encoding
  is determined at runtime or by protocol (HTTP `Content-Type`, BOM sniffing,
  database column metadata, `iconv` string labels) — the bytes are just bytes
  until you decode them

The only Unicode scalar type this library uses is `char32_t` — as the decoded
codepoint type.  Using `char32_t` for codepoints is the agreed pattern in WG21
rather than introducing a distinct `code_point` type: since UTF-32 code units
and Unicode code points are numerically identical, distinguishing them at the
type level just introduces syntactic noise and "range casts" that alias without
converting.

## Core APIs

### Decode Views — Bytes to Unicode

Pipe any byte range through `whatwg_decode<codec>` to get a lazy `char32_t` view:

```cpp
#include <beman/transcode/transcode.hpp>

std::vector<char32_t> decode_utf8(std::string_view input) {
    std::vector<char32_t> result;
    for (char32_t cp : input | beman::transcoding::whatwg_decode<beman::transcoding::codec::utf_8>) {
        result.push_back(cp);
    }
    return result;
}
```

For explicit error visibility, use the `_or_error` variant which yields
`std::expected<char32_t, whatwg_error>`:

```cpp
for (auto result : input | beman::transcoding::whatwg_decode_or_error<codec::utf_8>) {
    if (!result.has_value()) {
        // result.error() is whatwg_error::invalid_byte, ::truncated_sequence, etc.
    }
}
```

C strings work via `views::null_term`:

```cpp
auto decode_cstring(const char* text) {
    return beman::transcoding::views::null_term(text)
         | beman::transcoding::whatwg_decode<beman::transcoding::codec::utf_8>;
}
```

### Encode Views — Unicode to Bytes

The reverse direction encodes Unicode scalars back to bytes:

```cpp
// char32_t range → char range (UTF-8 bytes)
auto encoded = codepoints | beman::transcoding::whatwg_encode<codec::utf_8>;
```

### Transcode — Any Encoding to Any Other

Compose decode and encode into a single pipeline:

```cpp
#include <beman/transcode/transcode.hpp>

std::string shift_jis_to_utf8(std::string_view input) {
    std::string result;
    for (char byte : input | beman::transcoding::transcode<codec::shift_jis, codec::utf_8>) {
        result.push_back(byte);
    }
    return result;
}
```

For runtime codec selection (e.g., from HTTP headers):

```cpp
auto result = beman::transcoding::transcode_string(data, "shift_jis", "utf-8");
// returns std::optional<std::string>
```

### The Pluggable Codec Protocol

Implement `decode_codec` or `encode_codec` to add your own encoding:

```cpp
template <typename C>
concept decode_codec = std::semiregular<C> &&
    requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
        { c.decode_one(iter, sentinel) } -> std::same_as<decode_result>;
    };
```

For single-byte table-lookup codecs, use the provided `table_codec` template:

```cpp
inline constexpr std::array<char32_t, 128> my_table = { /* upper 128 codepoints */ };
using my_codec = beman::transcoding::table_codec<my_table>;

auto decoded = input | beman::transcoding::decode(my_codec{});
```

See [`examples/custom_single_byte_decoder.cpp`](examples/custom_single_byte_decoder.cpp)
for a complete working example.

### iconv Range Adaptor — System Encoding Support

If your code already uses `iconv` — or uses a library that does — the iconv
range adaptor is the interoperability path.  On most POSIX systems, `iconv` is
the system's encoding engine: glibc, musl, ICU, and platform-specific
implementations all expose the same `iconv_open`/`iconv`/`iconv_close` API.
Any encoding your system supports is available through this view, with no need
to reimplement the codec.

The raw `iconv` API requires manual buffer allocation, pointer arithmetic, error
code inspection, and careful resource cleanup.  The range adaptor handles all of
that:

```cpp
#include <beman/transcode/transcode.hpp>
#include <array>

std::array<char, 256> buffer{};
for (char byte : input | beman::transcoding::iconv_transcode("UTF-8", "UTF-32LE", std::span(buffer))) {
    output.push_back(byte);
}
```

Compare with the raw `iconv` equivalent — 30+ lines of buffer management, error
handling, and resource cleanup — in
[`examples/paper_iconv_view.cpp`](examples/paper_iconv_view.cpp).

An `_or_error` variant yields `std::expected<char, iconv_error>` instead of
replacement characters, for applications that need to distinguish
`invalid_sequence`, `incomplete_sequence`, and `output_full` conditions.

Key design choices:
- **External buffer**: caller provides the working `std::span<char>` — no hidden
  heap allocation
- **Dependency injection**: the `iconv_functions` struct accepts callable pointers
  for `iconv_open`/`iconv`/`iconv_close`, enabling test mocking and allowing
  alternative iconv implementations (e.g., libiconv vs glibc)
- **Move-only iterator**: safeguards the uncopyable `iconv_t` handle — copying
  the iterator would double-free

Bulk operations `iconv_transcode_to<std::string>(input, from, to)` and
`iconv_transcode_into(input, from, to, output_iter)` are now available.
They call raw `iconv()` with properly-sized buffers, bypassing per-byte
iteration.  See the Performance section for measured overhead vs raw iconv.

### API Surface Matrix

All three implementation families now have matching API surfaces for the
operations their encoding model supports.  The P2728R12 (UTF transcoding)
column shows the parallel standard proposal for type-safe UTF-to-UTF conversion.

Legend: ✅ implemented · n/a architectural model doesn't support this ·
🔴 not yet implemented

| API | WHATWG | Pluggable codec | iconv | P2728R12 |
|-----|--------|-----------------|-------|----------|
| **Codec identity** | `codec::utf_8` enum | `my_codec{}` type | `"UTF-8"` string | `char8_t`/`char16_t`/`char32_t` |
| **Decode view** | ✅ `whatwg_decode<C>` | ✅ `decode(codec)` | ✅ `iconv_transcode(f,t,buf)` | ✅ `views::to_utf32` |
| **Decode or-error view** | ✅ `whatwg_decode_or_error<C>` | ✅ `decode_or_error(codec)` | ✅ `iconv_transcode_or_error(…)` | ✅ `views::to_utf32_or_error` |
| **Encode view** | ✅ `whatwg_encode<C>` | ✅ `encode(codec)` | n/a ¹ | ✅ `views::to_utf8` / `to_utf16` |
| **Encode or-error view** | ✅ `whatwg_encode_or_error<C>` | ✅ `encode_or_error(codec)` | n/a ¹ | ✅ `views::to_utf8_or_error` |
| **Transcode pipeline** | ✅ `transcode<From,To>` | ✅ `pluggable_transcode(f,t)` | ✅ `iconv_transcode(f,t,buf)` | ✅ compose views via `\|` |
| **Bulk decode → container** | ✅ `v\|ranges::to<>()` | ✅ `v\|ranges::to<>()` | n/a ¹ | ✅ `v\|ranges::to<>()` |
| **Bulk encode → container** | ✅ `v\|ranges::to<>()` | ✅ `v\|ranges::to<>()` | n/a ¹ | ✅ `v\|ranges::to<>()` |
| **Bulk transcode → container** | n/a ² | n/a ² | ✅ `iconv_transcode_to(range,f,t)` | n/a ² |
| **Bulk decode → output iter** | ✅ `ranges::copy(v, out)` | ✅ `ranges::copy(v, out)` | n/a ¹ | ✅ `ranges::copy(v, out)` |
| **Bulk encode → output iter** | ✅ `ranges::copy(v, out)` | ✅ `ranges::copy(v, out)` | n/a ¹ | ✅ `ranges::copy(v, out)` |
| **Bulk transcode → output iter** | n/a ² | n/a ² | ✅ `iconv_transcode_into(range,f,t,out)` | n/a ² |
| **Null-terminated input** | ✅ `views::null_term(ptr)` | ✅ `views::null_term(ptr)` | ✅ `views::null_term(ptr)` | n/a ³ |
| **Runtime label lookup** | ✅ `get_encoding("utf-8")` | n/a ⁴ | n/a (string labels are the API) | n/a ⁵ |
| **Runtime transcode** | ✅ `transcode_string(src,from,to)` | n/a ⁴ | ✅ `iconv_transcode_to(r,f,t)` | n/a ⁵ |
| **BOM sniffing** | ✅ `sniff_encoding(range)` | n/a ⁶ | n/a ⁶ | n/a ⁷ |
| **Error type** | `whatwg_error` | `whatwg_error` | `iconv_error` | `utf_transcoding_error` |
| **Output element type** | `char32_t` | `char32_t` | `char` (raw bytes) | `char32_t` |
| **Input element type** | `char`/`unsigned char`/`byte` | `char`/`byte` (legacy) | `char` (any byte encoding) | `char8_t`/`char16_t`/`char32_t` |
| **constexpr** | ✅ | ✅ | 🔴 (POSIX userland library) | ✅ |

**Notes on n/a entries:**

¹ **iconv is a byte↔byte transcoder.** It converts one byte encoding to another
without exposing an intermediate `char32_t` codepoint stage.  "Encode" (from
`char32_t` to bytes) and "decode" (from bytes to `char32_t`) as distinct steps
are outside its model — it only offers transcode.

² **WHATWG and pluggable codecs separate decode and encode.** "Bulk transcode"
is just `encode_to(decode_to(range))` — there is no single-pass transcode
operation because the intermediate `char32_t` range is the natural composition
point.  iconv is the exception here: it performs single-pass byte→byte
conversion more efficiently than a two-step decode+encode.

³ **P2728 operates on typed Unicode character types** (`char8_t`, `char16_t`,
`char32_t`).  `views::null_term` produces a range of `char`, which is a
different type; adapting the two requires a reinterpret step.

⁴ **Pluggable codecs are identified by C++ type**, not by name.  There is no
runtime name-to-codec registry by design — codec selection happens at
compile time through the type system.  Runtime transcode is similarly outside
the model: you compose `decode(codec_a{}) | encode(codec_b{})` at compile time.

⁵ **P2728 is type-based**, not string-label-based.  Codec selection is
determined by the character types (`char8_t`, `char16_t`, `char32_t`), so
runtime label lookup and runtime transcode are outside its model.

⁶ **BOM sniffing is a property of the byte stream**, not of individual codecs.
`sniff_encoding()` examines the first bytes of a stream to detect UTF-8/16/32
BOMs and returns the appropriate `codec` enum value.  This is a WHATWG-specific
facility; pluggable codecs and iconv operate on already-identified encodings.

⁷ **P2728 operates on in-memory typed data** where the encoding is known from
the type.  BOM detection is an I/O concern; a separate endian-converting view
proposal addresses byte-order issues.

### Bulk Operations

The views compose directly with `std::ranges::to` and `std::ranges::copy` —
no dedicated bulk API is needed:

```cpp
using namespace beman::transcoding;

// Decode to vector — view | ranges::to
auto cps = bytes | whatwg_decode<codec::utf_8> | std::ranges::to<std::vector<char32_t>>();

// Encode to string — view | ranges::to
auto encoded = codepoints | whatwg_encode<codec::shift_jis> | std::ranges::to<std::string>();

// Output-iterator — ranges::copy
std::ranges::copy(bytes | whatwg_decode<codec::utf_8>, std::back_inserter(vec));
```

This is deliberate: `decode_to` / `encode_to` convenience wrappers exist in
`detail/bulk_transcode.hpp` but are not proposed for standardization.  The
standard library's `ranges::to` and `ranges::copy` already provide the
necessary functionality with no measurable performance difference.

## Codec Identifiers: WHATWG, iconv, and `std::text_encoding`

Encodings can be identified by IANA name, by WHATWG label, or by the string
labels that a system's `iconv` accepts.  In most cases these overlap — you can
look up a WHATWG codec by its IANA name or any of its aliases.  Where WHATWG goes
further is in nailing down the *exact algorithm and data tables* for each codec.
This matters most for encodings where there was historical diverging practice:
different implementations of "Shift_JIS" or "Big5" could disagree on edge cases,
unmapped byte values, or error recovery.  The WHATWG standard eliminates that
ambiguity by specifying precisely what every byte sequence means.

**`std::text_encoding`** (P1885) provides IANA charset names as a C++ vocabulary
type.  It identifies WHICH encoding a text uses but does not define HOW to
decode or encode — it is purely for labeling.  The explicit WHATWG API in
`beman.transcode` reflects the tighter algorithmic specification but does not
supersede `std::text_encoding`; the two serve different roles.

**POSIX `iconv`** uses string labels (`"UTF-8"`, `"SHIFT_JIS"`, `"ISO-8859-1"`)
to identify codecs at runtime.  The set of available encodings depends on the
system — glibc's iconv supports hundreds.  Whether any given iconv
implementation accepts `std::text_encoding` labels is purely quality-of-
implementation; the standard does not require it.  It may become recommended
practice for a compiler to identify an iconv function that interprets its
`"literal"` and `"locale"` encodings, but this is not currently required.  If
your codebase already uses iconv (or a library that wraps it — this includes
most C and C++ programs doing encoding conversion today), the
`iconv_transcode_view` accepts the same string labels you already have.

`beman.transcode` provides:

- **Compile-time WHATWG dispatch** via `codec::utf_8`, `codec::shift_jis`, etc. —
  enum constants used as non-type template parameters for zero-overhead codec
  selection and fully constexpr-capable decoding
- **Runtime label resolution** via `get_encoding("utf-8")` and
  `transcode_string(data, "shift_jis", "utf-8")` — for cases where the encoding
  name comes from user input, HTTP headers, or HTML meta tags
- **System iconv interop** via `iconv_transcode("UTF-8", "EUC-TW", buffer)` —
  string labels passed directly to the platform's iconv, giving access to every
  encoding the system supports without this library needing to implement it

## Performance

Measured with GCC 16.1.1, `-O3 -march=native -mtune=native -flto=auto
-fno-semantic-interposition` (Intel 13th-gen Alderlake, AVX2 enabled).
Corpora are full Wikipedia Mars articles and the Project Gutenberg War and
Peace text, downloaded via `uv run python tools/download_benchmark_corpora.py`.

### Competitive Comparison: UTF-8 Decode (57 KB English → char32_t)

This library is a naive scalar implementation — no SIMD, no hand-tuned assembly.
The comparison puts it in context against mature, production-optimized projects:

| Implementation | Mean | Throughput | Notes |
|----------------|------|-----------|-------|
| **simdutf** (SIMD ceiling) | 6.93 µs | 7.7 GiB/s | AVX2 vectorized, UTF-8→UTF-32 |
| **beman.transcode** (this library) | 21.4 µs | 2.5 GiB/s | Naive scalar, constexpr-capable |
| `mbstowcs` (glibc C89) | 31.1 µs | 1.7 GiB/s | C standard library, locale-dependent |
| `std::codecvt` (libstdc++ C++11) | 75 µs | 724 MiB/s | Deprecated C++17, removed C++26 |
| `iconv_transcode_to` (this library) | 83 µs | 653 MiB/s | Bulk API, pre-sized buffer |
| Raw `iconv()` (glibc, 4 KB buffer) | 334 µs | 163 MiB/s | Block API, looping over 4 KB |
| `iconv_transcode_view` (this library) | 351 µs | 155 MiB/s | Batched range adaptor over iconv |

The ~3x gap between beman.transcode and simdutf is the cost of scalar
byte-at-a-time iteration vs SIMD bulk processing.  This is expected and
acceptable for a portable, constexpr-capable, standards-track implementation.
SIMD backends could be plugged in behind the same range interface in the future
without changing user code.

`mbstowcs` (C89, still current) and `std::codecvt` (deprecated C++17, removed
C++26) represent the standard library's transcoding facilities.  `mbstowcs` is
a simple C API with no virtual dispatch overhead, which is why it outperforms
`std::codecvt` by ~2x.  Both are locale-dependent and UTF-only.
`iconv_transcode_to` matches `std::codecvt` throughput while supporting all
iconv encodings, not just UTF.

### UTF-8 Decode: Multibyte (142 KB Arabic, 2-byte sequences)

| Implementation | Mean | Throughput |
|----------------|------|-----------|
| **simdutf** (SIMD ceiling) | 58.6 µs | 2.3 GiB/s |
| **beman.transcode** | 306 µs | 442 MiB/s |

The gap widens to ~5x on multi-byte text where SIMD branch-free algorithms
excel.  Scalar implementations pay per-byte continuation checks.

### Shift-JIS → UTF-8 (28 KB Japanese, produces output)

The critical comparison for legacy CJK codecs: each implementation decodes
Shift-JIS and produces UTF-8 output bytes.

| Implementation | Mean | Throughput | Notes |
|----------------|------|-----------|-------|
| **beman.transcode** streaming (`transcode` pipe) | 80 µs | 332 MiB/s | Lazy views, `count_elements` only |
| **encoding_rs** (Rust bulk C FFI) | 103 µs | 258 MiB/s | Single Rust call, opaque to optimizer |
| **`iconv_transcode_to`** (this library) | 120 µs | 222 MiB/s | Bulk iconv, pre-sized buffer |
| **beman.transcode** bulk (`encode_to` + `decode_to`) | 237 µs | 112 MiB/s | Two-step, produces `std::string` |
| Raw `iconv()` (4 KB buffer) | 369 µs | 72 MiB/s | System iconv, many calls |
| `iconv_transcode_view` (streaming) | 368 µs | 72 MiB/s | Batched range adaptor over iconv |

The WHATWG table-driven decoder in this library's streaming view (80 µs, lazy
evaluation) is competitive with encoding_rs's Rust implementation (103 µs,
writing to a buffer).  The encoding_rs C FFI boundary is opaque to the C++
optimizer — the same effect that any range adaptor wrapping a bulk API
experiences.

The two-step bulk approach (237 µs) pays for the intermediate `char32_t` vector
allocation plus the Shift-JIS encode table search.

### Non-UTF → Non-UTF: EUC-JP → Shift-JIS (28 KB Japanese)

The problem case for a decode-codepoint-encode architecture: transcoding
between two legacy byte encodings that both represent the same characters.

| Implementation | Mean | Throughput | Notes |
|----------------|------|-----------|-------|
| **`iconv_transcode_to`** (single pass) | 144 µs | 185 MiB/s | Direct byte→byte via glibc iconv |
| **beman.transcode** streaming (`transcode` pipe) | 1.12 ms | 24 MiB/s | decode view \| encode view |
| **beman.transcode** bulk (`encode_to` + `decode_to`) | 1.30 ms | 20 MiB/s | Two-step, char32_t intermediate |

`iconv_transcode_to` is 7-9x faster because it performs a single-pass byte→byte
conversion inside glibc's codec tables (decades of optimization).  The
beman.transcode approach must
decode every byte to char32_t (EUC-JP table lookup), then encode each codepoint
back to Shift-JIS (linear scan of a 128-entry table per character).  The linear
scan in `encode_one()` is the dominant cost.

For non-UTF cross-encoding work, prefer `iconv_transcode_to` when both encodings
are available on the system.  The streaming views are appropriate when you need
composability with other range adaptors or when the intermediate `char32_t`
representation is needed for processing (filtering, normalization, etc).

### WHATWG Legacy Codecs (this library)

Decoded using WHATWG table-driven views.  Corpora are full Wikipedia Mars
articles in each language.

| Codec | Corpus | Mean | Throughput |
|-------|--------|------|-----------|
| windows-1251 | 11 KB Russian | < 1 ns | > 10 GiB/s ¹ |
| Shift-JIS | 28 KB Japanese | 19.8 µs | 1.3 GiB/s |
| UTF-8 (3-byte) | 40 KB Japanese | 27.8 µs | 1.4 GiB/s |
| GB18030 | 24 KB Chinese | 22.0 µs | 1.0 GiB/s |
| Big5 | 24 KB Trad. Chinese | 38.8 µs | 594 MiB/s |
| EUC-JP | 28 KB Japanese | 16.5 µs | 1.5 GiB/s |
| EUC-KR | 18 KB Korean | 30.8 µs | 549 MiB/s |
| ISO-2022-JP | 31 KB Japanese | 31.1 µs | 938 MiB/s |
| UTF-16LE | 6.2 MB War & Peace | 1.84 ms | 3.3 GiB/s |
| UTF-16BE | 6.2 MB War & Peace | 1.81 ms | 3.3 GiB/s |

¹ The windows-1251 random-access view is a sized range; GCC elides the
iteration loop entirely.  The view itself is O(1) per element via pointer
arithmetic on a 128-entry lookup table.

### iconv: Range Adaptor vs Bulk Operations

All measurements on real corpus data.

| Benchmark | Corpus | Mean | Throughput |
|-----------|--------|------|-----------|
| Raw `iconv()` (4 KB buffer) | 57 KB English UTF-8→UTF-32LE | 334 µs | 163 MiB/s |
| **`iconv_transcode_to`** | 57 KB English UTF-8→UTF-32LE | 83 µs | 653 MiB/s |
| `iconv_transcode_into` | 57 KB English UTF-8→UTF-32LE | 470 µs | 116 MiB/s |
| `iconv_transcode_view` | 57 KB English UTF-8→UTF-32LE | 351 µs | 155 MiB/s |
| Raw `iconv()` (4 KB buffer) | 28 KB Japanese SJIS→UTF-8 | 369 µs | 72 MiB/s |
| **`iconv_transcode_to`** | 28 KB Japanese SJIS→UTF-8 | 120 µs | 222 MiB/s |
| `iconv_transcode_view` | 28 KB Japanese SJIS→UTF-8 | 368 µs | 72 MiB/s |
| **`iconv_transcode_to`** | 28 KB Japanese EUC-JP→SJIS | 144 µs | 185 MiB/s |

`iconv_transcode_to` is **faster than naive raw iconv** because it pre-sizes
the output buffer to `input_size * 4`, making a single iconv call for the
entire conversion instead of looping over a 4 KB buffer.

`iconv_transcode_view` now batches input — for contiguous ranges it passes the
entire remaining input to iconv in one call per output-buffer fill, matching
raw iconv throughput within ~15%.  The view's residual overhead vs
`iconv_transcode_to` is the iterator machinery and the composability cost of
yielding one byte at a time through `operator++`.

### Pluggable Codec Performance (Phase 4 APIs)

Single-element streaming views on a 4 KB synthetic corpus of upper-half bytes
(0xC0), decoding/encoding with a custom `table_codec<latin1_upper>`.

| API | Corpus | Mean | Throughput |
|-----|--------|------|-----------|
| `decode(latin1_codec{})` | 4 KB synthetic | < 1 ns | > 10 GiB/s ¹ |
| `encode(latin1_codec{})` | 4 KB synthetic | 221 ns | ~18 GiB/s ² |
| `view\|ranges::to` decode iso-8859-15 | 11 KB Russian | 15.7 µs | 684 MiB/s |
| `view\|ranges::to` encode iso-8859-15 | 11 KB Russian | 131 µs | 82 MiB/s |
| `view\|ranges::to` decode UTF-8 | 57 KB English | 25.2 µs | 2.1 GiB/s |
| `transcode<windows_1252, utf_8>` | 4 KB synthetic | 5.88 µs | 664 MiB/s |
| `transcode<utf_8, windows_1252>` | 57 KB English | 27.8 µs | 1.9 GiB/s |

¹ Random-access sized range; GCC elides the iteration loop (O(1) pointer
arithmetic).
² Encode is a 128-entry linear scan per codepoint; GCC auto-vectorizes
the loop over the 4 KB synthetic corpus.  Real-world mixed-content
throughput will be lower.

### UTF-8 via `view | ranges::to`

| API | Corpus | Mean | Throughput |
|-----|--------|------|-----------|
| `whatwg_decode<utf_8> \| ranges::to` | 57 KB English | 21.4 µs | 2.5 GiB/s |
| `whatwg_decode<utf_8> \| ranges::to` | 3.1 MB War & Peace | 1.77 ms | 1.7 GiB/s |
| decode + encode round-trip `\| ranges::to` | 57 KB English | 75.5 µs | 721 MiB/s |
| decode + encode round-trip `\| ranges::to` | 3.1 MB War & Peace | 4.77 ms | 658 MiB/s |

### Reproducing

```bash
# Download full corpus (Wikipedia articles + 3 MB War and Peace)
uv run python tools/download_benchmark_corpora.py

# Build GCC-16 optimized config and run all core benchmarks
make bench-perf-all

# Competitive baselines (encoding_rs requires cargo; simdutf requires cmake flag)
.build/build-perf/benchmark/Perf/beman.transcode.benchmarks.encoding_rs "[benchmark]"
.build/build-perf/benchmark/Perf/beman.transcode.benchmarks.simdutf "[benchmark]"
```

Throughput formula: `input_bytes / mean_time_s / 2^20` MiB/s.

---

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++23 standard or greater
* CMake 3.30 or later
* (Test Only) GoogleTest

You can disable building tests by setting CMake option `BEMAN_TRANSCODE_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 16-13   | C++26-C++23   | libstdc++         |
| Clang      | 22-19   | C++26-C++23   | libstdc++, libc++*|
| Clang      | 18      | C++26-C++23   | libc++            |

* `libc++` on Clang 20+ is currently temporarily excluded from the CI matrix due to an upstream compiler bug causing a constraint recursion crash (`depends on itself`) when `std::expected` is used within `std::vector` combined with our iterators. We are tracking this upstream.
| Clang      | 18      | C++23         | libstdc++         |
| Clang      | 17      | C++26-C++23   | libc++            |
| AppleClang | latest  | C++26-C++23   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.transcode into your project

### Build

You can build transcode using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.transcode without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

#### Vcpkg

The preferred way to install transcode is via vcpkg. To do so, after installing vcpkg
itself, you need to add support for the Beman project's [vcpkg
registry](https://github.com/bemanproject/vcpkg-registry) by configuring a
`vcpkg-configuration.json` file (which transcode [provides](vcpkg-configuration.json)).

Then, simply run `vcpkg install beman-transcode`.

#### Manual

To install beman.transcode globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── transcode
│           ├── transcode.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.transcode
            ├── beman.transcode-config-version.cmake
            ├── beman.transcode-config.cmake
            └── beman.transcode-targets.cmake
```

### CMake Configuration

If you installed beman.transcode to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.transcode` package to define the `beman::transcode` CMake
target:

```cmake
find_package(beman.transcode REQUIRED)
```

You will then need to add `beman::transcode` to the link libraries of any libraries or
executables that include `beman.transcode` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::transcode)
```

### Using beman.transcode

To use `beman.transcode` in your C++ project,
include an appropriate `beman.transcode` header from your source code.

```c++
#include <beman/transcode/transcode.hpp>
```

> [!NOTE]
>
> `beman.transcode` headers are to be included with the `beman/transcode/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <transcode.hpp>`) is unsupported.
