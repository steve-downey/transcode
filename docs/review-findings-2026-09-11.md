<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Repository review findings — 2026-09-11

This document records issues found while reviewing the public headers,
implementation, tests, CMake packaging, CI, examples, proposal paper, generated
wording, README, and planning/benchmark documentation. It is a triage document,
not a set of fixes.

Status terms used below:

- **Confirmed** means the behavior was reproduced or follows directly from the
  implementation.
- **Decision** means the implementation and documentation are internally
  inconsistent, but the intended contract must be chosen before there is one
  correct fix.
- **Verify** means the evidence indicates a likely problem that still needs a
  clean build or focused experiment.

## Must address before proposal or public release

### C-01: `sniff_encoding` consumes single-pass input

**Status:** Confirmed correctness and contract defect.

`sniff_encoding` accepts a `legacy_byte_range`, which can be only an
`input_range`, then increments its iterator while examining up to three bytes
([`include/beman/transcode/sniff.hpp`](../include/beman/transcode/sniff.hpp),
lines 22-63). Its wording says the mark is not consumed and the range can be
passed on unchanged (lines 30-35).

With `std::ranges::istream_view<char>`, sniffing a BOM leaves only the payload
for the caller. Sniffing a non-BOM similarly discards the first three bytes.
This is silent data loss in the stream-oriented use case the API is intended to
serve.

**Recommended action:** Require a `forward_range`, return an advanced/replayable
range together with the result, or explicitly make consumption part of the API.
Add tests using a genuinely single-pass range for both BOM and non-BOM input.

### C-02: A too-small iconv view buffer silently loses valid input

**Status:** Confirmed correctness defect.

`iconv_transcode_view::iterator::load` treats `E2BIG` with no produced output as
the end of a load, then enters its flush/completion path
([`include/beman/transcode/iconv_transcode_view.hpp`](../include/beman/transcode/iconv_transcode_view.hpp),
lines 223-313). UTF-8 `"A"` to UTF-32LE with a one-byte caller buffer yields no
bytes. An empty buffer likewise produces an empty range. Neither case tells the
caller that valid input was not converted.

**Recommended action:** Define and enforce a minimum buffer size, dynamically
stage one complete output unit, or report `output_full` without consuming or
completing the range. Add real-iconv tests for zero-, one-, and boundary-sized
buffers.

### C-03: Streaming iconv flush is attempted only once

**Status:** Confirmed correctness defect.

Both streaming views set `flushed_` before one flush call and ignore the call's
return value and `errno`:

- [`include/beman/transcode/iconv_transcode_view.hpp`](../include/beman/transcode/iconv_transcode_view.hpp),
  lines 303-313
- [or-error iconv view](../include/beman/transcode/iconv_transcode_or_error_view.hpp),
  lines 296-307

A stateful converter may return `E2BIG` while still owing shift-state output.
That remaining output is then truncated; the `_or_error` variant does not
surface the error either.

**Recommended action:** Continue flushing until success, retaining pending
state between increments. Treat non-`E2BIG` flush failures according to the
view's error policy. Add a mock that requires two or more flush calls.

### C-04: Lossy iconv bulk replacement can corrupt the destination encoding

**Status:** Confirmed correctness defect and contract mismatch.

The lossy bulk helpers append the raw byte `0x3F` on `EILSEQ` or `EINVAL`
([`include/beman/transcode/iconv_bulk.hpp`](../include/beman/transcode/iconv_bulk.hpp),
lines 102-138 and 190-212). Invalid UTF-8 transcoded to UTF-16LE therefore can
produce the one-byte output `{0x3F}`, which is not valid UTF-16LE. Header wording
instead says rejected input is skipped (lines 79-84).

**Recommended action:** Choose one contract. Skipping preserves destination
well-formedness and matches current generated wording. If replacement is
desired, encode the chosen replacement through the destination converter; do
not append an encoding-independent byte. Apply the same policy to the container
and output-iterator helpers and test multibyte destinations.

### C-05: iconv open and unexpected system errors are conflated

**Status:** Confirmed API and error-model defect.

Failure to open a conversion descriptor has three incompatible results:

- streaming views appear empty;
- lossy bulk APIs return an empty container or unchanged iterator;
- `iconv_transcode_to_or_error` reports `invalid_sequence`
  ([`include/beman/transcode/iconv_bulk.hpp`](../include/beman/transcode/iconv_bulk.hpp),
  lines 89-91, 182-184, and 253-255).

The paper says `iconv_error` also describes a descriptor that would not open
([`papers/transcode-view.md`](../papers/transcode-view.md), line 530), but the
enum contains only the three conversion `errno` categories
([`include/beman/transcode/error.hpp`](../include/beman/transcode/error.hpp),
lines 53-71). Conversion branches also classify any unexpected `errno` as one
of `EILSEQ`, `EINVAL`, or `E2BIG`, depending on the function.

