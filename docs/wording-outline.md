<!-- markdownlint-disable MD013 -->

# Wording outline

The clause tree the paper's wording is generated into, and the map from it back
to the headers.  This is the contract the markup steps (`docs/plans/p5-step4-*`
through `p5-step9-*`) follow: a stable name, a title, a depth and an
expos/omit decision are settled here once, because each of them is
simultaneously a clause identifier, a fragment filename, an `{- .sref}` span and
an ordering key.  Changing one later renames a file, a heading and a
cross-reference at once.

See `docs/plans/phase5-index.md` for the plan, its decisions D1-D7, and the
upstream specgen items U1-U9 referenced below, whose state was re-measured
on 2026-09-04 (see the index).

## How the tree maps onto fragments

`--split` writes one fragment per **top-level** section, so fragment
granularity is decided by which `\rSec` markers are shallowest in a header, not
by the clause numbering the paper prints.  Two consequences:

- Every clause that needs to be a fragment of its own is a `\rSec2`.  A
  `\rSec3` under it rides along inside that fragment.  This is what lets Step 10
  put an authored editing instruction between two clauses.
- The umbrella clause `[transcode]` is **not** generated.  A `\rSec1` wrapping
  everything would collapse the whole header into a single fragment, and a
  `\rSec` section with no declarations under it renders as a bare heading
  (observed in Step 1).  The umbrella, and `[transcode.general]`, are authored
  in the paper around the generated clauses.

Heading level follows from mpark's fixed `base_heading_level = 2` (index U2):
`\rSec2` prints as `##`, `\rSec3` as `###`.

## `<transcode>`

Generated from `transcode.hpp`, which is the umbrella of `#include`s it always
was: specgen follows the headers included inside its `\rSec2[transcode.syn]`
region (index D1), so the component headers are the document without being one
file.  Fragment order is the order of the `\rSec` markers in `transcode.hpp`,
which is the order below; the *synopsis* reads in include order, which is the
order the compiler needs and need not match.

| Stable name | Depth | Title | Entities | Generated from |
|---|---|---|---|---|
| `transcode.general` | — | General | scope, terms, the byte/scalar model | authored in the paper |
| `transcode.syn` | 2 | Header `<transcode>` synopsis | everything below, gathered | `transcode.hpp`'s region and the sixteen headers included in it, done 2026-09-07 |
| `transcode.errors` | 2 | Error types | `whatwg_error`, `iconv_error`, `transcode_error_kind` | `error.hpp` routes to it, done 2026-09-06 |
| `transcode.reqs` | 2 | Range requirements | `legacy_byte_range`, `unicode_scalar_range` | `concepts.hpp` routes to it, done 2026-09-06 |
| `transcode.codec` | 2 | Encodings | `enum class codec` | `codec.hpp`, done 2026-09-07 |
| `transcode.codec.label` | 3 | Label lookup | `get_encoding` | `label.hpp`, done 2026-09-07 |
| `transcode.codec.sniff` | 3 | Byte order mark sniffing | `sniff_encoding` | `sniff.hpp`, done 2026-09-07 |
| `transcode.whatwg.decode` | 2 | Decoding views | `whatwg_decode_view`, its closures `whatwg_decode<C>` / `whatwg_decode_or_error<C>`, and its `enable_borrowed_range` specialization | `whatwg_decode_view.hpp`, done 2026-09-08 |
| `transcode.whatwg.encode` | 2 | Encoding views | `whatwg_encode_view`, likewise | `whatwg_encode_view.hpp`, done 2026-09-08 |
| `transcode.custom.reqs` | 2 | Codec requirements | `decode_codec`, `encode_codec`, `flushable_decode_codec`, `random_access_decode_codec_type`, `decode_result`, `encode_result` | `codec_concepts.hpp`, `codec_result.hpp`, done 2026-09-08 |
| `transcode.custom.decode` | 2 | Class template `decode_view` | `decode_view`, `decode`, `decode_or_error` | `decode_view.hpp`, done 2026-09-08 |
| `transcode.custom.encode` | 2 | Class template `encode_view` | `encode_view`, `encode`, `encode_or_error` | `encode_view.hpp`, done 2026-09-08 |
| `transcode.pipeline` | 2 | Transcoding pipelines | `transcode`, `pluggable_transcode` | `transcode_view.hpp`, done 2026-09-08 |
| `transcode.string` | 2 | Eager transcoding | `transcode_string`, both overloads | `transcode_string.hpp`, done 2026-09-08 |
| `transcode.iconv` | 2 | iconv adaptors | `iconv_functions`, the two views, the closures, `iconv_transcode`, `iconv_transcode_or_error`, `iconv_transcode_to`, `iconv_transcode_into`, `iconv_transcode_to_or_error` | the four `iconv_*.hpp` headers, done 2026-09-08 |
| `transcode.iconv.iterator` | 3 | The two iconv iterators | `iconv_transcode_view::$iterator$`, `iconv_transcode_or_error_view::$iterator$` | the same headers, done 2026-09-08 |

