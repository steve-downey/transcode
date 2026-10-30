<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Review of PR #131, the SG16 review response — 2026-10-30

This document records a review of pull request #131 (`sg16-review-response`,
head `f225333`) against three things: the pull request on its own, the SG16
mailing-list thread on P4246R0 it answers, and the WHATWG Encoding Standard
as checked out at `whatwg/encoding` commit `be17ac5`. It is a triage
document, not a set of fixes.

Status terms:

- **Confirmed** means the behavior was reproduced, or follows directly from
  the code, the spec text, or a CI log.
- **Decision** means a contract must be chosen before there is one correct
  fix.
- **Verify** means the evidence indicates a likely problem, or a claim, that
  still needs a focused experiment.

## Verdict

The PR answers the thread well in substance. The names follow the Encoding
Standard, the WHATWG views are now the generic views over `whatwg::decoder`
and `whatwg::encoder`, the `std::text_encoding::id` bridge goes both ways, and
the four wording defects Jonathan Wakely listed are fixed in the generated
clauses. Two findings block a merge, one of them a CI failure. Two thread
points are under-answered, and two benchmark claims the paper and README rest
on need confirming and surfacing.

## Blocking

### B-01: Every clang CI job is red (Confirmed)

Commit `5965c43` ("tighten loose regexes") rewrote the
`PASS_REGULAR_EXPRESSION` of 17 negative compile tests to GCC's phrasing,
`required for the satisfaction of 'X<T>' ... [with T = ...]`. Clang reports
the same failure as `'T' does not satisfy 'X'` and `candidate template
ignored: constraints not satisfied [with T = ...]`. All twelve clang jobs
(19 through 22, C++23 and C++26, Debug, Release and MaxSan) fail those 17
tests, and "Required CI" fails with them. GCC jobs pass. The regexes on
`main` were compiler-neutral (`satisf.*forward_range`, `legacy_byte_range`),
which is why the failure is new.

`PASS_REGULAR_EXPRESSION` takes a list and passes on any match, so the GCC
regex can stay and a clang regex be appended. The patterns below were checked
against the diagnostics in the clang 19 C++23 Debug job log (job
123669031415); each matches its own test's output and nothing else in it.

| test | clang regex to append |
|---|---|
| `flushable_encoder_reject_fail` | `'wrong_flush_encoder' does not satisfy 'flushable_text_encoder'` |
| `whatwg_decoder_decode_byte_reject_fail` | `does not satisfy 'random_access_decode_codec'` |
| `bom_aware_decoder_reject_fail` | `'wrong_first_decoder' does not satisfy 'bom_aware_text_decoder'` |
| `whatwg_decoder_decode_first_reject_fail` | `does not satisfy 'strips_leading_bom'` |
| `whatwg_decoder_flush_reject_fail` | `no_decode_state>' evaluated to false` |
| `whatwg_encoder_encode_byte_reject_fail` | `does not satisfy 'random_access_encode_codec'` |
| `random_access_encode_view_reject_fail` | `encode_view<multibyte_encoder, .*does not satisfy 'random_access_range'` |
| `whatwg_encoder_reject_fail` | `class template 'encoder' \[with C = .*encoding::utf_16be\]` |
| `whatwg_encoder_flush_reject_fail` | `== encoding::iso_2022_jp' evaluated to false` |
| `concepts_reject_unicode_chars_fail` | `'std::vector<char8_t> &' does not satisfy 'legacy_byte_range'` |
| `whatwg_encode_reject_char_range_fail` | `'std::vector<char>' does not satisfy 'unicode_scalar_range'` |
| `utf16_decode_reject_char32_range_fail` | `'std::vector<char32_t>' does not satisfy 'legacy_byte_range'` |
| `transcode_decoder_as_encoder_fail` | `whatwg::decoder<.*encoding::gbk>' does not satisfy 'text_encoder'` |
| `sniff_reject_char32_range_fail` | `'std::vector<char32_t> &' does not satisfy 'legacy_byte_range'` |
| `sniff_reject_input_range_fail` | `single_pass_bytes &' does not satisfy 'forward_range'` |
| `decode_view_dangling_fail` | `indirection requires pointer operand \('borrowed_iterator_t<decode_view<identity_codec, owning_view` |
| `encode_view_reject` | `'not_an_encode_codec' does not satisfy 'text_encoder'` |