**Recommended action:** Add an explicit unsupported-encoding/open/system error,
or make construction/opening a separate fallible operation. Specify how
unexpected platform errors propagate. Do not use valid empty output as a
failure signal.

### C-06: `null_term_view` has no reachable-terminator precondition

**Status:** Confirmed specification and object-state defect.

`null_sentinel_t` dereferences the current iterator to find zero, while the
adaptor accepts any pointer or array
([`include/beman/transcode/null_term.hpp`](../include/beman/transcode/null_term.hpp),
lines 18-39 and 79-88). No wording requires a zero-valued element to be
reachable before the storage ends, nor states the storage/lifetime requirement.
Without that precondition, iteration can read beyond the object.

The defaulted constructor also leaves `ptr_` indeterminate for pointer
specializations (lines 32-39). Calling `begin` on that object exposes the
indeterminate pointer.

**Recommended action:** Specify a reachable terminator and lifetime
precondition. Value-initialize `ptr_` and define the singular state, or remove
default construction if the view cannot have a useful safe default.

### C-07: Runtime transcoding uses empty output as an unsupported-target error

**Status:** Confirmed API defect.

The `codec` overload of `transcode_string` returns `std::string` and documents
empty output for a codec with no encoder
([`include/beman/transcode/transcode_string.hpp`](../include/beman/transcode/transcode_string.hpp),
lines 71-80 and 316-320). This is indistinguishable from successful conversion
of empty input. The label overload returns an engaged `optional` in this case,
while the compile-time equivalent rejects an unsupported target.

**Recommended action:** Reject decode-only targets before decoding and return a
fallible result such as `expected`/`optional`, or narrow the accepted target
type. Make the codec and label overloads agree.

### P-01: The optional/portable iconv story does not match the build

**Status:** Decision required; build behavior is confirmed.

The paper calls iconv optional
([`papers/transcode-view.md`](../papers/transcode-view.md), line 38), and the
README distinguishes a portable header-only WHATWG implementation. The build
unconditionally requires and links `Iconv::Iconv`
([`CMakeLists.txt`](../CMakeLists.txt), lines 128-140), and the umbrella header
unconditionally includes all iconv APIs
([`include/beman/transcode/transcode.hpp`](../include/beman/transcode/transcode.hpp),
lines 48-52). This also conflicts with the advertised MSVC portability unless a
compatible iconv is separately supplied.

**Recommended action:** Either add a supported feature option and header split
that allows the portable API to install without iconv, or state iconv as a
mandatory platform dependency and narrow the portability claims.

### P-02: Proposed ISO C++ wording exposes an undefined POSIX type

**Status:** Confirmed proposal-surface issue.

The generated `<transcode>` synopsis exposes `iconv_t` in `iconv_functions`
([`include/beman/transcode/iconv_transcode_view.hpp`](../include/beman/transcode/iconv_transcode_view.hpp),
lines 38-41; generated at
[`papers/wording/transcode.syn.md`](../papers/wording/transcode.syn.md), lines
341-344). `iconv_t` and `<iconv.h>` are not ISO C++ facilities, and the proposal
does not define how that name exists on a conforming non-POSIX implementation.

**Recommended action:** Split the POSIX adaptor from the ISO proposal, expose a
library-owned opaque handle/callable contract, or provide explicit wording for
the implementation-defined integration. Resolve this before asking LEWG to
review the synopsis.

## Specification and API decisions

### S-01: Invalid `char32_t` is both a precondition violation and a reported error

**Status:** Decision required.

The codec requirements and encode-view wording require every input element to
already be a Unicode scalar value
([`include/beman/transcode/codec_concepts.hpp`](../include/beman/transcode/codec_concepts.hpp),
lines 45-52;
[`include/beman/transcode/encode_view.hpp`](../include/beman/transcode/encode_view.hpp),
lines 28-35). The UTF-8 implementation and tests nevertheless deliberately
replace or report surrogate and out-of-range `char32_t` values.

**Recommended action:** Decide whether invalid values are outside the function
contract or supported erroneous input. Remove the precondition if reporting is
part of the API; otherwise remove normative promises and tests that imply
defined recovery.

### S-02: Lossy iconv APIs do not share one recovery policy

**Status:** Decision required.

The streaming view skips `EILSEQ` one byte at a time
([`include/beman/transcode/iconv_transcode_view.hpp`](../include/beman/transcode/iconv_transcode_view.hpp),
lines 252-257 and 391-396). Bulk APIs insert raw `?`, while README prose often
speaks generally about replacement. Users cannot substitute an eager API for a
lazy one without changing output.

**Recommended action:** Define skip, replacement, and fatal policies once and
apply them consistently across streaming and eager forms. If multiple policies
are intentional, put the distinction in API names/types rather than prose.

