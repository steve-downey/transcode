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
| **Bulk decode → container** | ✅ `decode_to<C>(range)` | ✅ `decode_to(codec{},range)` | n/a ¹ | ✅ `ranges::to<u32string>()` |
| **Bulk encode → container** | ✅ `encode_to<C>(range)` | ✅ `encode_to(codec{},range)` | n/a ¹ | ✅ `ranges::to<u8string>()` |
| **Bulk transcode → container** | n/a ² | n/a ² | ✅ `iconv_transcode_to(range,f,t)` | n/a ² |
| **Bulk decode → output iter** | ✅ `decode_into<C>(range,out)` | ✅ `decode_into(codec{},range,out)` | n/a ¹ | ✅ `ranges::copy(v\|to_utf32, out)` |
| **Bulk encode → output iter** | ✅ `encode_into<C>(range,out)` | ✅ `encode_into(codec{},range,out)` | n/a ¹ | ✅ `ranges::copy(v\|to_utf8, out)` |
| **Bulk transcode → output iter** | n/a ² | n/a ² | ✅ `iconv_transcode_into(range,f,t,out)` | n/a ² |
| **Null-terminated input** | ✅ `views::null_term(ptr)` | ✅ `views::null_term(ptr)` | ✅ `views::null_term(ptr)` | n/a ³ |
| **Runtime label lookup** | ✅ `get_encoding("utf-8")` | n/a ⁴ | n/a (string labels are the API) | 🔴 |
| **Runtime transcode** | ✅ `transcode_string(src,from,to)` | 🔴 | 🔴 | 🔴 |
| **BOM sniffing** | ✅ `sniff_encoding(range)` | 🔴 | 🔴 | 🔴 |
| **Error type** | `whatwg_error` | `whatwg_error` | `iconv_error` | `utf_transcoding_error` |
| **Output element type** | `char32_t` | `char32_t` | `char` (raw bytes) | `char32_t` |
| **Input element type** | `char`/`unsigned char`/`byte` | `char`/`byte` (legacy) | `char` (any byte encoding) | `char8_t`/`char16_t`/`char32_t` |
| **constexpr** | ✅ | ✅ | 🔴 (OS syscall) | ✅ |

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
compile time through the type system.

### Bulk Operations

When you want a container rather than a lazy view:

```cpp
// Decode to vector
std::vector<char32_t> cps = beman::transcoding::decode_to<codec::utf_8>(bytes);

// Encode to string
std::string encoded = beman::transcoding::encode_to<codec::shift_jis>(codepoints);

// Output-iterator versions (zero-copy into existing containers)
beman::transcoding::decode_into<codec::utf_8>(bytes, std::back_inserter(vec));
```

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

### Competitive Comparison: UTF-8 Decode (57 KB English, ASCII-heavy)

This library is a naive scalar implementation — no SIMD, no hand-tuned assembly.
The comparison puts it in context against mature, production-optimized projects:

| Implementation | Mean | Throughput | Notes |
|----------------|------|-----------|-------|
| **simdutf** (SIMD ceiling) | — | ~21 GiB/s | AVX2 vectorized; not re-run with full corpus |
| **encoding_rs** (Rust) | — | ~17 GiB/s | SIMD-accelerated; not re-run with full corpus |
| **beman.transcode** (this library) | 24.1 µs | 2.2 GiB/s | Naive scalar, constexpr-capable |
| Raw `iconv()` (glibc) | 289 µs | 188 MiB/s | Block API, system implementation |
| `iconv_transcode_view` (this library) | 1.59 ms | 34 MiB/s | Per-byte range adaptor over iconv |

The ~9x gap between beman.transcode and simdutf is the cost of scalar
byte-at-a-time iteration vs SIMD bulk processing.  This is expected and
acceptable for a portable, constexpr-capable, standards-track implementation.
SIMD backends (simdutf, encoding_rs) could be plugged in behind the same range
interface in the future without changing user code.

### UTF-8 Decode: Multibyte (142 KB Arabic, 2-byte sequences)

| Implementation | Mean | Throughput |
|----------------|------|-----------|
| **beman.transcode** | 306 µs | 444 MiB/s |

The gap vs the ASCII-heavy English corpus reflects the per-byte overhead of
two-byte sequence processing.  SIMD implementations maintain near-constant
throughput on multi-byte text; scalar implementations do not.

### WHATWG Legacy Codecs (this library)

Decoded using WHATWG table-driven views.  Corpora are full Wikipedia Mars
articles in each language.