`transcode.custom.*` rather than `transcode.decode` / `transcode.encode` for the
codec-parameterized family: the WHATWG views are what most readers reach for, so
they get the short names, and grouping the extension point under one prefix says
what it is for.  It also keeps `transcode.codec.*` (the enumeration and its
lookups) from colliding with the codec *requirements*, which are a different
subject entirely.

## `<null_term>`

| Stable name | Depth | Title | Entities | Generated from |
|---|---|---|---|---|
| `null.term.syn` | root | Header `<null_term>` synopsis | `null_sentinel_t`, `null_sentinel`, `null_term_view`, `views::null_term` | gathered region, done 2026-09-06 |
| `null.term.sentinel` | 2 | Class `null_sentinel_t` | the hidden friend `operator==` | done 2026-09-06 |
| `null.term.view` | 2 | Class template `null_term_view` | the constructor, `begin`, `end` | done in Step 1 |
| `null.term.adaptor` | 2 | `views::null_term` | the range adaptor object | done 2026-09-06; `$unspecified$` in the synopsis, routed prose in the clause |

Step 1 generates `null.term` (the root, un-gathered) and `null.term.view`.
`null.term.sentinel` landed with the gathered region, and `null.term.adaptor`
in Step 4: its declaration is folded into the synopsis and a
`\ref{null.term.adaptor}` group header routes its prose to the clause, which is
what specgen#69 made possible.

## Exposition-only, omitted, and not proposed

The three ways a declaration stays out of the wording, decided per entity here
so no step has to decide twice.

### Exposition-only (`\expos`)

| Entity | Rendered as | Why |
|---|---|---|
| `null_term_view::ptr_` | `$ptr$` | the view's state, which the wording refers to |
| `detail::legacy_byte_type` | `$legacy-byte-type$` | a real conjunct of `legacy_byte_range`; the draft would spell it out.  Marked in Step 4 |
| every view's `base_`, `codec_`, `buf_`, iterator state | exposition names | the wording describes what they hold |
| a view's nested `iterator` | `class $iterator$;` | `\expos` with bare `\seebelow`, which renders the declaration and leaves the state out of the synopsis -- [range.transform.view]'s shape.  Needed specgen#80 |
| the closure types (`whatwg_decode_closure` and the seven others) | ideally `unspecified` | unblocked 2026-09-05; specgen#24 fixed |
| `detail::const_iterator_compatible_range`, `detail::const_sentinel_compatible_range` | `$const-iterator-compatible-range$`, `$const-sentinel-compatible-range$` | the const-compatibility chain the views constrain `begin`/`end` on |
| `detail::random_access_decode_codec`, `detail::random_access_encode_codec` | `$random-access-decode-codec$`, `$random-access-encode-codec$` | which codecs are O(1) per element; a property of the specification, not of the tables |
| `detail::whatwg_encode_codec`, `detail::whatwg_encode_input` | `$whatwg-encode-codec$`, `$whatwg-encode-input$` | WHATWG defines no encoder for some codecs; the constraint says which |

### Omitted (`\omit`)

| Entity | Why |
|---|---|
| `random_access_whatwg_decode_view` and the encode and pluggable equivalents, and the `enable_borrowed_range` specializations written for them | decision W1 below |
| the `_or_error_view` / `_or_error_closure` alias templates | transition spellings for the pre-unification names, not API |
| `null_term_view`'s deduction guide | the implicit guide from the constructor is identical; index U4 no longer applies |
| `detail::null_term_fn`, `detail::null_term_adaptor` | the adaptor object's type is unspecified |
| `detail::label_entry`, `label_table` | generated data behind `get_encoding`, and all `tools/generate_labels.py` emits since Step 5 |
| `detail::iconv_guard`, `iconv_input_buf` | RAII plumbing |
| `iconv_error_rc` | a POSIX return-value constant, not API |

### Not proposed at all

- `decode_to`, `encode_to`, `decode_into`, `encode_into`
  (`detail/bulk_transcode.hpp`).  The header says so itself, and the paper's API
  surface table agrees: its bulk rows are `v | ranges::to<>()` and
  `ranges::copy(v, out)`, which are the standard's own facilities.  Nothing to
  specify.
- ~~`make_real_iconv_fns`, and `iconv_functions` as an injection seam.~~
  Settled in Step 9: both are proposed.  `iconv_functions` is what makes the
  view testable without the platform's iconv tables
  (`tests/beman/transcode/iconv_mock.hpp`), and a seam that only the library's
  own tests can reach is a seam the library is keeping to itself.  It is
  specified as a named requirement on the template parameter, and
  `make_real_iconv_fns` as the function that supplies the platform's.

## The `detail::` audit (Step 3 task 3)

After Step 3, `specgen render --validate` reports **no** `detail` leakage in any
specification header.  Two chains got there differently, and the difference is
worth recording because Steps 6 and 7 inherit it.

- `detail::random_access_decode_codec` and `detail::random_access_encode_codec`
  appeared in the `random_access_*` views' requires-clauses.  W1 omits those
  views, and Step 3 moved the concepts themselves into
  `detail/whatwg_decode_select.hpp` and `detail/whatwg_encode_select.hpp`, out
  of the specification headers entirely.  Nothing left to decide.
