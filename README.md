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

> **TODO**: Provide bulk `iconv_transcode_to<std::string>(input, from, to)` and
> `iconv_transcode_into(input, from, to, output_iter)` operations that call raw
> `iconv()` with properly-sized buffers, bypassing per-byte iteration.  The view
> is ~15-20x slower than raw iconv; bulk operations should close that to ~1.5-2x.

### API Surface Matrix

Cross-reference of operations across all component families.  The P2728R12
(UTF transcoding) column shows alignment with the proposed standard UTF views.

| API | WHATWG | Pluggable | iconv | P2728R12 |
|-----|--------|-----------|-------|----------|
| **Codec identity** | `codec::utf_8` (enum) | `my_codec{}` (type) | `"UTF-8"` (string) | `char8_t`/`char16_t`/`char32_t` |
| **Decode view** | ✅ `whatwg_decode<C>` | ✅ `decode(codec)` | ✅ `iconv_transcode(from,to,buf)` | ✅ `views::to_utf32` |
| **Decode view (errors)** | ✅ `whatwg_decode_or_error<C>` | ✅ `decode_or_error(codec)` | ✅ `iconv_transcode_or_error(…)` | ✅ `views::to_utf32_or_error` |
| **Encode view** | ✅ `whatwg_encode<C>` | 🔴 | 🔴 | ✅ `views::to_utf8`, `to_utf16` |
| **Encode view (errors)** | ✅ `whatwg_encode_or_error<C>` | 🔴 | 🔴 | ✅ `views::to_utf8_or_error` |
| **Transcode pipeline** | ✅ `transcode<From,To>` | 🔴 | ✅ `iconv_transcode(from,to,buf)` | ✅ compose `to_utfN` views |
| **Bulk decode → vector** | ✅ `decode_to<C>(range)` | 🔴 | 🔴 | ✅ `ranges::to<u32string>()` |
| **Bulk encode → string** | ✅ `encode_to<C>(range)` | 🔴 | 🔴 | ✅ `ranges::to<u8string>()` |
| **Bulk decode → output iter** | ✅ `decode_into<C>(range,out)` | 🔴 | 🔴 | 🔴 |
| **Bulk encode → output iter** | ✅ `encode_into<C>(range,out)` | 🔴 | 🔴 | 🔴 |
| **Runtime label lookup** | ✅ `get_encoding("utf-8")` | 🔴 | n/a (labels are the API) | 🔴 |
| **Runtime transcode** | ✅ `transcode_string(…)` | 🔴 | 🔴 | 🔴 |
| **BOM sniffing** | ✅ `sniff_encoding(range)` | 🔴 | 🔴 | 🔴 |
| **Null-terminated input** | ✅ `views::null_term(ptr)` | ✅ `views::null_term(ptr)` | 🔴 | 🔴 |
| **Error enum** | `whatwg_error` | `decode_error` | `iconv_error` | `utf_transcoding_error` |
| **Codepoint type** | `char32_t` | `char32_t` | `char` (raw bytes) | `char32_t` |
| **Input type** | `char`/`unsigned char`/`byte` | `unsigned char` | `char` | `char8_t`/`char16_t`/`char32_t` |
| **constexpr** | ✅ | ✅ | 🔴 (system call) | ✅ |

Observations:

- The **pluggable codec** column has the most gaps — it lacks encode views, bulk
  operations, and runtime label lookup.  These should be filled so that custom
  codecs get the same convenience as built-in WHATWG codecs.
- The **iconv** column lacks bulk operations (`iconv_transcode_to`,
  `iconv_transcode_into`), encode-only views, and label lookup (inherently
  runtime).  The bulk operations are the highest-priority gap since they would
  eliminate the per-byte overhead.
- **Error enum names** diverge across columns: `whatwg_error`, `decode_error`,
  `iconv_error`, `utf_transcoding_error`.  These should either unify or
  explicitly document why they differ.