Clang prints an enumerator argument as `(encoding)36` in these notes, so the
tests that take an enumerator cannot pin the argument on clang the way they
do on GCC; on clang the concept name is what they check.

### B-02: The wording says the Encoding Standard defines no x-user-defined encoder; it does (Confirmed)

`encoding.bs` has an "x-user-defined encoder" section: an ASCII code point
is its own byte, U+F780 to U+F7FF map back to 0x80 to 0xFF, and anything
else is an error. The Standard's own note under "Encoders and decoders" says
that only the replacement and UTF-16BE/LE encodings have no encoder. The
library has never implemented this encoder, and on `main` that was a
comment. This PR writes the claim into the proposed wording and the README:

- `[transcode.whatwg.encode]` *Mandates*: "It defines none for `utf_16be`,
  `utf_16le`, `replacement` or `x_user_defined`."
- `[transcode.whatwg.encoders]` *Remarks*: "for which the Encoding Standard
  defines no encoder".
- `README.md`, the paragraph after the encode example.

The encoder is about ten lines, is stateless, and every scalar value encodes
to one byte or none, so `whatwg::encoder<x_user_defined>` can model
`single_byte_text_encoder` and keep the random-access path. If it is left out
on purpose, the wording has to say the library does not provide it, not that
the Standard does not define it. This is the one place the PR contradicts
the spec checkout.

## Spec conformance

Everything else was checked against the `encoding` checkout and matches.

- `docs/whatwg/encodings.json` and all index files are byte-identical to the
  spec repository. The two spec commits after the one the paper's
  bibliography pins (`a985b62`) are editorial: "run these steps" is dropped
  from algorithm openings, and the `peek` loop is reindexed. The pin is still
  accurate.
- `whatwg::get_encoding` is "get an encoding": strip leading and trailing
  ASCII whitespace, ASCII case-insensitive match against the label table. The
  generated `name_table` holds the Standard's names, which is what makes
  `to_text_encoding_id` land on `windows1252` rather than on an alias.
- `whatwg::bom_sniff` is "BOM sniff": a non-consuming peek of up to three
  bytes, same table, same order. Requiring `forward_range` is how a range
  says "peek".
- `whatwg::decoder<replacement>` matches the replacement decoder: one error
  for any non-empty input.
- The ISO-2022-JP encoder's "escape back to ASCII, then the replacement"
  matches the Standard's handler, which restores the code point, emits
  `1B 28 42`, and only then returns the error. Encoder-supplied replacement
  bytes are the right model for that.
- No encoder for replacement and UTF-16BE/LE: per spec.
- **Decision**, one sentence owed in `[transcode.whatwg.decoders]`. The
  Standard says byte order mark handling is "not part of the encodings
  themselves" but of the wrapper algorithms (`decode`, `UTF-8 decode`, and
  `TextDecoder` with `ignoreBOM` false). The clause calls
  `whatwg::decoder<C>` "the decoder ... the Encoding Standard defines" and
  then gives it `decode_first`, which strips the mark. The behavior is
  unchanged and is what the Web Platform Tests expect; the remark should say
  the object is the decoder plus the BOM step of the Standard's decode
  wrapper, or SG16 will quote the note back.
- The error modes are replacement and report. The Standard's "html" encoder
  mode (`&#NNNN;`) is not offered. Pre-existing and reasonable, but the paper
  now carries Jan Schultke's "inherit that mess" quote, so the question will
  come.

## Does the PR answer the thread?