| Codec | Corpus | Mean | Throughput |
|-------|--------|------|-----------|
| windows-1251 | 11 KB Russian | < 1 ns | > 10 GiB/s ¹ |
| Shift-JIS | 27 KB Japanese | 34.5 µs | 771 MiB/s |
| UTF-8 (3-byte) | 40 KB Japanese | 30.4 µs | 1.2 GiB/s |
| GB18030 | 24 KB Chinese | 23.0 µs | 1.0 GiB/s |
| Big5 | 24 KB Trad. Chinese | 44.1 µs | 523 MiB/s |
| EUC-JP | 28 KB Japanese | 18.8 µs | 1.4 GiB/s |
| EUC-KR | 18 KB Korean | 32.6 µs | 520 MiB/s |
| ISO-2022-JP | 31 KB Japanese | 38.1 µs | 767 MiB/s |
| UTF-16LE | 6.2 MB War & Peace | 2.01 ms | 3.0 GiB/s |
| UTF-16BE | 6.2 MB War & Peace | 2.05 ms | 2.9 GiB/s |

¹ The windows-1251 random-access view is a sized range; GCC elides the
iteration loop entirely.  The view itself is O(1) per element via pointer
arithmetic on a 128-entry lookup table.

### iconv: Range Adaptor vs Bulk Operations

All measurements on real corpus data (57 KB English / 28 KB Japanese).

| Benchmark | Corpus | Mean | Throughput |
|-----------|--------|------|-----------|
| Raw `iconv()`: UTF-8 → UTF-32LE | 57 KB English | 289 µs | 188 MiB/s |
| `iconv_transcode_view`: same | 57 KB English | 1.59 ms | 34 MiB/s |
| **`iconv_transcode_to`**: same | 57 KB English | 310 µs | 176 MiB/s |
| **`iconv_transcode_into`**: same | 57 KB English | 461 µs | 118 MiB/s |
| Raw `iconv()`: Shift-JIS → UTF-8 | 28 KB Japanese | 362 µs | 74 MiB/s |
| `iconv_transcode_view`: same | 28 KB Japanese | 682 µs | 39 MiB/s |
| **`iconv_transcode_to`**: same | 28 KB Japanese | 143 µs | 186 MiB/s |

`iconv_transcode_view` pays ~5-10x overhead vs raw iconv due to per-byte
iteration.  The bulk APIs (`iconv_transcode_to`, `iconv_transcode_into`)
are within ~1.5-2.5x of raw iconv — matching the design goal — since they
call iconv with full buffers.  The extra overhead vs raw iconv is allocation
of the output container.

### Pluggable Codec Performance (Phase 4 APIs)

Single-element streaming views on a 4 KB synthetic corpus of upper-half bytes
(0xC0), decoding/encoding with a custom `table_codec<latin1_upper>`.

| API | Corpus | Mean | Throughput |
|-----|--------|------|-----------|
| `decode(latin1_codec{})` | 4 KB synthetic | < 1 ns | > 10 GiB/s ¹ |
| `encode(latin1_codec{})` | 4 KB synthetic | 249 ns | ~15 GiB/s ² |
| `decode_to<iso_8859_15>` | 11 KB Russian | 17.5 µs | 614 MiB/s |
| `encode_to<iso_8859_15>` (round-trip) | 11 KB Russian | 120 µs | 90 MiB/s |
| `decode_to<utf_8>` | 57 KB English | 31.7 µs | 1.7 GiB/s |
| `transcode<windows_1252, utf_8>` | 4 KB synthetic | 4.34 µs | 900 MiB/s |
| `transcode<utf_8, windows_1252>` | 57 KB English | 29.2 µs | 1.8 GiB/s |

¹ Random-access sized range; GCC elides the iteration loop (O(1) pointer
arithmetic).
² Encode is a 128-entry linear scan per codepoint; GCC auto-vectorizes
the loop over the 4 KB synthetic corpus.  Real-world mixed-content
throughput will be lower.

### UTF-8 Bulk Operations

| API | Corpus | Mean | Throughput |
|-----|--------|------|-----------|
| `decode_to<utf_8>` | 57 KB English | 24.1 µs | 2.2 GiB/s |
| `decode_to<utf_8>` | 3.1 MB War & Peace | 2.02 ms | 1.5 GiB/s |
| `encode_to<utf_8>` (round-trip) | 57 KB English | 79.5 µs | 687 MiB/s |
| `encode_to<utf_8>` (round-trip) | 3.1 MB War & Peace | 5.05 ms | 621 MiB/s |

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
