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
| `runtime-encoding-visitor` | Decided 2026-11-08: no visitor.  Most users never write the `switch`: `whatwg::transcode_string` takes both encodings at run time, as values or as labels, and that covers the "range in, code units out" case Schultke raised.  That the question came up at all was a documentation defect: the paper's runtime example ended in "Use std::visit or switch".  The example now calls `transcode_string` with the labels, and "Template Parameter vs Runtime Selection" says which programs write a `switch`: only one that wants a lazy view, or `char32_t` values, over a runtime encoding. |

## Open

Ordered so that each stage can land as its own pull request, smallest and
least contested first.  Stage 1, the stale names, is done on this branch and
listed above.

### Stage 2 — the x-user-defined encoder

[x-user-defined-encoder](../review-findings-2026-10-30-pr131.md#x-user-defined-encoder).
The one place the proposal contradicts the Encoding Standard.  Still open:
`whatwg/encode_view.hpp`, `whatwg/encoder.hpp`, the generated
`[transcode.whatwg.encoders]` and `[transcode.string]`, and the README all say
the Standard defines no x-user-defined encoder.  It defines one.

Default: implement it.  ASCII code points are their own byte, U+F780 to U+F7FF
map to 0x80 to 0xFF, anything else is an error.  It is stateless and every
scalar value encodes to one byte or none, so `whatwg::encoder<x_user_defined>`
models `single_byte_text_encoder` and takes the random-access path.  Runtime
test, WPT vectors if the suite has them, the constraint tests that currently
expect rejection flipped, `constify` test, `make wording`, and the README and
paper text corrected.  `transcode_string` and `text_encoding.hpp` special-case
`x_user_defined` and need revisiting.

### Stage 3 — wording

- `decode-state-constraint-leak`.  `flush`'s *Constraints* in
  `[transcode.whatwg.decoders]` and the synopsis render as
  `same_as<detail::decode_state_t<C>, detail::no_decode_state>`.  It needs an
  exposition-only name, such as `$has-decode-state$<C>`.
- `encoding-standard-review-draft`.  The bibliography pins a commit of the
  living standard.  ISO allows a normative reference only to a dated document,
  and `whatwg/encoding` publishes dated Review Drafts for exactly this.  Cite
  the Review Draft as well, and say the enumerators are that draft's table.
  Also add a dated entry to
  [encoding-standard-reference](../decisions.md#encoding-standard-reference).

### Stage 4 — answers owed to the SG16 thread

Paper prose, each a sentence or a paragraph where the point was raised.

- `text-encoding-cast` (Wakely).  Mostly answered: the paper shows `ISOLatin1`
  and `ASCII` both landing on windows-1252 and lists the three encodings with
  no id.  It never draws the conclusion he asked for: a many-to-one map with
  holes means no cast between the two enumerations can be correct.
- `label-matching-quote` (Wakely).  Quote the Standard's note under "get an
  encoding": its matching is more restrictive than UTS #22, which
  `text_encoding` follows, for compatibility with deployed content.
- `caller-buffer-answer` (Freire).  Not addressed.  The views allocate
  nothing; `decode_into`, `encode_into` and `ranges::copy` into a `span` are
  the caller-buffer path.

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
- `benchmark-baseline-break`.  `docs/benchmarks/README.md` should say once
  that numbers from before #131 and after it are not comparable:
  `count_elements` now consumes its values, and the `gcc-release` preset now
  really builds at `-O3`.
- `corpus-download-user-agent`.  `tools/download_benchmark_corpora.py` sends no
  User-Agent, and Wikipedia refuses it.

## Not actionable

- `decode_result` carries a `whatwg::error` while the concepts are presented
  as generic.  The paper owns this; it is the next design question, not a
  defect.
- The random-access iterators compare by position only.  Consistent with the
  single-byte codecs being stateless.