| reviewer | point | state |
|---|---|---|
| Wakely | why a new enum and not `text_encoding::id` | Answered: "Why Not `text_encoding::id`?" and the two-way bridge, with the Shift_JIS `5C 7E` example. |
| Wakely | if an enum is a must, make a cast between the two sufficient | Not addressed as asked. The bridge is functions, not aligned values. One sentence that the map is many-to-one (`ISOLatin1`, `ASCII` and `windows1252` all land on `windows_1252`) and that three encodings have no id closes it. |
| Wakely | why a novel label mechanism | Answered implicitly by the `latin1` to windows-1252 example. The Standard's note under "get an encoding" says outright that its matching is more restrictive than UTS #22, which is what `text_encoding`'s alias matching follows, and that this is for compatibility with deployed content. Quote it. |
| Wakely | is "codec" the right word; is it an enum or a class? | Answered: `whatwg::encoding`, `whatwg::decoder` and `whatwg::encoder`, `text_decoder` and `text_encoder`. "codec" now only ever means the object that does the work. The stable names `[transcode.codec.*]` and the header `codec_concepts.hpp` keep the word; fine. |
| Wakely | "values unspecified" contradicts the wording | Fixed in the paper prose. |
| Wakely | "Each enumerator names the encoding of the same name..." is rationale | Rationale moved to prose. The remark is now "denotes the encoding that the WHATWG Encoding Standard names correspondingly", which is still specification by reference to a living document. ISO rules allow a normative reference only to a dated document, and `whatwg/encoding` publishes dated Review Drafts (`review-drafts/2025-06.bs`) for exactly this. The bibliography already pins a commit; cite the Review Draft too and say the enumerators are that draft's table. |
| Wakely | orphaned *Remarks* and *Constraints* after the synopsis | Fixed: `transcode.syn.md` has no paragraphs. The follow-ups list notes 47 `wording-validate` findings where `whatwg::` and `detail::` leak into rendered wording. One of them is not a specgen problem: `flush`'s *Constraints* reads `same_as<detail::decode_state_t<C>, detail::no_decode_state>` and needs an exposition-only name such as `$has-decode-state$<C>` now. |
| Freire | closed enumeration, no user extension | Answered: three layers, `text_decoder` and `text_encoder`, the ISO-8859-1 example compiled as a test. |
| Freire | heap-allocated results, no caller buffer | Not called out. The views allocate nothing; `decode_into`, `encode_into` and `ranges::copy` into a `span` are the caller-buffer path. Say so where the point was raised. See also BM-01 below. |
| Schultke | algorithm, not view; "range in, code units out" | Partly. `decode_to` and `encode_to` are that, and the dispatch cost is now measured. The complaint underneath is that a runtime `encoding` makes the user write a 39-case `switch`, and the paper still answers "use `std::visit` or `switch`". `whatwg/transcode_string.hpp` already has that switch. Expose it once, as a visitor over `whatwg::encoding` that calls `f(integral_constant<encoding, E>{})`, and the objection goes away in twenty lines. |
| Schultke, Freire | views versus a bulk API, performance | The callgrind gate and the `-O3` preset fix are the right evidence. The wall-time tables in the commit messages flag Shift-JIS decode at +167% and +413% in intermediate commits; the PR body explains those as noise at `-O2` with the values discarded. Keep that explanation next to the numbers wherever they are published. See BM-02. |

## Benchmarks: claims to confirm, and where readers find them

Two claims carry weight in the thread and are stated in the paper and README
on numbers that predate this PR's benchmark changes. Both need re-running on
the PR head, because `count_elements` now consumes its values and the
`gcc-release` preset now really builds at `-O3`, so the published numbers and
the new ones are not comparable.

### BM-01: a bulk WHATWG helper adds nothing over `view | ranges::to` (Verify)

The paper says there is "no measurable difference in speed between a named
`decode_to` and the equivalent `view | ranges::to` composition", and early
benchmarks said the same. That is the answer to Freire's buffer point and
half the answer to Schultke's "range in, code units out": the eager helper
is a name, not a faster path, and an eager API with a caller-supplied buffer
would be the same loop again. The claim is load-bearing and is asserted
without a table. Measure `whatwg::decode_to<C>(r)` and
`whatwg::encode_to<C, string>(r)` against `r | whatwg::decode<C> |
ranges::to<vector<char32_t>>()` and `r | whatwg::encode<C> |
ranges::to<string>()`, and `decode_into` and `encode_into` against
`ranges::copy` into a pre-sized buffer, at `-O3` with values consumed, on the
existing corpora. Put the table in the README's performance section under its
own heading and cite it from the paper's "Bulk Operations" paragraph, with
the preset and commit it was taken at.

### BM-02: every path in the library beats naive iconv (Verify)

