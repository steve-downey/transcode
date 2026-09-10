## Acknowledgements

- Zach Laine for `Boost.Text` and extensive work on Unicode in C++.
- Tom Honermann and SG16 for guidance on text encoding issues.
- The WHATWG for the Encoding Standard [@whatwg-encoding].
- The Web Platform Tests project [@wpt-encoding] for comprehensive test vectors.

## Prior Art

This design did not arise in isolation. It draws on several earlier libraries and proposals that explored encoding-aware iteration, transcoding APIs, and implementation techniques.

Tom Honermann's `text_view` library and paper [@P0244R2; @text-view] established an early range-based model for enumerating encoded text as code points. It made the encoding explicit in the type system, and it ran into the ergonomics problem that follows from doing so: iteration has to surface decoding status somehow.

Zach Laine's `Boost.Text` [@boost-text] integrates range-based text facilities with C++ algorithms and pipelines, and its Unicode-oriented design and normalization support mark out territory that a proposal about external byte streams should stay out of.

JeanHeyd Meneide's `P1629R1` proposal and `ztd.text` implementation [@P1629R1; @ztd-text] go the other way: a broad text and transcoding library, eager and lazy interfaces both, encoding objects as values, extensible throughout. The surface here is narrower and fixed on browser-compatible WHATWG transcoding. But the split between lazy views and eager bulk helpers is the same admission that both usage styles are needed.

Henri Sivonen's `encoding_rs` [@encoding-rs] is the important implementation prior art for WHATWG-compatible transcoding specifically. It targets the Encoding Standard directly, provides streaming and bulk-oriented APIs both, and validates against the web-platform tests while still chasing fast paths. Exact WHATWG semantics, WPT-derived conformance coverage, and eager helpers alongside the range adaptors are all lessons taken from it.

The deliberate departure from `text_view`, Boost.Text, and `ztd.text` is
scope. Their general encoding abstractions can represent more models, but that
breadth makes the common boundary operation harder to name and specify. This
proposal fixes one compatibility model, WHATWG, and gives it direct decode,
encode, and collection names. The narrower surface is the point, not an
intermediate step toward another general text framework.

## Relationship to Other Proposals

### P2728 Unicode in the Library

[@P2728R13] proposes transcoding views for UTF-8, UTF-16, and UTF-32 that operate on the strict Unicode character types (`char8_t`, `char16_t`, `char32_t`). This proposal overlaps in the UTF portions but differs in scope and philosophy:

| Aspect | P2728 | This Proposal |
| -------- | ------- | --------------- |
| **Input types** | `char8_t`, `char16_t`, `char32_t` | `char`, `signed char`, `unsigned char`, `std::byte` |
| **UTF-16 model** | `char16_t` code units (native order) | Byte stream with explicit BE/LE |
| **Encodings** | UTF-8, UTF-16, UTF-32 only | 40 codec identifiers, including WHATWG special and decode-only entries |
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

When UTF-16 data arrives from external sources — network protocols, file formats, binary blobs — it arrives as bytes with a specific byte order, determined by the protocol or the BOM. The host's own endianness does not enter into it. The `codec::utf_16be` and `codec::utf_16le` variants decode these byte streams correctly either way.

The proposals are complementary:

- **P2728** is ideal when working with well-typed Unicode data within an application.
  Its strict typing catches encoding mismatches at compile time.

- **This proposal** is necessary when ingesting data from external sources — network protocols, file formats, databases — where bytes arrive as `char*` or `std::byte*`.
  The encoding is determined by metadata or heuristics that the C++ type system never sees.

A typical workflow might use this proposal to decode legacy-encoded input to `char32_t`, process it, then use P2728's facilities for UTF-to-UTF conversions within the application, and finally use this proposal again to encode output for a legacy system.

The UTF-8 decode/encode in this proposal follows WHATWG semantics (replacement character on error), which may differ slightly from P2728's error handling.
Applications requiring strict UTF validation should prefer P2728 for that portion of the pipeline.