- `detail::const_iterator_compatible_range` and
  `const_sentinel_compatible_range` constrain every `begin() const` and
  `end() const`, so they are genuinely part of what the specification says.
  They are **exposition-only in the wording**, but they cannot be rendered that
  way yet: `\expos` works on a concept (verified) and not on the alias template
  and class template the concept is defined in terms of (index U8).  They
  therefore stay in `detail/range_traits.hpp` for now, and **Step 6 chooses**
  between restating the concept in the specification header without helper
  aliases so a single `\expos` covers it, and waiting for U8.  Whichever it
  picks, the constraint is authored `\constraints` prose on the affected
  members rather than derived from the requires-clause.

## Decisions this outline settles

**W1 — the `random_access_` view specializations are not separate specified
entities.**  *Carried out in Step 6 (2026-09-08): both are `\omit`ted, and each
view's clause says instead that it models `random_access_range` when the codec
decodes one byte to one scalar value and the base range does.  That also
removed the last `detail::` qualifier from the document, which was reaching the
wording through those views' requires-clauses.*  They exist so a single-byte codec gets O(1) indexing.  That is a
property of the view, not a second view: the specification says
`whatwg_decode_view<C, R>` models `random_access_range` when `C` is a
single-byte codec and `R` models `random_access_range`, and an implementation
gets there however it likes.  Omitting them also removes
`detail::random_access_decode_codec` from every spec-visible signature, which
D7 requires anyway.  The alternative — specifying eight view templates instead
of four — doubles the clause for no reader benefit.  If SG16 wants the
distinction visible, it comes back as a *Remarks* paragraph, not as a type.

**W2 — the WHATWG enumeration is specified by reference.**  `enum class codec`
has 39 enumerators; the wording says each names the WHATWG encoding of the
corresponding name and cites the Encoding Standard, rather than restating a
table that is normative somewhere else and changes there.

*Step 5's answer to how the reference is spelled* (2026-09-07), in two parts,
because the question has two halves:

- **In the generated wording**, the standard is named in prose -- "the WHATWG
  Encoding Standard" -- and nothing else.  specgen's `\iref` points at a stable
  name in this draft and there is none to point at; a bibliography key is the
  paper's and would not survive into the working draft.
- **In the paper, and in the eventual working draft**, it is a normative
  reference in the [intro.refs] sense: an entry naming the Encoding Standard,
  its URL, and the version it was read at.  A living standard has no edition to
  cite, so the entry carries a date, and the paper says which snapshot
  `docs/whatwg/` was downloaded from.  Step 10 writes that entry.  Whether a
  normative reference to a living standard is acceptable at all is SG16's to
  answer, and it is a question about the paper's prose rather than something
  the wording can settle.

**W3 — `<null_term>` stays a separate header.**  It is the paper's position
already, and `views::null_term` has nothing to do with encodings; it is a range
adaptor over a C string that this proposal happens to need.

## Open, and deliberately not settled here

- ~~**The `_or_error` split.**~~  Settled in P5-Step 3c, before the clauses were
  written.  Each family is one view template parameterized on
  `transcode_error_kind`, so `transcode.whatwg.decode`,
  `transcode.whatwg.encode`, `transcode.custom.decode` and
  `transcode.custom.encode` each specify **one** class template and two closure
  objects rather than two of each.  `transcode.errors` gains
  `transcode_error_kind`, and its wording must not fix an underlying type for
  either enum (P2728 R13).
- ~~**Whether iconv is proposed at all**~~  Settled in Step 9: **iconv is
  proposed**, with full wording, and no feature-test-macro condition.  The
  argument for dropping it was that POSIX `iconv` is implementation-defined
  across glibc, musl and the BSDs, so specifying a view over it specifies
  "whatever the implementation's iconv does".  That is true and it is not
  disqualifying: the wording says what the *adaptor* guarantees -- one
  descriptor per `begin`, closed by the iterator that owns it, input the
  conversion refuses skipped or surfaced -- and leaves the conversion itself to
  the implementation, exactly as `<locale>` leaves the locale's own tables to
  it.  The leak-freedom is the whole point of proposing it: the C interface
  hands out a descriptor that a program must remember to close, and a range
  adaptor is where that stops being the program's problem.  `transcode.iconv`
  is still listed last, and it is still the one clause the paper can drop
  without renumbering anything else, which is what makes it a clean question to
  put to SG16 rather than one to settle silently.
- **Two error vocabularies.**  `transcode.iconv` reports `iconv_error` while
  every other clause reports `whatwg_error`, and Step 3c's unification does not
  merge them: it unified the *view* pairs on `transcode_error_kind`, which is
  orthogonal to what an error *is*.  They stay separate because the failures
  are not the same failures.  `whatwg_error` names what the Encoding Standard
  says went wrong in a byte sequence the library itself decoded;
  `iconv_error` names what POSIX reported -- `EILSEQ`, `EINVAL`, `E2BIG`, or a
  descriptor that would not open -- about a conversion the library did not
  perform.  A single enumeration would have to either drop the distinction or
  carry both sets, and the second is two vocabularies with one name.  See
  `docs/p2728-alignment.md`.