"Naive iconv" means the code a search for "run iconv into a `std::string`"
produces with no other constraints: `iconv_open`, a `std::string` sized to
a guess, a loop of `iconv()` calls that grows the string and fixes up the
output pointer on `E2BIG`, a final `resize` to the bytes written, and
`iconv_close`. The paper's "Before: `iconv`" example in the comparison
table is exactly that code.

The benchmark does not measure it. The `Raw iconv()` rows in the README
come from `raw_iconv_process` in `benchmark/iconv_benchmarks.bench.cpp`,
which converts through an already-open handle into a static 4 KB buffer
that it overwrites on every call, never grows, never copies, and never
produces a container; it counts bytes and discards them. That is a floor
for `iconv()` itself, not the program a user would write, and it is the
row the README's "~4x" claims are measured against. The comparison the
paper makes in prose (the "Before" code against the WHATWG pipe) has no
row.

Add the naive loop as a benchmark, as the paper's example is written:
`std::string` sized to four times the input, doubled on `E2BIG`, trimmed at
the end, with `iconv_open` and `iconv_close` inside the timed region, since
the "Before" code opens a handle per call. Then re-run the `iconv`,
`whatwg` and `pluggable` suites on the PR head, confirm that
`iconv_transcode_to`, the streaming WHATWG pipe and `view | ranges::to`
each beat it, and publish it so that it is found:

- one table in the paper, next to the "Before" and "After" examples it
  contrasts, with naive `iconv()`, the `iconv()` floor,
  `iconv_transcode_to`, the streaming WHATWG pipe and `std::codecvt` on the
  same corpus, so a reader who opens the paper for the motivation sees the
  numbers beside the code they describe;
- one sentence in the table's caption saying what "naive" is, so the
  comparison cannot be read as a straw man, and one saying that the
  `iconv()` floor row discards its output;
- the preset, compiler and commit for every row, since the published
  numbers were taken with `count_elements` discarding its values and the
  preset at `-O2`.

If the naive loop turns out faster than `iconv_transcode_to` on some
corpus, that is a finding about `iconv_transcode_to`'s staging, and it
should be reported rather than left out of the table.

The benchmark `README.md` under `docs/benchmarks/` should also say, once, that
numbers from before this PR and after it are not comparable and why.

## Code and documentation

- **Confirmed**: `README.md` footnote 5 now reads "P2728 selects its
  `whatwg::encoding` from the character type", a search-and-replace
  over-reach; it should still say "codec".
- **Confirmed**: old names remain in `docs/benchmarks/README.md` (the
  `transcode<codec::shift_jis, codec::utf_8>` example near line 365),
  `docs/benchmarks/PLATFORM_NOTES.md` (`whatwg_decode<utf_8>`) and
  `docs/p2728-alignment.md` (`whatwg_error`, `transcode_error_kind`).
  `CLAUDE.md` is known and listed in the follow-ups.
- `decode_result` carries a `whatwg::error` while the concepts are presented
  as generic and not WHATWG's. The paper owns this ("the protocol adopts
  WHATWG's error model"); it is the next question after the namespace one.
- The random-access decode and encode iterators compare by position only,
  while the forward iterators also compare codec state. Consistent with the
  single-byte codecs being stateless, which the `single_byte_text_encoder`
  remark requires.
- The PR carries the design response together with `metrics/fanout-runs.jsonl`,
  the `-O3` preset fix and the `count_elements` change. The last two change
  the benchmark baselines for everyone; the PR body says so, and the benchmark
  README should too.

## What was run

- GCC 13, Debug, `make test TOOLCHAIN=gcc-13` on `f225333`: 813 of 813
  ctest pass (the C++26 `text_encoding` targets and the module build are not
  built with GCC 13 at C++23). pytest: 370 pass, 1 skip, 1 fail. The failure
  is `test_tool_versions.py::test_pre_commit_matches_pyproject[ruff]`: the
  dependabot bump of ruff to 0.17.2 in `pyproject.toml` (`27c31b5`) left the
  pre-commit hook at v0.17.1. It is on `main` too, not this PR's change.
- Clang could not be run in the review container, which has clang 18 with
  libstdc++ only; the project's `std::expected` probe rejects that pair. The
  clang findings come from the CI job logs.
- The `voice` lexical checker on `papers/transcode-view.md`, paper register:
  clean.