- **P2728R12** operates on a different axis (type-encoded UTF, not byte-oriented
  I/O), so 🔴 gaps between the columns are expected rather than defects.  The
  shared patterns are the `_or_error` suffix convention and `char32_t` as the
  codepoint type.

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
-fno-semantic-interposition` (Intel 13th-gen Alderlake, AVX2 enabled).  Corpora
sourced from Wikipedia Mars articles in each language.

### Competitive Comparison: UTF-8 Decode (586 B English, ASCII-heavy)

This library is a naive scalar implementation — no SIMD, no hand-tuned assembly.
The comparison puts it in context against mature, production-optimized projects:

| Implementation | Mean | Throughput | Notes |
|----------------|------|-----------|-------|
| **simdutf** (SIMD ceiling) | 26 ns | 21.5 GiB/s | AVX2 vectorized, validate+transcode |
| **encoding_rs** (Rust) | 32 ns | 17.4 GiB/s | SIMD-accelerated, production Mozilla code |
| **beman.transcode** (this library) | 234 ns | 2.4 GiB/s | Naive scalar, constexpr-capable |
| Raw `iconv()` (glibc) | 712 ns | 785 MiB/s | Block API, system implementation |
| `std::codecvt` (libstdc++) | 968 ns | 577 MiB/s | Deprecated, locale-dependent |
| `iconv_transcode_view` (this library) | 15.4 µs | 36 MiB/s | Per-byte range adaptor over iconv |

The ~9x gap between beman.transcode and simdutf is the cost of scalar
byte-at-a-time iteration vs SIMD bulk processing.  This is expected and
acceptable for a portable, constexpr-capable, standards-track implementation.
SIMD backends (simdutf, encoding_rs) could be plugged in behind the same range
interface in the future without changing user code.

### Competitive Comparison: Shift-JIS Decode (340 B Japanese)

| Implementation | Mean | Throughput |
|----------------|------|-----------|
| **encoding_rs** (Rust) | 395 ns | 821 MiB/s |
| **beman.transcode** (this library) | 261 ns | 1.2 GiB/s |
| Raw `iconv()` (glibc) | 977 ns | 332 MiB/s |

For legacy multi-byte CJK codecs, the WHATWG table-driven implementation is
competitive with encoding_rs and significantly faster than system iconv.

### UTF-8 Decode: Multibyte (640 B Arabic, 2-byte sequences)

| Implementation | Mean | Throughput |
|----------------|------|-----------|
| **simdutf** | 249 ns | 2.5 GiB/s |
| **beman.transcode** | 1025 ns | 596 MiB/s |

The gap widens on multi-byte text where SIMD branch-free algorithms excel.

### WHATWG Legacy Codecs (this library)

| Codec | Corpus | Mean | Throughput |
|-------|--------|------|-----------|
| Shift-JIS | 340 B Japanese | 261 ns | 1.2 GiB/s |
| UTF-8 | 504 B Japanese (3-byte) | 423 ns | 1.1 GiB/s |
| GB18030 | 98 B Chinese | 94 ns | 993 MiB/s |
| Big5 | 98 B Trad. Chinese | 111 ns | 842 MiB/s |
| EUC-JP | 93 B Japanese | 47 ns | 1.9 GiB/s |
| EUC-KR | 108 B Korean | 103 ns | 1.0 GiB/s |
| ISO-2022-JP | 111 B Japanese | 131 ns | 808 MiB/s |
| UTF-16LE | 936 B | 218 ns | 4.1 GiB/s |
| UTF-16BE | 936 B | 226 ns | 3.9 GiB/s |

### iconv Range Adaptor Overhead

| Benchmark | Mean | Throughput |
|-----------|------|-----------|
| Raw `iconv()`: UTF-8 → UTF-32LE | 712 ns | 785 MiB/s |
| `iconv_transcode_view`: same | 15.4 µs | 36 MiB/s |
| Raw `iconv()`: Shift-JIS → UTF-8 | 977 ns | 332 MiB/s |
| `iconv_transcode_view`: same | 10.9 µs | 30 MiB/s |

The ~15-20x overhead of the range adaptor vs raw iconv is the cost of per-byte
iteration through a block-oriented C API.  The adaptor's value is safety (no
resource leaks, no manual buffer management) and composability with other range
adaptors.

### Reproducing

```bash
# Build GCC-16 optimized config and generate full report
make bench-perf-report

# Competitive baselines (encoding_rs requires cargo; simdutf requires cmake flag)
make bench-perf-all                                          # core benchmarks
.build/build-perf/benchmark/Perf/beman.transcode.benchmarks.encoding_rs "[benchmark]"
.build/build-perf/benchmark/Perf/beman.transcode.benchmarks.simdutf "[benchmark]"

# Download full corpus (3 MiB War and Peace + Wikipedia articles)
uv run python tools/download_benchmark_corpora.py
```

Throughput formula: `corpus_bytes / mean_time_s / 2^20` MiB/s.

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
