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
upstream specgen items U1-U7 referenced below.

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

Generated from the spec-facing `<transcode>` header (Step 3 decides whether that
is one file or a family; index D1).  Fragment order is the order of the `\rSec`
markers in the source, which is the order below.

| Stable name | Depth | Title | Entities | Generated from |
|---|---|---|---|---|
| `transcode.general` | — | General | scope, terms, the byte/scalar model | authored in the paper |
| `transcode.syn` | root | Header `<transcode>` synopsis | everything below, gathered | the header's declaration region |
| `transcode.errors` | 2 | Error types | `whatwg_error`, `iconv_error` | `error.hpp` |
| `transcode.reqs` | 2 | Range requirements | `legacy_byte_range`, `unicode_scalar_range` | `concepts.hpp` |
| `transcode.codec` | 2 | Encodings | `enum class codec` | `whatwg_decode_view.hpp` |
| `transcode.codec.label` | 3 | Label lookup | `get_encoding` | `detail/labels.hpp` |
| `transcode.codec.sniff` | 3 | Byte order mark sniffing | `sniff_encoding` | `sniff.hpp` |
| `transcode.whatwg.decode` | 2 | Decoding views | `whatwg_decode_view`, `whatwg_decode_or_error_view`, their closures and `enable_borrowed_range` specializations | `whatwg_decode_view.hpp` |
| `transcode.whatwg.encode` | 2 | Encoding views | `whatwg_encode_view`, `whatwg_encode_or_error_view`, likewise | `whatwg_encode_view.hpp` |
| `transcode.custom.reqs` | 2 | Codec requirements | `decode_codec`, `encode_codec`, `flushable_decode_codec`, `random_access_decode_codec_type`, `decode_result`, `encode_result` | `codec_concepts.hpp`, `codec_result.hpp` |
| `transcode.custom.decode` | 2 | Class template `decode_view` | `decode_view`, `decode_or_error_view`, `decode`, `decode_or_error` | `decode_view.hpp` |
| `transcode.custom.encode` | 2 | Class template `encode_view` | `encode_view`, `encode_or_error_view`, `encode`, `encode_or_error` | `encode_view.hpp` |
| `transcode.pipeline` | 2 | Transcoding pipelines | `transcode`, `pluggable_transcode` | `transcode_view.hpp` |
| `transcode.string` | 2 | Eager transcoding | `transcode_string`, both overloads | `transcode_string.hpp` |
| `transcode.iconv` | 2 | iconv adaptors | `iconv_functions`, the two views, the closures, `iconv_transcode`, `iconv_transcode_or_error`, `iconv_transcode_to`, `iconv_transcode_into`, `iconv_transcode_to_or_error` | the four `iconv_*.hpp` headers |

`transcode.custom.*` rather than `transcode.decode` / `transcode.encode` for the
codec-parameterized family: the WHATWG views are what most readers reach for, so
they get the short names, and grouping the extension point under one prefix says
what it is for.  It also keeps `transcode.codec.*` (the enumeration and its
lookups) from colliding with the codec *requirements*, which are a different
subject entirely.

## `<null_term>`

| Stable name | Depth | Title | Entities | Generated from |
|---|---|---|---|---|
| `null.term.syn` | root | Header `<null_term>` synopsis | `null_sentinel_t`, `null_sentinel`, `null_term_view`, `views::null_term` | `null_term.hpp` |
| `null.term.sentinel` | 2 | Class `null_sentinel_t` | the hidden friend `operator==` | blocked on index U5 |
| `null.term.view` | 2 | Class template `null_term_view` | the constructor, `begin`, `end` | done in Step 1 |
| `null.term.adaptor` | 2 | `views::null_term` | the range adaptor object | blocked on index U7 |

Step 1 generates `null.term` (the root, un-gathered) and `null.term.view`.  The
other two are what Step 4 adds once their upstream items land.

## Exposition-only, omitted, and not proposed

The three ways a declaration stays out of the wording, decided per entity here
so no step has to decide twice.

### Exposition-only (`\expos`)

| Entity | Rendered as | Why |
|---|---|---|
| `null_term_view::ptr_` | `$ptr$` | the view's state, which the wording refers to |
| `detail::legacy_byte_type` | `$legacy-byte-type$` | a real conjunct of `legacy_byte_range`; the draft would spell it out |
| every view's `base_`, `codec_`, `buf_`, iterator state | exposition names | the wording describes what they hold |
| the closure types (`whatwg_decode_closure` and the seven others) | ideally `unspecified` | blocked on index U7; see below |

### Omitted (`\omit`)

| Entity | Why |
|---|---|
| `random_access_whatwg_decode_view`, `random_access_whatwg_decode_or_error_view`, and the encode and pluggable equivalents | decision W1 below |
| `null_term_view`'s deduction guide | the implicit guide from the constructor is identical; it also triggers index U4 |
| `detail::null_term_fn`, `detail::null_term_adaptor` | the adaptor object's type is unspecified |
| `detail::label_entry`, `label_table` | generated data behind `get_encoding` |
| `detail::iconv_guard`, `iconv_input_buf` | RAII plumbing |
| `iconv_error_rc` | a POSIX return-value constant, not API |

### Not proposed at all

- `decode_to`, `encode_to`, `decode_into`, `encode_into`
  (`detail/bulk_transcode.hpp`).  The header says so itself, and the paper's API
  surface table agrees: its bulk rows are `v | ranges::to<>()` and
  `ranges::copy(v, out)`, which are the standard's own facilities.  Nothing to
  specify.
- `make_real_iconv_fns`, and `iconv_functions` as an injection seam.  The
  injection exists so the tests can run without the platform's iconv tables
  (`tests/beman/transcode/iconv_mock.hpp`); whether it is API at all is part of
  Step 9's scope question.

## Decisions this outline settles

**W1 — the `random_access_` view specializations are not separate specified
entities.**  They exist so a single-byte codec gets O(1) indexing.  That is a
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
table that is normative somewhere else and changes there.  How a normative
reference to a living standard is spelled is a question for SG16, and Step 5
records the answer.

**W3 — `<null_term>` stays a separate header.**  It is the paper's position
already, and `views::null_term` has nothing to do with encodings; it is a range
adaptor over a C string that this proposal happens to need.

## Open, and deliberately not settled here

- **The `_or_error` split.**  `docs/p2728-alignment.md` has an open action item
  to unify each view with its `_or_error` twin under one error-kind-parameterized
  template, and to give the error enums an unspecified underlying type.  That
  would halve `transcode.whatwg.decode`, `transcode.whatwg.encode`,
  `transcode.custom.decode` and `transcode.custom.encode`.  Steps 6 and 7 must
  check that item before writing those clauses; writing wording for a shape that
  is about to change is the one avoidable waste in this phase.
- **Whether iconv is proposed at all** — Step 9's scope question, which is why
  `transcode.iconv` is listed last and is the only clause the paper can drop
  without renumbering anything else.
