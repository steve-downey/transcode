# Execution Plan: C++29 Transcoding Architecture (Beman Project)

## 1. Type Architecture & I/O Interfaces

To prevent strict-aliasing pessimizations and structurally ban array decay, the foundational types must be rigorously constrained before any pipelines are built.

* **The `byte_like` Output Concept:**
    To bypass compiler aliasing traps associated with `char` and `std::byte`, the output of our encoding pipelines will support custom enumeration-backed types (e.g., `enum class octet : unsigned char {}`).
    * *Implementation:* `detail::byte_like<T>` constrains `T` to exactly `char`, `signed char`, `unsigned char`, `std::byte`, or an `enum` mathematically backed by one of these four core types.
* **The `unicode_scalar_range` Input Constraint:**
    WHATWG encoding strictly requires resolved Unicode code points. Passing `char8_t` or `char16_t` will fail at compile time.
    * *Implementation:* Requires an `std::ranges::input_range` whose `range_value_t` is strictly `char32_t`.
* **Array Decay Ban:**
    All byte ingestion pipelines will `static_assert` against taking `const char[N]` directly. Users must wrap C-style string literals via C++26's `std::views::null_term` (P3705R2) to prevent accidental null-byte transposition.

## 2. WHATWG Codecs: Symmetric Pipelines & Concrete Roster

The WHATWG pipeline models strict adherence to the web platform, reifying the `char32_t` scalar intermediate between bytes and encoded structures.

### The Interfaces
* **Decoder:** `whatwg_decode_view<Encoding, InByte = char>` (Consumes `byte_like`, yields `char32_t`).
* **Encoder:** `whatwg_encode_view<Encoding, OutByte = char>` (Consumes `char32_t`, yields `byte_like`).
* **Error Handling:** Both views feature `_or_error` variants that return `std::expected` types rather than web-standard `U+FFFD` / Numeric Character Reference (NCR) substitution.

### The Codec Roster & Algorithm/Table Sources
The WHATWG Encoding Standard is closed; no new encodings will be added. All data is officially sourced from the machine-readable `https://encoding.spec.whatwg.org/indexes.json`.

1.  **Category A: The Algorithmic UTF Family**
    * *Names:* `utf_8`, `utf_16le`, `utf_16be`.
    * *Implementation Details:* Requires no external tables. Implemented purely via bit-shifting and continuation-byte logic.
2.  **Category B: Single-Byte Legacy (Flat Arrays)**
    * *Names:* `ibm866`, `iso_8859_2` through `iso_8859_16`, `koi8_r`, `koi8_u`, `macintosh`, `windows_874`, `windows_1251` through `windows_1258`, `x_mac_cyrillic`.
    * *Implementation Details:* A Python build script (`scripts/generate_whatwg_tables.py`) will parse `indexes.json` during the CMake build step to generate `constexpr std::array<char32_t, 128>` for the upper block (0x80–0xFF) of each encoding. Provides `O(1)` performance.
3.  **Category C: Multi-Byte CJK (Complex Indexing)**
    * *Names:* `big5`, `euc_jp`, `euc_kr`, `gb18030`, `iso_2022_jp`, `shift_jis`.
    * *Implementation Details:* Highly stateful (especially `iso_2022_jp`). The build script will parse `indexes.json` into multi-level tries (segmented lookup arrays) in `constexpr` headers to prevent binary bloat while maintaining fast pathing.
4.  **Special Categories:**
    * `replacement`: Unconditionally consumes all input and yields `U+FFFD`.
    * `x-user-defined`: Maps 0x80–0xFF strictly to the Private Use Area `U+F780`–`U+F7FF`.

## 3. POSIX `iconv`: Direct Legacy-to-Legacy Abstraction

The `iconv` pipeline acts as a zero-cost abstraction over the native OS C library. Unlike WHATWG, `iconv` natively manages the translation matrix between legacy encodings.

### The Interface
* **Transcoder:** `iconv_transcode_view<InByte = char, OutByte = char>` (Consumes `byte_like`, yields `byte_like`).
* **Key Distinction:** This view completely bypasses the `char32_t` constraint. It allows a user to pipe `shift_jis` bytes directly into `utf_8` bytes in a single pass without reifying the intermediate scalar in C++, preventing memory bottlenecking and unlocking maximum pipeline throughput.
* **Constraint Safety:** Models a move-only iterator to safely handle the lifetime of the `iconv_t` POSIX system handle. Operates over a user-injected `std::span<OutByte>` to eliminate heap allocations.

## 4. Validation Architecture

To ensure implementation immutability, safety, and correct licensing, testing is bifurcated.

### A. In-Tree WHATWG Conformance (W3C Web Platform Tests)
* **Data Source:** The `encoding/` directory from the `web-platform-tests/wpt` GitHub repository.
* **Execution:** A build script will extract the JSON-based WPT vectors (which the W3C provides under permissive licenses) and serialize them into C++ `constexpr` arrays.
* **Result:** `tests/beman/transcoding/whatwg.test.cpp` runs these arrays to prove 100% bug-for-bug compatibility with major browsers (including stateful BOM sniffing and `gb18030` fallbacks).

### B. Out-of-Tree Iconv Conformance ("Clean Room" Testing)
* **Data Source:** Edge-case test buffers generated from FreeBSD's Citrus tests, `musl libc`, and fuzzed JSON outputs from GNU `libiconv`.
* **Architecture:** Stored in a separate repository (`bemanproject/iconv-conformance-suite`).
* **Execution:** A data-driven C++ test harness parses JSON vectors to test `E2BIG` (buffer exhaustion), `EINVAL` (incomplete sequences), and `EILSEQ` (invalid sequences).
* **Result:** Standard Library maintainers (like MSVC and LLVM) can run this harness against their `iconv_transcode_view` to prove state-machine resilience without their engineers ever reading potentially GPL-tainted GNU test data.

## 5. Next Steps & Standardization

1.  **Repository Setup:** Stand up the `transcoding` directory using Beman `stamp.sh`.
2.  **Code Generation Script:** Write the Python script to fetch `indexes.json` and the WPT files to output the `.inc` C++ headers.
3.  **Core Views Implementation:** Write the underlying iterators, ensuring `iconv` is mockable via dependency injection for CI/CD boundary testing.
4.  **Proposal Drafting:** Draft `papers/PXXXX.md` targeting SG16 (Text) and SG9 (Ranges). Emphasize the "Aliasing Rationale" (using `octet` output types), the explicit array decay ban, and the robust out-of-tree testing architecture as primary selling points for the C++29 inclusion.