### S-03: `random_access_decode_codec_type` reserves U+FFFD as an error sentinel

**Status:** Confirmed extensibility limitation; decision required.

The concept says `decode_byte` returns U+FFFD when a byte is unmapped
([`include/beman/transcode/codec_concepts.hpp`](../include/beman/transcode/codec_concepts.hpp),
lines 35-43), and `decode_view` interprets U+FFFD for a high byte as an error
([`include/beman/transcode/decode_view.hpp`](../include/beman/transcode/decode_view.hpp),
lines 278-280). A valid custom single-byte codec therefore cannot map a high
byte to the actual U+FFFD scalar in `_or_error` mode.

**Recommended action:** Return a result/optional value from `decode_byte`, add a
separate validity query, or document U+FFFD as reserved and accept the loss of
that mapping.

### S-04: Review generated free-standing synopsis prose

**Status:** Verify generator intent.

The generated synopsis contains substantial semantic prose after the class and
function declarations, including encode/decode view and iconv remarks
([`papers/wording/transcode.syn.md`](../papers/wording/transcode.syn.md), lines
462-490). Much of it is duplicated in the dedicated semantic clauses.

**Recommended action:** Confirm whether specgen is intentionally attaching
these remarks to synopsis declarations. If not, relocate the source annotations
so the synopsis remains a synopsis and each normative rule has one home.

### S-05: Narrow or substantiate factual and performance claims

**Status:** Editorial verification required.

Claims such as “`std::codecvt` allocated on every call,” `mbstowcs` being about
twice as fast, “four browser engines,” and broad iconv adoption are stronger
than the nearby citations or benchmark scope establish
([`papers/transcode-view.md`](../papers/transcode-view.md), lines 46-54 and
329-337). The README phrase “all ISO-8859 and Windows code pages” also exceeds
the actual WHATWG-listed set ([`README.md`](../README.md), lines 6-9).

**Recommended action:** Cite a reproducible implementation/benchmark for each
claim, qualify it to the tested platform and wrapper, and say
“WHATWG-listed ISO-8859 and Windows variants.” Verify the browser-engine count
or use “major browser engines.”

## Documentation, paper, build, and CI drift

### D-01: Duplicate author email key

**Status:** Confirmed.

The paper YAML has two `email` keys for one author
([`papers/transcode-view.md`](../papers/transcode-view.md), lines 11-14). The
renderer warns `Duplicate key: .author[0].email` and discards one value.

**Action:** Keep one address or use the renderer's supported representation for
multiple addresses.

### D-02: Feature-test macro placeholders disagree

**Status:** Confirmed.

The paper uses both `202XXXL` and `20XXXXL`
([`papers/transcode-view.md`](../papers/transcode-view.md), lines 866-870 and
937-942).

**Action:** Choose one placeholder until a real value is assigned.

### D-03: Documentation claims unsupported UTF-32 BOM sniffing

**Status:** Confirmed.

README and paper-tail text say `sniff_encoding` detects UTF-8/16/32 BOMs
([`README.md`](../README.md), lines 291-294;
[`papers/parts/tail.md`](../papers/parts/tail.md), lines 125-128). The function
and `codec` enum support only UTF-8, UTF-16BE, and UTF-16LE BOM results.

**Action:** Remove UTF-32 from the claim, or add UTF-32 codecs and detection as
a separately designed feature.

### D-04: Paper and related proposal revisions have drifted

**Status:** Confirmed.

- [`papers/README.md`](../papers/README.md), line 7 says D4246R0; the paper is
  D4246R1.
- [`README.md`](../README.md), lines 235-242 refers to P2728R12; the paper tail
  and [`docs/p2728-alignment.md`](p2728-alignment.md) use P2728R13.

**Action:** Update the stale revisions and consider defining each cited revision
once to reduce repeated manual updates.

### D-05: Build instructions name the wrong test framework and language level

**Status:** Confirmed.

README and CONTRIBUTING say GoogleTest, but test CMake requires Catch2
([`tests/beman/transcode/CMakeLists.txt`](../tests/beman/transcode/CMakeLists.txt),
lines 1-12). CONTRIBUTING's manual examples request C++17 or C++20
([`CONTRIBUTING.md`](../CONTRIBUTING.md), lines 39-47, 70-77, and 94-101), while
the target requires C++23.

**Action:** Replace GoogleTest with Catch2 and make every configuration example
request C++23.

### D-06: Advertised compiler support disagrees with CMake

**Status:** Confirmed.

README advertises Clang 17 and 18
([`README.md`](../README.md), lines 570-580), but CMake rejects every Clang
version below 19 before considering the selected standard library
([`CMakeLists.txt`](../CMakeLists.txt), lines 22-29).

