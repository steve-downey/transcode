# beman.transcode

A C++23 header-only library for Unicode transcoding using ranges and views.
Implements the [WHATWG Encoding Standard](https://encoding.spec.whatwg.org/),
providing compile-time-safe pipeline-oriented views that transform byte sequences
to Unicode codepoints and back.  Covers 39 codecs including UTF-8, all ISO-8859
and Windows code pages, and CJK multi-byte encodings (GB18030, Big5, Shift-JIS,
EUC-JP/KR, ISO-2022-JP).

Part of the [Beman Project](https://github.com/bemanproject), targeting C++29
standardization.

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

### iconv as a Range Adaptor

Wraps the POSIX `iconv` API as a C++ range adaptor — no manual buffer juggling,
no `iconv_close` leaks:

```cpp
#include <beman/transcode/transcode.hpp>
#include <array>

std::array<char, 256> buffer{};
for (char byte : input | beman::transcoding::iconv_transcode("UTF-8", "UTF-32LE", std::span(buffer))) {
    output.push_back(byte);
}
```

Compare with the raw iconv equivalent (30+ lines of buffer management, error
handling, and resource cleanup) in
[`examples/paper_iconv_view.cpp`](examples/paper_iconv_view.cpp).

Key design choices:
- **External buffer**: caller provides the working `std::span<char>` — no hidden
  heap allocation
- **Dependency injection**: the `iconv_functions` struct accepts callable pointers
  for `iconv_open`/`iconv`/`iconv_close`, enabling test mocking
- **Move-only iterator**: safeguards the uncopyable `iconv_t` handle

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

## Codec Identifiers: WHATWG vs `std::text_encoding`

`std::text_encoding` (P1885) provides IANA charset *names* — it identifies WHICH
encoding a text uses, but does not define HOW to decode or encode.  It is a
vocabulary type for labeling, not an implementation.

The [WHATWG Encoding Standard](https://encoding.spec.whatwg.org/) defines the
actual byte-level decode and encode *algorithms* — the same algorithms browsers
execute when they process `<meta charset="...">`.  It specifies exact error
recovery semantics per codec (replacement character U+FFFD on decode error, `?`
on encode error), alias consolidation (e.g., "ascii", "us-ascii", "iso-ir-6",
"ansi_x3.4-1968" all map to `windows-1252` per the web-compatibility spec), and
multi-byte state machines.

`beman.transcode` implements the WHATWG algorithms and provides both:

- **Compile-time dispatch** via `codec::utf_8`, `codec::shift_jis`, etc. — enum
  constants used as non-type template parameters for zero-overhead codec
  selection
- **Runtime label resolution** via `get_encoding("utf-8")` and
  `transcode_string(data, "shift_jis", "utf-8")` — for cases where the encoding
  name comes from user input, HTTP headers, or HTML meta tags

A future integration point: `std::text_encoding::id()` could map directly to the
`beman::transcoding::codec` enum, providing the standard vocabulary type on top
of this library's implementation.

## Performance

Measured with GCC 16.1.1, `-O3 -march=native -mtune=native -flto=auto
-fno-semantic-interposition` (Intel 13th-gen Alderlake, AVX2 enabled).  Corpora
sourced from Wikipedia Mars articles in each language.

### UTF-8 Decode and Encode

| Benchmark | Corpus | Mean | Throughput |
|-----------|--------|------|-----------|
| UTF-8 decode: English (ASCII-heavy) | 586 B | 234 ns | 2.4 GiB/s |
| UTF-8 decode: Arabic (multibyte-heavy) | 640 B | 1025 ns | 596 MiB/s |
| UTF-8 encode round-trip: English | 586 B | 726 ns | 770 MiB/s |

### WHATWG Legacy Codecs

| Benchmark | Corpus | Mean | Throughput |
|-----------|--------|------|-----------|
| Shift-JIS decode: Japanese | 340 B | 261 ns | 1.2 GiB/s |
| UTF-8 decode: Japanese (3-byte heavy) | 504 B | 423 ns | 1.1 GiB/s |
| GB18030 decode: Chinese | 98 B | 94 ns | 993 MiB/s |
| Big5 decode: Traditional Chinese | 98 B | 111 ns | 842 MiB/s |
| EUC-JP decode: Japanese | 93 B | 47 ns | 1.9 GiB/s |
| EUC-KR decode: Korean | 108 B | 103 ns | 1.0 GiB/s |
| ISO-2022-JP decode: Japanese (stateful) | 111 B | 131 ns | 808 MiB/s |
| UTF-16LE decode | 936 B | 218 ns | 4.1 GiB/s |
| UTF-16BE decode | 936 B | 226 ns | 3.9 GiB/s |

### iconv Comparison

| Benchmark | Corpus | Mean | Throughput |
|-----------|--------|------|-----------|
| Raw `iconv()`: UTF-8 → UTF-32LE | 586 B | 821 ns | 680 MiB/s |
| `iconv_transcode_view`: UTF-8 → UTF-32LE | 586 B | 13.5 µs | 41 MiB/s |
| Raw `iconv()`: Shift-JIS → UTF-8 | 340 B | 853 ns | 380 MiB/s |

The `iconv_transcode_view` adds per-byte iteration overhead vs the block-oriented
raw `iconv()` API.  Its value is safety (no resource leaks, no buffer management)
and composability with other range adaptors.

### Reproducing

```bash
# Build GCC-16 optimized config and generate full report
make bench-perf-report

# Or run individual suites
make bench-perf       # smoke only
make bench-perf-all   # all core benchmarks

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