### API Surface Comparison

The three implementation families in this proposal have matching API
surfaces for every operation their encoding model supports.  The P2728R13
column shows the parallel design in the proposed standard UTF transcoding views.

| API | WHATWG | Pluggable codec | iconv | P2728R13 |
|-----|--------|-----------------|-------|----------|
| **Codec identity** | `codec::utf_8` enum | `my_codec{}` type | `"UTF-8"` string | `char8_t`/`char16_t`/`char32_t` |
| **Decode view** | `whatwg_decode<C>` | `decode(codec)` | `iconv_transcode(f,t,buf)` | `views::to_utf32` |
| **Decode or-error view** | `whatwg_decode_or_error<C>` | `decode_or_error(codec)` | `iconv_transcode_or_error(…)` | `views::to_utf32_or_error` |
| **Encode view** | `whatwg_encode<C>` | `encode(codec)` | n/a ¹ | `views::to_utf8` / `to_utf16` |
| **Encode or-error view** | `whatwg_encode_or_error<C>` | `encode_or_error(codec)` | n/a ¹ | `views::to_utf8_or_error` |
| **Transcode pipeline** | `transcode<From,To>` | `pluggable_transcode(f,t)` | `iconv_transcode(f,t,buf)` | compose via `|` |
| **Bulk decode → container** | `decode_to<C>(r)` | `decode_to(codec,r)` | n/a ¹ | `v\|ranges::to<>()` |
| **Bulk encode → container** | `encode_to<C,Cont>(r)` | `encode_to(codec,r)` | n/a ¹ | `v\|ranges::to<>()` |
| **Bulk transcode → container** | n/a ² | n/a ² | `iconv_transcode_to(range,f,t)` | n/a ² |
| **Bulk decode → output iter** | `decode_into<C>(r,out)` | `decode_into(codec,r,out)` | n/a ¹ | `ranges::copy(v, out)` |
| **Bulk encode → output iter** | `encode_into<C>(r,out)` | `encode_into(codec,r,out)` | n/a ¹ | `ranges::copy(v, out)` |
| **Bulk transcode → output iter** | n/a ² | n/a ² | `iconv_transcode_into(range,f,t,out)` | n/a ² |
| **Null-terminated input** | `views::null_term(ptr)` | `views::null_term(ptr)` | `views::null_term(ptr)` | n/a ³ |
| **Runtime label lookup** | `get_encoding("utf-8")` | n/a ⁴ | n/a (string labels are the API) | n/a ⁵ |
| **Runtime transcode** | `transcode_string(src,from,to)` | n/a ⁴ | `iconv_transcode_to(r,f,t)` | n/a ⁵ |
| **BOM sniffing** | `sniff_encoding(range)` | n/a ⁶ | n/a ⁶ | n/a ⁷ |
| **Error type** | `whatwg_error` | `whatwg_error` | `iconv_error` | `utf_transcoding_error` |
| **Output element type** | `char32_t` | `char32_t` | `char` (raw bytes) | `char32_t` |
| **Input element type** | `char`/`byte` (legacy) | `char`/`byte` (legacy) | `char` (any byte encoding) | `char8_t`/`char16_t`/`char32_t` |
| **constexpr** | yes | yes | no (POSIX userland library) | yes |

**Notes on n/a entries:**

¹ **iconv is a byte↔byte transcoder.**  It converts one byte encoding directly
to another without exposing an intermediate `char32_t` stage.  Encode (from
`char32_t` to bytes) and decode (from bytes to `char32_t`) as separate steps
are outside its model.

² **WHATWG and pluggable codecs compose decode and encode.**  The named bulk
helpers forward to `view | ranges::to<Container>()` and
`ranges::copy(view, output)`. There is no separate bulk transcode helper:
transcode remains `decode | encode` composed with `|`. iconv performs
single-pass byte→byte conversion and exposes it as a first-class bulk operation.