**Action:** Align the matrix and configuration check. If the limitation is only
Clang plus libstdc++, make the check test that combination rather than the
compiler version alone.

### D-07: The documented `table_codec` API and example are not public examples

**Status:** Confirmed.

README presents `beman::transcoding::table_codec` as provided public API and
links `examples/custom_single_byte_decoder.cpp`
([`README.md`](../README.md), lines 165-186). The type lives under `detail/` and
is explicitly excluded from the proposal surface
([`include/beman/transcode/transcode.hpp`](../include/beman/transcode/transcode.hpp),
lines 56-61). The linked example manually decodes bytes and does not use
`table_codec` or another transcode API.

**Action:** Promote and specify the helper with a real API example, or remove
the public claim and replace the example with a custom codec implementing the
documented concepts.

### D-08: ICU does not expose the POSIX iconv API as stated

**Status:** Confirmed factual error.

README groups ICU with implementations exposing
`iconv_open`/`iconv`/`iconv_close` ([`README.md`](../README.md), lines 188-195).
ICU's native converter API is the `ucnv_*` family.

**Action:** Remove ICU from that list or describe it as a separate transcoding
backend that would need an adapter.

### D-09: A root file literally named `-` contains stale generated wording

**Status:** Confirmed repository hygiene issue.

The tracked root file `./-` contains an old wording snapshot. Git history shows
it was introduced by commit `94a32d2c` (“Print the enumerator meanings as
tables, now that specgen can”).

**Action:** Remove the file and fix the generation/redirection invocation that
created it. Add a CI hygiene check if the failure mode can recur.

### D-10: `docs/plans/handoff-next.md` is stale operational guidance

**Status:** Confirmed.

The handoff reports 707 C++ tests, a future paper update, and merge instructions
for an old branch
([`docs/plans/handoff-next.md`](plans/handoff-next.md), lines 20, 71-88). It now
reads as current guidance despite describing an earlier project state.

**Action:** Archive/remove it, or add a prominent historical date and completed
status.

### D-11: “Manual Lint” CI repeats the normal lint target

**Status:** Confirmed.

The `Manual Lint` step runs `make lint`, exactly like the preceding `Lint` step
([`.github/workflows/test_makefile.yaml`](../.github/workflows/test_makefile.yaml),
lines 86-95).

**Action:** Run `make lint-manual` if that is the intended additional gate, or
remove/rename the duplicate step.

### D-12: Module packaging needs a clean install-consumer test

**Status:** Verify; SPDX omission is confirmed.

[`include/beman/transcode/transcode.cppm`](../include/beman/transcode/transcode.cppm)
lacks the repository-required SPDX header. In module mode, the installed header
file set contains only `codec.hpp`, `config.hpp`, `transcode.hpp`, `todo.hpp`,
and generated config
([`include/beman/transcode/CMakeLists.txt`](../include/beman/transcode/CMakeLists.txt),
lines 3-15), while the module interface includes `transcode.hpp`, which includes
many more headers.

**Action:** Add the license header. Test install-and-consume in module mode from
a clean prefix; if consumers rebuild the interface unit, install every header
needed by its include graph.

### D-13: Telemetry script shebang is on line 2

**Status:** Confirmed, low priority.

`infra/cmake/telemetry.sh` has its shebang on the second line, which shellcheck
flags. Current CMake usage invokes it explicitly via Bash, so this is not a
runtime blocker.

**Action:** Put the shebang first, then the SPDX comment, or suppress the check
with an explanation.

## Verification results and gaps

The following checks passed against the reviewed tree:

- Python tests: 252 passed (`make pytest`).
- Python typing: `make mypy` passed.
- C++ formatting checks and Ruff passed.
- Every example compiled and ran directly against the current headers with
  `g++-16`.
- Paper rendering completed, apart from the duplicate-email warning in D-01.
- `make wording-inputs-check` passed.

The following remain open verification work:

- A fresh C++ test build was not completed. CMake stalled while cloning Catch2.
  Existing CTest executables predate recent public-header changes, so their
  results must not be treated as verification of the current tree.
- Full `make wording-check` could not run because `specgen` is not installed.
- `codespell` was not available.
- Run the new focused regressions described in C-01 through C-05 on at least
  glibc iconv and one other iconv implementation.
- Run an install-consumer test with `BEMAN_TRANSCODE_USE_MODULES=ON` for D-12.

## Suggested triage order

1. Resolve the ISO/POSIX scope and optional-dependency questions (P-01, P-02).
2. Fix silent data loss and malformed output (C-01 through C-04).
3. Settle the public error contracts (C-05, C-07, S-01 through S-03).
4. Regenerate and review wording after the API decisions (S-04, D-01 through
   D-04).
5. Clean up user-facing build/platform docs and CI (D-05 through D-13).
6. Complete fresh C++, wording, install, and cross-iconv verification.
