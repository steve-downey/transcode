<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Adopting the PR #131 review

[`review-findings-2026-10-30-pr131.md`](../review-findings-2026-10-30-pr131.md)
reviewed the SG16 review response (#131) at `f225333`, before it merged.
This plan takes each point the review raised, says where it stands against
`main` as of 2026-11-08, and orders what is left.

Each item has a slug for its identity.  The four findings the review gave
headings to keep the review's own anchors; the rest are named here.

## Resolved

| Item | How |
|---|---|
| [clang-regex-ci-failure](../review-findings-2026-10-30-pr131.md#clang-regex-ci-failure) | #138 gave every negative compile test one `EXPECT` line per compiler.  CI on `main` is green across the clang matrix. |
| `ruff-pin-mismatch` | `pyproject.toml` and `.pre-commit-config.yaml` both pin ruff 0.17.2. |
| `stale-names` | This branch.  `README.md` footnote 5 says "codec" again; `docs/benchmarks/README.md` and `PLATFORM_NOTES.md` use `whatwg::transcode` and `whatwg::decode`; `docs/p2728-alignment.md` describes the current code in current names and P5-Step 3c's in its own; `CLAUDE.md`'s architecture section describes the `whatwg/` layout. |
| [x-user-defined-encoder](../review-findings-2026-10-30-pr131.md#x-user-defined-encoder) | `whatwg::encoder<x_user_defined>` implements the Standard's encoder: an ASCII code point is its own byte, U+F780 to U+F7FF are 0x80 to 0xFF, anything else is an error.  It models `single_byte_text_encoder`, so its views are random access.  `has_encoder` and `transcode_string` accept it, and the wording no longer says the Standard defines no such encoder. |
| `decode-state-constraint-leak` | `flush` is constrained on the exposition-only `$has-decode-state$<C>`, and the decoder's Remarks say which encodings it holds for.  Still showing `detail::` in the synopsis, and not raised by the review: the types of the exposition-only `$state$` members of `whatwg::decoder` and `whatwg::encoder`. |
| `encoding-standard-review-draft` | [intro.refs] names the June 2025 Review Draft.  The five commits between it and the commit the tables come from are meta and editorial.  Recorded in [encoding-standard-reference](../decisions.md#encoding-standard-reference). |
| `text-encoding-cast` | "Why Not `text_encoding::id`?" now says no cast between the enumerations can be correct: several ids land on one encoding, and three encodings have none. |
| `label-matching-quote` | The same section quotes the Encoding Standard's note that its label matching is stricter than UTS #22, for compatibility with deployed content. |
| `caller-buffer-answer` | The bulk-conversion section says the WHATWG views allocate nothing and shows a bounded write into a caller-owned buffer, compiled in `examples/paper_bulk_conversion.cpp`. |
| `runtime-encoding-visitor` | Decided 2026-11-08: no visitor over `whatwg::encoding`.  The paper's runtime example ended in "Use std::visit or switch", which invited the question; it now hands the labels to `whatwg::transcode_string`.  Looking at the use cases behind the question found a real gap, which is [runtime-codec-objects](#stage-6--runtime-codec-objects) below. |

## Open

Ordered so that each stage can land as its own pull request, smallest and
least contested first.  Stages 1 to 4 are done and listed above; Stage 6 is built, below; Stage 5's measurements are what remain.

### Stage 5 — benchmarks

These need a quiet machine and `make bench`.  The cloud container's numbers in
the review are a guide, not publishable.

- [naive-iconv-comparison](../review-findings-2026-10-30-pr131.md#naive-iconv-comparison).
  The `naive_iconv_to_string` benchmark is on this branch.  Re-measure, then:
  rename or drop the README's `Raw iconv()` row, which measures 4 KB chunking
  rather than a floor; replace the "~4x" claims; put the table next to the
  paper's Before/After `iconv` example, with a caption saying what "naive"
  means; and say in the iconv adaptor section that `iconv_transcode_to` is the
  whole-buffer form and that the view and `iconv_transcode_into` lose to the
  naive loop.
- [bulk-helper-vs-ranges-to](../review-findings-2026-10-30-pr131.md#bulk-helper-vs-ranges-to).
  Measure `decode_to` / `encode_to` against `view | ranges::to`, and
  `decode_into` / `encode_into` against `ranges::copy` into a pre-sized buffer.
  Add a `view | ranges::to` row for UTF-8 to UTF-32 beside the iconv rows.
- `benchmark-baseline-break`.  Done: `docs/benchmarks/README.md` says once,
  near the top, that numbers from before and after commit `c447694a` are not
  comparable, and why.
- `corpus-download-user-agent`.  Done: `tools/download_benchmark_corpora.py`
  sends a User-Agent naming the project, with a test.

### Stage 6 — runtime codec objects

`runtime-codec-objects`.  Decided 2026-11-08.

Processing an SMTP or MIME message is a prime use case: each part declares its
`charset` in the data, and the reader wants the part's text as `char32_t`.
Today it cannot get that without writing a `switch` over every encoding.
`whatwg::transcode_string` takes runtime encodings but always re-encodes to
bytes, and the code-point helpers `decode_to<C>` and `decode_into<C>` take the
encoding only as a template parameter.  It is also a gap against P2728, where
getting `char32_t` from UTF is the straightforward case: code points are the
common currency of text conversion, and getting them should be as easy for an
encoding named at run time as for one named at compile time.

The fix, in two layers:

- `whatwg::dynamic_decoder` and `whatwg::dynamic_encoder`, codec objects that
  hold a `whatwg::encoding` chosen at run time and model `text_decoder` and
  `text_encoder`.  They plug into what exists: `bytes |
  decode(whatwg::dynamic_decoder{e})` is a lazy `char32_t` range, and
  `decode_or_error`, `encode`, `transcode(from, to)` and `ranges::to` work
  unchanged.  A MIME body arrives as a range out of a base64 or
  quoted-printable decoder, and can be large, so the lazy form matters.  Each
  call to `decode_one` dispatches on the encoding; callers opt into that cost,
  so the paper's argument against dispatching on every code point by default
  still stands.
- Runtime overloads of the bulk helpers on top of them: `decode_to(src, e)`,
  `encode_to(src, e)` and the `_into` forms, by `whatwg::encoding` and by
  label.  `transcode_string` becomes a thin layer over the same objects, and
  its 40-case `switch` goes away.

A MIME reader, as a real example, is the first caller of both layers and is
written before them, so the API is shaped by the program that needs it:

- `examples/mime_reader/`, its own example target.  Not a library facility,
  and nothing in it goes near a specification header.
- A small parser-combinator kit, copied from specgen's `parse/cursor.hpp` and
  `parse/parser.hpp` (themselves adapted from compile-time-scheme to run at
  run time; same author, same license) and adjusted to read bytes, since a
  MIME message is bytes until a part's charset says otherwise.
- A grammar written close to the RFCs' ABNF, covering what a reader needs and
  no more: RFC 5322 header fields with folding; RFC 2045 `Content-Type` with
  token and quoted-string parameters, and `Content-Transfer-Encoding`; RFC
  2046 multipart bodies with nested boundaries, preamble and epilogue
  skipped; RFC 2047 encoded-words in unstructured headers such as `Subject`.
  RFC 2231 parameter continuations are out of scope.
- Base64 and quoted-printable as lazy byte views, so a part's text is
  `body | base64 | decode(whatwg::dynamic_decoder{*cs})`.  Encoded-words are
  short, so they use the eager `decode_to(bytes, label)`.
- The charset goes through `whatwg::get_encoding`, so `us-ascii` and
  `iso-8859-1` mean windows-1252 as they do in a browser.  An unknown charset
  is reported, not guessed.
- A few messages under `examples/mime_reader/messages/`, chosen to exercise
  the runtime path: an ISO-2022-JP text part, a Shift_JIS attachment, a
  windows-1252 quoted-printable part, a UTF-8 base64 part, and a KOI8-R
  encoded-word subject.  A ctest runs the reader over each and compares its
  UTF-8 output with an expected file.

What was built, and how the questions were settled:

- `whatwg::dynamic_decoder` and `whatwg::dynamic_encoder` in
  `whatwg/dynamic_codec.hpp`, clause `[transcode.whatwg.dynamic]`.  Each holds
  a `std::variant` over the per-encoding codecs, indexed by the enumerator's
  value.
- An encoding with no encoder: constructing a `dynamic_encoder` for it is a
  precondition violation (*Preconditions:* `has_encoder(e)`).  The eager
  `encode_to` returns `nullopt` and `encode_into` returns `false` for it, so a
  program holding a charset from the data checks once, in the call it makes.
- Refinements: both objects model the flushable refinement, and the decoder
  models `bom_aware_text_decoder`, forwarding `decode_first` to `decode_one`
  where the encoding strips no BOM.  Neither models the single-byte
  refinements.
- `constexpr`: both are usable in constant evaluation, with `constify` tests.
- The eager runtime overloads dispatch once, through
  `detail::with_encoding`, and then run the compile-time loop.
  `transcode_string` uses the same dispatch, so its two 40-case `switch`es are
  gone.
- The MIME reader in `examples/mime_reader/` decodes part bodies lazily through
  `dynamic_decoder` and encoded-words through `decode_to(bytes, label)`.  Its
  seven messages match Python's `email` package.
- The paper's runtime example and "Template Parameter vs Runtime Selection"
  now show both forms and what each costs.

Still open:

- The paper's dispatch-cost figure (about 121 instructions per call) was
  measured against `transcode_string`'s old `switch`.  The fold-based dispatch
  may compile differently, so re-run the dispatch benchmark with Stage 5's
  measurements before the figure is published again.
- Argument order: the runtime overloads take the source first,
  `decode_to(source, e)`, like `transcode_string`, while the codec-object
  bulk helpers take the codec first, `decode_to(codec, source)`.  Worth a
  decision before R1.
- The exposition-only members of `dynamic_decoder` and `dynamic_encoder` show
  a `detail::` type in the synopsis, as `whatwg::decoder`'s `$state$` does.

## Not actionable

- `decode_result` carries a `whatwg::error` while the concepts are presented
  as generic.  The paper owns this; it is the next design question, not a
  defect.
- The random-access iterators compare by position only.  Consistent with the
  single-byte codecs being stateless.