³ **P2728 operates on typed Unicode character types** (`char8_t`, `char16_t`,
`char32_t`); `views::null_term` produces a range of `char`.  Bridging the two
requires a reinterpret step that is outside both proposals.

⁴ **Pluggable codecs are identified by C++ type.**  Codec
selection happens at compile time through the type system; there is no runtime
name-to-codec registry by design.  Runtime transcode is similarly outside
the model: you compose `decode(codec_a{}) | encode(codec_b{})` at compile time.

⁵ **P2728 selects its codec from the character type.**  Selection is
determined by the character types (`char8_t`, `char16_t`, `char32_t`), so
runtime label lookup and runtime transcode are outside its model.

⁶ **BOM sniffing is a property of the byte stream.**
`sniff_encoding()` examines the first bytes of a stream to detect UTF-8/16/32
BOMs and returns the appropriate `codec` enum value.  This is a WHATWG-specific
facility; pluggable codecs and iconv operate on already-identified encodings.

⁷ **P2728 operates on in-memory typed data** where the encoding is known from
the type.  BOM detection is an I/O concern; a separate endian-converting view
proposal addresses byte-order issues.

The shared design patterns across all four columns are the `_or_error` suffix
for error-surfacing variants, `char32_t` as the interchange type, and
composition with `|`.

## References

- WHATWG Encoding Standard [@whatwg-encoding]
- Web Platform Tests [@wpt-encoding]
- Beman.Transcode reference implementation: <https://github.com/bemanproject/transcode>

The framework resolves `N`/`P`-numbered citations from wg21.link on its own.
Everything else is a manual reference, which mpark/wg21 takes as a
`references` metadata block rather than a `bibliography` file.

---
references:
  - id: whatwg-encoding
    citation-label: WHATWG-Encoding
    type: webpage
    title: "Encoding Standard"
    author:
      - literal: "WHATWG"
    issued: { year: 2026 }
    URL: "https://encoding.spec.whatwg.org/"
    note: "Living Standard. Index data retrieved 2026-05-23."
  - id: wpt-encoding
    citation-label: WPT-Encoding
    type: webpage
    title: "Web Platform Tests --- Encoding"
    author:
      - literal: "W3C Web Platform Tests Contributors"
    issued: { year: 2026 }
    URL: "https://github.com/web-platform-tests/wpt/tree/master/encoding"
    note: "Test data retrieved 2026-05-24."
  - id: boost-text
    citation-label: Boost.Text
    type: webpage
    title: "Boost.Text"
    author:
      - family: Laine
        given: Zach
    issued: { year: 2018 }
    URL: "https://github.com/tzlaine/text"
    note: "What a C++ standard Unicode library might look like."
  - id: text-view
    citation-label: text_view
    type: webpage
    title: "text_view"
    author:
      - family: Honermann
        given: Tom
    issued: { year: 2017 }
    URL: "https://github.com/tahonermann/text_view"
    note: "Reference implementation for P0244."
  - id: ztd-text
    citation-label: ztd.text
    type: webpage
    title: "ztd.text"
    author:
      - family: Meneide
        given: JeanHeyd
    issued: { year: 2021 }
    URL: "https://github.com/soasis/text"
    note: "A text library for C++ with transcoding and encoding-aware text facilities."
  - id: encoding-rs
    citation-label: encoding_rs
    type: webpage
    title: "encoding_rs"
    author:
      - family: Sivonen
        given: Henri
    issued: { year: 2018 }
    URL: "https://github.com/hsivonen/encoding_rs"
    note: "A Gecko-oriented implementation of the WHATWG Encoding Standard in Rust."
  - id: schultke-quote
    citation-label: Schultke
    type: personal_communication
    title: "Personal correspondence"
    author:
      - family: Schultke
        given: Jan
    issued: { year: 2026 }
    note: "WG21 Mattermost server, quoted with permission."
---

::: {#refs}
:::
