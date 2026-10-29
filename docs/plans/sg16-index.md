<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# SG16 review response: living design doc

This is the durable, cross-step record for the SG16 review response to P4246
("Transcoding Text Views"), revision D4246R1. This committed file, on branch
`sg16-review-response`, is the only copy. Steps update it in place, in their own
worktrees, and the merge carries the change forward.

Read it **by anchor only**: open the section your step file names. Do not read
it front to back.

Sections marked **Provisional** record what was actually needed when the
decision was made, and what would justify revisiting it. If a later step finds
one of them inadequate, that is an amendment (see `AGENT-PROMPT.md`). It is not
a reason to build a parallel version.

---

## Decisions the user made {#user-decisions}

Do not relitigate these. They come from the user's brief for this run.

1. The WHATWG surface moves into a nested `whatwg` namespace (table below).
   The paper flags this as a revisitable choice about how many names `std::`
   takes.
2. `transcode_error_kind::{replacement, expected}` becomes
   `error_mode::{replacement, report}`, outside `whatwg::`.
3. The library keeps its own enum and does not adopt `std::text_encoding::id`.
   It bridges both ways instead (see #text-encoding-bridge).
4. Close the gaps in the pluggable layer: `flushable_text_encoder`, codec
   objects for the WHATWG encodings with the enum adaptors specified in terms of
   them, and `enable_borrowed_range` for `decode_view` and `encode_view`.
5. The paper revision covers the renames, a "Why not text_encoding::id"
   section, the three-layer framing, the dispatch numbers, and Jonathan Wakely's
   wording defects.
6. The rendered paper is published on gh-pages. That work is a separate branch
   off `main` and is not merged into the integration branch.
7. *(Amendment before execution.)* The WHATWG spec says "Each encoding has an
   associated decoder and most of them have an associated encoder." So the
   1:1 WHATWG mapping belongs on the **codec objects**:
   `whatwg::decoder<C>` and `whatwg::encoder<C>`. It does not belong on the
   concepts. The concepts describe the generic pluggable protocol and live at
   the top level: `text_decoder`, `text_encoder`,
   `flushable_text_decoder`, `flushable_text_encoder`,
   `single_byte_text_decoder`, `single_byte_text_encoder`. Verb and noun:
   `whatwg::decode<C>` is `decode(whatwg::decoder<C>{})`, and
   `whatwg::transcode<F, T>` is
   `transcode(whatwg::decoder<F>{}, whatwg::encoder<T>{})`. There is **no**
   `transcoder<F, T>` type.

## Namespace and directory layout {#namespace-layout}

The project rule is that namespace nesting mirrors directory depth. The nested
namespace `beman::transcoding::whatwg` therefore lives in
`include/beman/transcode/whatwg/`. The top level is already `transcode/` mapping
to `transcoding::`, and that mismatch is tolerated.

- Spec-facing `whatwg::` headers go in `include/beman/transcode/whatwg/*.hpp`.
  Their include guards are `INCLUDE_BEMAN_TRANSCODE_WHATWG_<NAME>_HPP`.
- Implementation stays in `include/beman/transcode/detail/`, in namespace
  `beman::transcoding::detail`, even where it serves only `whatwg::`. That
  includes the codec engines, the select headers, and the generated
  `detail/labels.hpp`. Moving detail code would be churn with no reader
  benefit.
- Generic plumbing stays at the top level, in `beman::transcoding`:
  `null_term`, `legacy_byte_range`, `unicode_scalar_range`, `iconv_*`,
  `error_mode`, and the codec-object layer: the codec concepts
  (`text_decoder`, `text_encoder` and their refinements, in
  `codec_concepts.hpp`), the codec results (`decode_result`,
  `encode_result`, in `codec_result.hpp`), `decode_view`, `encode_view`,
  `decode(d)`, `encode(e)`, `transcode(from, to)`, and the codec-object bulk
  helpers. The concepts describe the generic pluggable protocol, not WHATWG,
  so they sit beside the adaptors they constrain.
- Old header paths are removed, not forwarded. This is a pre-standard
  incubator and the stacked PRs rebase anyway (see #follow-ups).

Target header map, after steps 05 and 06:

| Old header | New header | Contents |
|---|---|---|
| `codec.hpp` | `whatwg/encoding.hpp` | `whatwg::encoding` |
| `error.hpp` (part) | `whatwg/error.hpp` | `whatwg::error` |
| `error.hpp` | `error.hpp` | `iconv_error`, `error_mode` |
| `codec_result.hpp` | `codec_result.hpp` (stays at top level) | `decode_result`, `encode_result` (names unchanged; they now carry `whatwg::error`) |
| `codec_concepts.hpp` | `codec_concepts.hpp` (stays at top level) | `text_decoder`, `text_encoder`, `flushable_text_decoder`, `single_byte_text_decoder` (+ `flushable_text_encoder` in 07, `single_byte_text_encoder` in 11) |
| `label.hpp` | `whatwg/label.hpp` | `whatwg::get_encoding` |
| `sniff.hpp` | `whatwg/bom_sniff.hpp` | `whatwg::bom_sniff` |
| `whatwg_decode_view.hpp` | `whatwg/decode_view.hpp` | `whatwg::decode`, `whatwg::decode_or_error` |
| `whatwg_encode_view.hpp` | `whatwg/encode_view.hpp` | `whatwg::encode`, `whatwg::encode_or_error` |
| `transcode_view.hpp` (part) | `whatwg/transcode_view.hpp` | `whatwg::transcode<From, To>` |
| `transcode_view.hpp` | `transcode_view.hpp` | `transcode(from, to)` (was `pluggable_transcode`) |
| `transcode_string.hpp` | `whatwg/transcode_string.hpp` | `whatwg::transcode_string`, `whatwg::has_encoder` |
| `bulk_transcode.hpp` (part) | `whatwg/bulk.hpp` | `whatwg::decode_to<C>` and friends |
| `bulk_transcode.hpp` | `bulk_transcode.hpp` | `decode_to(d, r)` and friends (codec objects) |
| (new, 08) | `whatwg/decoder.hpp` | `whatwg::decoder<C>` |
| (new, 10) | `whatwg/encoder.hpp` | `whatwg::encoder<C>` |
| (new, 14) | `whatwg/text_encoding.hpp` | `std::text_encoding::id` bridge |

Every header added or moved must also be updated in three places:
`include/beman/transcode/CMakeLists.txt` (`BEMAN_TRANSCODE_HEADERS`, which
`tools/tests/test_installed_headers.py` enforces), the synopsis region of
`transcode.hpp` if the header is spec-facing, and `docs/wording-outline.md`.

## Rename table {#rename-table}

This is the single table every step uses. Template parameter names (`C` for
an encoding, `E` for the error mode, `Codec` for a codec object) do not change.

| Old | New | Step |
|---|---|---|
| `transcode_error_kind` | `error_mode` | 04 |
| `transcode_error_kind::replacement` | `error_mode::replacement` | 04 |
| `transcode_error_kind::expected` | `error_mode::report` | 04 |
| `codec` (enum class) | `whatwg::encoding` (enumerators unchanged) | 05 |
| `whatwg_error` | `whatwg::error` | 05 |
| `decode_result` / `encode_result` | unchanged, top level (the codec protocol is generic); the `error` member is typed `whatwg::error` | 05 |
| `decode_codec` | `text_decoder` | 05 |
| `encode_codec` | `text_encoder` | 05 |
| `flushable_decode_codec` | `flushable_text_decoder` | 05 |
| `random_access_decode_codec_type` | `single_byte_text_decoder` | 05 |
| `get_encoding` | `whatwg::get_encoding` | 05 |
| `sniff_encoding` | `whatwg::bom_sniff` | 05 |
| `whatwg_decode<C>` / `whatwg_decode_or_error<C>` | `whatwg::decode<C>` / `whatwg::decode_or_error<C>` | 06 |
| `whatwg_encode<C>` / `whatwg_encode_or_error<C>` | `whatwg::encode<C>` / `whatwg::encode_or_error<C>` | 06 |
| `whatwg_decode_view` / `random_access_whatwg_decode_view` / `whatwg_decode_closure` | `whatwg::decode_view` / `whatwg::random_access_decode_view` / `whatwg::decode_closure`. Interim only: deleted in 09 | 06 |
| `whatwg_encode_view` / `random_access_whatwg_encode_view` / `whatwg_encode_closure` | `whatwg::encode_view` / ... Interim only: deleted in 12 | 06 |
| `*_or_error_view`, `*_or_error_closure` compatibility aliases (WHATWG and pluggable) | deleted. Spell the view with `error_mode::report` instead | 06 |
| `transcode<From, To>` / `transcode_closure` | `whatwg::transcode<From, To>` / `whatwg::transcode_closure` | 06 |
| `pluggable_transcode(from, to)` | `transcode(from, to)`. Closure type name `pluggable_transcode_closure` unchanged (`\omit`) | 06 |
| `transcode_string(...)` (both overloads) | `whatwg::transcode_string(...)` | 06 |
| `has_whatwg_encoder(codec)` | `whatwg::has_encoder(whatwg::encoding)` | 06 |
| `decode_to<C>`, `encode_to<C, Container>`, `decode_into<C>`, `encode_into<C>` | `whatwg::decode_to<C>` and so on | 06 |
| `decode_view`, `encode_view`, `random_access_decode_view`, `decode(codec)`, `decode_or_error(codec)`, `encode(codec)`, `encode_or_error(codec)`, `decode_to(codec, r)` and friends | unchanged, in `beman::transcoding` | — |
| (new) | `flushable_text_encoder` | 07 |
| (new) | `whatwg::decoder<C>` (class template per encoding; models `text_decoder`) | 08 |
| (new) | `whatwg::encoder<C>` (constrained class template; exists only where `whatwg::has_encoder(C)`; models `text_encoder`) | 10 |
| (new) | `single_byte_text_encoder`, `random_access_encode_view` | 11 |
| (new) | `whatwg::get_encoding(std::text_encoding::id)`, `whatwg::to_text_encoding_id(whatwg::encoding)` | 14 |

Docs prose follows the table. Write "report mode" for `error_mode::report`, and
"the `whatwg::encoding` enumeration" where the old text said "the codec
enumeration".

## Name collision between the two layers {#collision-resolution}

**Provisional.** The user asked for this to be resolved explicitly.

Enum-keyed adaptors are **variable templates in `whatwg::`**
(`whatwg::decode<C>`, `whatwg::encode<C>`, `whatwg::transcode<From, To>`).
Codec-object adaptors are **function templates in `beman::transcoding`**
(`decode(d)`, `encode(e)`, `transcode(from, to)`).

- They cannot share a namespace. A variable template and a function template
  cannot have the same name in one scope, and that is why
  `pluggable_transcode` becomes `transcode(from, to)` one level up rather than
  an overload beside `whatwg::transcode<From, To>`.
- Inside `namespace whatwg`, an unqualified `decode`, `encode` or `transcode`
  finds the `whatwg::` variable template. Implementation code in `whatwg::`
  that means the codec-object adaptor writes `transcoding::decode(...)`.
- A program that applies `using namespace` to both namespaces gets an
  ambiguity on unqualified `decode`. Tests and examples use
  `using namespace beman::transcoding;` and spell `whatwg::decode<...>`
  qualified. Never use-directive `whatwg`.
- The paper reports this under the namespace/landgrab discussion (step 18).
  Revisit if SG16 or LEWG prefers distinct names, such as `decode_with(d)`.

## Error mode {#error-mode}

`enum class error_mode { replacement, report };` lives in `error.hpp`, at the
top level.

- In `replacement` mode, a decoding error yields U+FFFD, ill-formed UTF-32 is
  replaced with U+FFFD before encoding, and an unencodable scalar yields `'?'`
  or the encoder's replacement bytes (see #encode-result-contract).
- In `report` mode, the value type is `expected<T, whatwg::error>`.

## Codec contract: decode_result {#decode-result-contract}

**Done, step 08.** `decode_result` gains a trailing, defaulted
`bool is_empty{false}`. It says the decoder consumed input but produced no
scalar value. The WHATWG cases are an ISO-2022-JP escape sequence at the end of
the input, and a UTF byte order mark with nothing after it (returned by
`decode_first`, see #bom-aware-decoder). `decode_view::iterator::load()` loops: at the end of the input it flushes (an
engaged result with `is_empty` ends the range like `nullopt`), otherwise it
calls `decode_one` and goes round again on an `is_empty` result. Existing three-member
brace-initialisation keeps compiling. Revisit if a consumer needs to
distinguish "nothing yet, need more input" from "nothing, by design".

`flush()` is called when the input is exhausted. It is called again after each
engaged result, until it returns `nullopt`. That is what `decode_view` has
always done, and step 07 corrects the docblock that said "called once".

## Codec contract: encode_result {#encode-result-contract}

**Provisional, step 10.** When `is_error` is true and `count > 0`,
`bytes[0..count)` is the encoder's own replacement for the unencodable scalar,
and replacement mode yields those bytes instead of `'?'`. ISO-2022-JP needs
this, because its replacement carries the escape back to ASCII. A result with
`is_error` true and `count == 0` still means `'?'`, so existing custom encoders
are unaffected. Report mode yields one `unexpected` element either way.

## flushable_text_encoder {#flushable-encoder}

**Done, step 07.** This mirrors `flushable_text_decoder`:

```cpp
template <typename C>
concept flushable_text_encoder = text_encoder<C> && requires(C& c) {
    { c.flush() } -> std::same_as<std::optional<encode_result>>;
};
```

`encode_view` calls `flush()` at the end of input, emits its bytes, and calls
it again until it returns `nullopt`. The first real consumer is
`whatwg::encoder<encoding::iso_2022_jp>` in step 10. Until then, the only
consumer is the toy `shift_state_encoder` in `tests/beman/transcode/encode_view.test.cpp`.
An engaged result with `count == 0` and no error is treated as "owes nothing"
(the view finishes), so a buggy codec cannot loop forever. Both `flush()`
docblocks (decoder and encoder) now say "called until `nullopt`".

## bom_aware_text_decoder {#bom-aware-decoder}

**Provisional, step 09 (amendment 09, name approved).**

```cpp
template <typename C>
concept bom_aware_text_decoder =
    text_decoder<C> && requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
        { c.decode_first(iter, sentinel) } -> std::same_as<decode_result>;
    };
```

`decode_view::iterator`'s constructor (`load_first()`, exposition-only) calls
`decode_first` instead of `decode_one` for the first element of a non-empty
input, once, then behaves as `operator++` does. It is the start-of-input twin of
`flushable_text_decoder`'s `flush()` at the end. It exists so that once-only
behaviour (the UTF byte order mark) stays out of the per-element path: the
generic view then decodes as fast as the deleted hand-specialized WHATWG views
did. Revisit if SG16 or LEWG would rather the BOM rule live in the adaptor
(`whatwg::decode<C>` as BOM-strip plus `decode(decoder)`, which would break
"`whatwg::decode<C>` is `decode(whatwg::decoder<C>{})`").

## Pluggable view invariants {#pluggable-view-invariants}

- **Hot-path rule (amendment 09).** The per-element path of a codec-object view
  (`load()` and the codec's `decode_one`/`encode_one`) carries no once-only
  test: start-of-input behaviour goes in an optional hook called once
  (`decode_first`), end-of-input behaviour in `flush()`. No compiler-specific
  attribute (`[[gnu::always_inline]]`, `__attribute__`) or macro goes on a
  declaration in a specification header: specgen renders attributes verbatim
  into the wording. Measured with `tmp/plan/ir-bench/ir-compare.sh`.

- The views and their iterators hold the codec as
  `[[no_unique_address]] Codec`, so a stateless codec costs no storage
  (steps 07 and 08).
- Iterator `operator==` also compares the codec (and, for decode, the loaded
  value) when `std::equality_comparable<Codec>`. Two iterators at the same
  byte position with different codec state are different positions
  (steps 07 and 08).
- `enable_borrowed_range` is `borrowed_range<R>` for `decode_view`,
  `random_access_decode_view`, `encode_view`, and, from step 11,
  `random_access_encode_view` (step 03). This is sound because each iterator
  holds its own copy of the codec.
- `decode(d)` selects `random_access_decode_view` when `d` models
  `single_byte_text_decoder` and the base is random-access. `encode(e)`
  selects `random_access_encode_view` when `e` models
  `single_byte_text_encoder` (step 11).

## Codec objects for the WHATWG encodings {#codec-objects}

**Decoder done in step 08; encoder provisional, step 10.**

- `whatwg::decoder<C>` (`whatwg/decoder.hpp`) is defined for every
  `whatwg::encoding`. It models `text_decoder`, and it models `single_byte_text_decoder`
  exactly when `detail::random_access_decode_codec<C>`. It models
  `flushable_text_decoder` exactly when the encoding keeps state between calls
  (UTF-16BE/LE, GBK/gb18030, Big5, ISO-2022-JP). It is `equality_comparable`.
  The UTF decoders (exposition-only concept `detail::strips_leading_bom<C>`,
  rendered `$strips-leading-bom$`) also model `bom_aware_text_decoder`:
  their `decode_first` drops one leading U+FEFF, and `decode_one` never does.
  There is no "at start" flag: step 08's `bom_` member and
  `detail::leading_bom_state_t` were removed in step 09 (amendment 09),
  because testing the flag on every element cost UTF-16 decode +21% (gcc-16)
  to +83% (clang-20) in instructions. `decode_one` is the per-encoding dispatch, and `flush()`
  runs it over an empty range.
- `whatwg::encoder<C>` (`whatwg/encoder.hpp`) exists only where
  `whatwg::has_encoder(C)`: it is a **constrained primary template**
  (`requires detail::whatwg_encode_codec<C>`). It is not a `static_assert`
  in the body, and it is not an absent specialization, so
  `requires { typename whatwg::encoder<C>; }` detects it, and naming it for a
  decode-only encoding is ill-formed (negative test, step 10). The friendly
  "WHATWG does not define an encoder" diagnostic, which the existing negative
  tests match, stays on the adaptors `whatwg::encode<C>` and
  `whatwg::transcode<F, T>`, through a `detail::` helper that
  `static_assert`s before naming `whatwg::encoder<C>` (steps 12 and 13). It
  models `text_encoder`, models `single_byte_text_encoder` for
  the single-byte encodings, and models `flushable_text_encoder` for ISO-2022-JP.
- Done in step 09 for decode (step 12 for encode): `whatwg::decode<C>` is
  `transcoding::decode(whatwg::decoder<C>{})`,
  `whatwg::decode_or_error<C>` is `transcoding::decode_or_error(whatwg::decoder<C>{})`,
  and the encode side is the same. The WHATWG view classes are gone. Range
  properties come from the codec-object views.
- After step 13, `whatwg::transcode<From, To>` is
  `transcoding::transcode(whatwg::decoder<From>{}, whatwg::encoder<To>{})`, and
  `transcode(from, to)` mixes library codec objects with program-defined ones. There is no
  `transcoder<F, T>` type (user decision).
- The sentence to aim for in the paper and wording: "`whatwg::decoder<C>`
  models `text_decoder`."

## single_byte_text_encoder {#single-byte-encoder}

**Provisional, step 11.** This is added only so that `whatwg::encode<C>` keeps
`random_access_range` for the single-byte encodings once it is defined in
terms of `encode(whatwg::encoder<C>{})`. It requires
`c.encode_byte(char32_t scalar) -> std::optional<unsigned char>`, where
`nullopt` means unencodable. Revisit, and possibly drop it along with
random-access encode, if SG16 or LEWG object to the extra public name.

## std::text_encoding bridge {#text-encoding-bridge}

**Provisional, step 14.**

- `constexpr std::optional<whatwg::encoding> whatwg::get_encoding(std::text_encoding::id id) noexcept;`
  applies the WHATWG label lookup to `std::text_encoding{id}.name()` and then
  to each of its `aliases()`, and returns the first hit. So `id::ISOLatin1`
  maps to `windows_1252`, because its aliases include `latin1` and
  `ISO-8859-1`. `id::other` and `id::unknown` map to `nullopt`.
- `constexpr std::optional<std::text_encoding::id> whatwg::to_text_encoding_id(whatwg::encoding e) noexcept;`
  tries each WHATWG label of `e` with `std::text_encoding`'s name lookup and
  returns the first registered id. `replacement` and `x_user_defined` map to
  `nullopt`. The user expects these to be the only two. Step 14 verifies that
  per encoding and records any other here, without inventing a mapping.
- Both are compiled only `#if defined(__cpp_lib_text_encoding)`. **GCC 16
  libstdc++ defines that macro only under `-std=c++26`, not `-std=c++23`**
  (verified 2026-10-28). The project builds as C++23, so a dedicated test
  target sets `CXX_STANDARD 26`, or the bridge has no coverage. specgen parses
  with `-std=c++2c`, so the bridge does appear in the wording.
- The example `examples/text_encoding_two_answers.cpp` (also C++26) decodes
  Shift_JIS bytes `5C 7E` through WHATWG and through iconv. Verified on this
  machine on 2026-10-28: glibc `SHIFT_JIS` gives U+00A5 U+203E, glibc `CP932`
  gives U+005C U+007E, and WHATWG `shift_jis` gives U+005C U+007E. It is an
  example and not a test, because the answer depends on the platform's iconv
  tables.

## Wording regeneration {#wording}

- The wording fragments in `papers/wording/*.md` are generated from the header
  markup. Never edit them by hand.
- Any step that changes a header in the document extent (the root
  `transcode.hpp` or `null_term.hpp`, or anything included inside the
  `[transcode.syn]` region) runs `make wording` and commits the result.
  `make wording-check` and `make wording-inputs-check` must pass.
- A specgen built from `papers/wording/specgen-ref` is on `PATH`
  (`~/.local/bin/specgen`). `make wording-check` passed at the base commit in
  about 6 seconds.
- Do not use `papers/wording/PENDING` to defer regeneration in this run.
- **specgen renders nested namespaces flat.** Verified 2026-10-28: a
  declaration in `beman::transcoding::whatwg` appears in the synopsis directly
  inside `namespace std {`, and `views::null_term` already renders as plain
  `null_term`. Fixing that is a specgen change and is out of scope (see
  #follow-ups). Step 15 adds an editor's note to the paper's Wording section
  saying which declarations are in `std::whatwg` (and `std::views`).
- specgen emits a class's own doc comment in the synopsis fragment when the
  class has no item declaration in a subclause. That is what produces the
  orphaned *Remarks* paragraphs. Step 15 fixes this through header markup.

## Paper scope {#paper-scope}

- The source of record is `papers/transcode-view.md` (mpark/wg21), plus the
  generated `papers/wording/*.md` and `papers/parts/tail.md`.
- The wg21org copy of the paper **no longer exists; there is nothing to keep
  in sync.** The user removed it on branch `remove-wg21org-paper` (off
  `origin/main`, not yet merged). That branch deletes
  `papers/wg21org/transcode-view.org`, `transcode-view.bib`,
  `papers/wg21org/wording/` and the transcode rules in
  `papers/wg21org/Makefile`, and rewrites the top of `papers/README.md`.
  `papers/wording/generate.sh` keeps `--backend org`. No step edits
  `papers/README.md` or `papers/wg21org/`, so if the orchestrator merges
  `main` into the integration branch after that branch lands, no conflict is
  expected. A conflict there means some step strayed outside its scope.
- Build: `make -C papers transcode-view.html transcode-view.pdf`. To avoid a
  download, seed the framework deps first:
  `cp -a /home/sdowney/src/steve-downey/transcode/p4246-r0/papers/wg21/deps/pandoc papers/wg21/deps/`
  and copy `csl.json`, `srefs.json` and `srefs.defs` from that worktree's
  `papers/wg21/data/`. The Python venv builds itself over the network. HTML
  took 47 s and PDF 9 s (xelatex) on 2026-10-28.
- The prose is Steve Downey's. Paper steps invoke the `voice` skill on every
  new or rewritten prose passage, and never on code, normative wording, or
  quotations.

## Dispatch numbers {#dispatch-numbers}

These were measured in PR #128 (branch `dispatch-overhead-benchmark`, not on
`main`). Step 18 uses them in "Template Parameter vs Runtime Selection".

- By-codec runtime dispatch costs about 121 instructions per call, regardless
  of encoding or input size. That figure is an upper bound, because it
  includes `transcode_string`'s wrapper.
- Each `get_encoding` label lookup costs about 550 instructions.
- Net effect: under 1% on whole corpora, and 3-5% on 32-byte inputs.

## Benchmark comparison for the codec-object switch {#perf-switch}

Steps 09 and 12 compare release-build benchmarks before and after the switch.

- Commands: `uv run cmake --preset gcc-release` creates
  `build/gcc-release/`, which is gitignored. Then build the targets
  `beman.transcode.benchmarks.whatwg`, `.utf` and `.pluggable`, and run them
  with the Catch2 tags `[benchmark][whatwg]`, `[benchmark][utf]` and
  `[benchmark][pluggable]`.
- Threshold (revised by amendment 09): the gate is **instruction counts** from
  `tmp/plan/ir-bench/ir-compare.sh` (callgrind, a value-consuming loop, the
  benchmark corpora), under gcc-16 `-O3 -march=native` (the Perf configuration
  the paper's numbers come from) and clang-20 `-O3`. A decode or encode row
  that regresses by more than 10% under either is a finding. The gcc-13 `-O2`
  column (the `gcc-release` preset's effective level; see #follow-ups) and the
  Catch2 wall-time tables are reported, not gated: wall time on these
  100-1000 ns benchmarks moved +-20% between identical binaries, and
  `count_elements` discards the values, so it rewards a view whose table
  lookups the optimizer can delete.
- Known residual at step 09: Big5 decode +9% to +16% under gcc-16 (about five
  instructions per character, a fully register-allocated loop; clang-20 +2%).
  Accepted in amendment 09 as code-layout variance, not abstraction cost.
- The before/after table goes in the commit message body. Steps record the
  base commit SHA of each comparison here:
  - step 09 base: 505359dc1fd9d032358588baf42ca3f209fe1ddc
  - step 12 base: (filled by step 12)

## Follow-ups (not steps) {#follow-ups}

Steps append one line each here when they discover something out of scope.
The integration review reads this section.

- Stacked PRs #126 <- #127 <- #128 (`label-lookup-benchmarks`,
  `label-table-string-view`, `dispatch-overhead-benchmark`) are left
  untouched. After this run they need rebasing onto the renamed surface:
  `benchmark/*.cpp` uses `codec::`, `transcode_string` and `get_encoding`.
  #127 rewrites `tools/generate_labels.py` and `detail/labels.hpp` to
  `string_view`, and that collides with step 05's generator change.
- `benchmark/benchmark_sink.hpp`'s `count_elements` never reads the elements.
  For a hand-specialized view GCC then deletes the table lookups, so a decode
  benchmark measures parsing (amendment 09: Shift_JIS, EUC-KR and UTF-8 looked
  +36% to +144% slower through the generic view in count mode and were 0% to
  +2% with the values used). Before step 18 quotes decode numbers, the user
  should decide whether the sink should consume values.
- The `gcc-release` preset is effectively `-O2`: `infra/cmake/gnu-toolchain.cmake`
  sets `-O3` in the `_INIT` flags and CMake appends its own `-O2 -g`, so the
  compile line is `-O3 -O2 -g`. The Perf configuration (`etc/gcc-flags.cmake`)
  is a real `-O3`. Under `-O2`, GCC 13 does not inline the codec-object chain
  as far, and UTF-16, Big5 and gb18030 decode are +23% to +78% in
  instructions after step 09. `infra/` is the Beman infrastructure subtree.
- The project `CLAUDE.md` "Architecture" section names the old headers and
  identifiers. Agents may not edit `CLAUDE.md`, so the user updates it after
  merge.
- Branch `remove-wg21org-paper` (which deletes the wg21org copy of the paper)
  is unmerged. It is independent of this series and touches only
  `papers/wg21org/` and `papers/README.md`, which no step edits. Merging and
  pushing it is the user's call.
- specgen does not render nested namespaces (`whatwg::`, `views::`) in a
  synopsis. That needs a specgen change.
- The uncommitted work in
  `/home/sdowney/src/steve-downey/transcode/paper-pluggable-runtime` was
  folded in by step 02. The user decides whether to discard that worktree.
