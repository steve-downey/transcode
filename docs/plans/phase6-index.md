# Phase 6: Acting on the 2026-09-11 Review

## Context

`docs/review-findings-2026-09-11.md` is a triage of the public headers, the
implementation, the tests, CMake packaging, CI, the examples, D4246R1, the
generated wording, the README and the planning documents.  It records 30
findings and says of itself that it is "a triage document, not a set of fixes."

Phase 6 is the set of fixes.

The review is not a code review in the usual sense.  Seven of its findings are
places where the implementation and its own docblock say different things, and
because those docblocks *are* the wording -- specgen reads them and renders the
paper's normative clauses from them -- a disagreement between code and comment
in this repository is a disagreement between the implementation and the
proposal.  That is what makes them worth a phase rather than a cleanup branch.

## Measurements taken before planning (2026-09-11)

Facts from the tree at `6281764f`, not estimates.  Every substantive finding in
the review was checked rather than taken.

- **All seven `C-` findings reproduce.**  The line references in the review are
  current.  Two details the review states loosely are worth writing down
  precisely:
  - `sniff_encoding` advances its iterator **twice**, not three times
    (`sniff.hpp:44`, `:49`); the third byte is read without an increment.  A
    single-pass caller loses two bytes, not three.
  - The `else` branches at `iconv_bulk.hpp:125`, `:207`, `:277` and
    `iconv_transcode_view.hpp:252` are commented `EILSEQ` and `EINVAL` but are
    reached for **any** unexpected `errno`.  The misclassification the review
    mentions under C-05 is in the streaming view as well as the bulk helpers.
- **The iconv adaptors are inside the proposed synopsis.**
  `transcode.hpp:49-52` includes all four iconv headers between the
  `\rSec2[transcode.syn]` marker and the `/// END` fence, so `iconv_t`
  (`iconv_transcode_view.hpp:39`) is a declaration of the proposed
  `<transcode>`.  P-01 and P-02 are therefore one finding with two faces, and
  neither can be answered without deciding the other.
- **`make lint-manual` already exists** (`Makefile:281`).  D-11 is a one-word
  fix in the workflow, not a new target.
- **`specgen` is not installed on this machine.**  `~/.local/bin/specgen` is
  absent and `~/src/specgen/` is empty.  Building it links LLVM's Clang front
  end.  See "The wording gate" below: this phase revises the check that would
  otherwise make that build a prerequisite for eight steps.
- **711 C++ tests register** under `ctest -N`.  `docs/plans/handoff-next.md`
  says 707, which is the drift D-10 is about.
- **One finding the review did not have.**  `pre-commit run gitleaks` fails on
  every file, modified or not: the pinned hook calls a `gitleaks git`
  subcommand the resolved binary does not have.  `make lint` therefore has a
  permanently red hook.  It is D-14, and it belongs to Step 9.
- **The four sibling worktrees are stale.**  `add-clang-tidy`,
  `codex/gb18030-e5e5-conformance`, `codex/remove-utf16-encode-paths` and
  `codex/scalar-value-precondition` are all ancestors of `main`; none carries
  unmerged work.  There is no in-flight change this phase can collide with.

## Decisions taken

The review says of six findings that a decision must come before there is a
correct fix.  Those decisions are made here, once, so no step has to re-open
them at the keyboard.

| Findings | Decision |
| --- | --- |
| P-01, P-02 | **Document as implementation-defined.**  `iconv_functions` and its handle type get wording saying they are implementation-defined and that `[transcode.iconv]` is POSIX-conditional.  No header split, no CMake option, no new wording root.  The README's portability claims are narrowed to match a build that requires iconv, and the paper's existing question to SG16 about splitting the adaptor is sharpened into an explicit ask. |
| C-02, C-03, C-05 | **Extend `iconv_error` and fix the loops.**  New enumerators for open and system failures, a flush that resumes instead of being abandoned, unexpected `errno` classified rather than folded into the skip path, and a documented minimum buffer size.  The API shapes do not change. |
| C-04, S-02 | **Skip, do not replace.**  The bulk helpers stop writing a raw `0x3F` and skip, which is what the streaming view does and what the committed wording already says.  One recovery policy across the iconv family. |
| C-01 | **Require `forward_range`.**  The docblock's promise that the mark is not consumed becomes true, rather than being rewritten to admit that it is. |
| C-06 | **Specify the precondition and value-initialize.**  A reachable terminator and a lifetime requirement become `\expects`; `ptr_` stops being indeterminate. |
| C-07 | **Return `optional`.**  The `codec` overload of `transcode_string` returns `optional<string>` and rejects an encoder-less target before decoding, so it agrees with the label overload and empty output means empty output. |
| S-01 | **Drop the precondition; specify UTF-32 input validation.**  A `char32_t` that is not a Unicode scalar value yields U+FFFD in replacement mode and the corresponding error in `expected` mode, and the validation happens once, above codec dispatch.  See "S-01, resolved" below; this reverses the provisional decision. |
| C-08 (new) | **Validate above dispatch.**  The surrogate and out-of-range checks exist only for UTF-8; the other thirty-nine codecs misreport the same input as `unmapped_codepoint`.  Fixed by the same change as S-01. |
| S-03 | **Document the reservation.**  U+FFFD is reserved as the unmapped-byte signal, the extensibility cost is stated, and the question of a better signal goes to SG16 rather than being answered here. |

C-07 is source-breaking: it changes a return type.  That is cheap now and
expensive after LEWG review, which is the argument for doing it in this phase
rather than the next.

### S-01, resolved

The first decision here was "keep the precondition, drop the promises."  It was
wrong, and the way it was wrong is worth recording, because the same mistake is
easy to make again.

It rested on a true premise: WHATWG's encoders do take scalar values, and the
Encoding Standard enforces it in *process an item* (§4.1) with `Assert:
encoderDecoder is not an encoder instance or item is not a surrogate`, plus a
second assertion that an encoder never runs in `"replacement"` mode -- so the
U+FFFD branch is structurally unreachable on the encode path.

The error was inferring from that that surrogate substitution is a JavaScript
binding artifact this library need not honour.  It is not.  It is Infra's
*convert a string into a scalar value string* (§4.7), and **three** separate
specifications invoke it above the codec: Web IDL for
`TextEncoder.encode(optional USVString input)`; the Encoding Standard itself in
§7.6 for `TextEncoderStream`, whose note says outright that "lone surrogates
will be replaced with U+FFFD"; and HTML in *create an entry*, for form names
and values.  URL's *percent-encode after encoding* takes a scalar value string
as a typed precondition.  Every entry point launders; none feeds a surrogate
in.  A library that offers the entry point owns the conversion.

The encoder-layer reading also fails on its own terms.  Encoding §8.1.2 selects
by range, and U+D800 falls inside the `U+0800 to U+FFFF` row -- so a "total"
encoder would emit `ED A0 80`, which Unicode D92 calls ill-formed and C10 says
"must never be generated."  The reading that makes the encoder total produces
the one output that is definitely non-conformant.

So the behaviour is defined, the implementation already implements it, and the
wording is what was wrong.  The library validates its `char32_t` input as
UTF-32 and substitutes U+FFFD -- which is P2728's model too, attributed there
to the UTF-32 decode step, since `char32_t` in and bytes out is two operations
and WHATWG specifies only the second.

Two things fell out of settling it:

- **C-08.**  The surrogate and out-of-range checks live inside the
  `if constexpr (C == codec::utf_8)` branch of `whatwg_encode_view::load()`.
  Every other codec reaches `emit_error(whatwg_error::unmapped_codepoint,
  {'?'})` instead (`whatwg_encode_view.hpp:499`, `:524`, `:535`, `:546`,
  `:557`, `:568`), so U+D800 through `windows_1252` is reported as "the
  encoding has no representation for the Unicode scalar value being encoded" --
  which is wrong twice over.  A defect under either answer to S-01.
- **D-15.**  `error.hpp:79` and `:86-89`, `encode_view.hpp:229` and
  `papers/transcode-view.md:371` all say an encode failure yields `'?'`.
  `whatwg_encode<codec::utf_8>` emits `EF BF BD`.

Two things also came off the list.  **Noncharacters are not in play** -- per
Infra §4.6 a noncharacter is not a surrogate, so U+FFFE and U+FDD0–U+FDEF *are*
scalar values and encode normally; the library correctly says nothing about
them.  And **there is no divergence between WHATWG's UTF-8 decoder and
Unicode's U+FFFD guidance**: Encoding §8.1.1 says the constraints "match 'Best
Practices for Using U+FFFD'… No other behavior is permitted," while Unicode
§3.9.6 says it "does not require this practice for conformance."  WHATWG
mandates what Unicode recommends, so "conforms to WHATWG decode" is the
stronger claim and should be stated as such rather than as two co-equal ones.

Values above U+10FFFF remain genuinely undecided *by the standards* -- Infra
caps a code point at U+10FFFF, so such a value is not a code point, not a
surrogate, and has no conversion defined anywhere.  `char32_t` can hold one and
a JavaScript string cannot, so this is a case C++ has to answer for itself.
Step 6 treats it uniformly with surrogates rather than splitting: both are one
comparison away from valid, and putting undefined behaviour one value from
defined behaviour with nothing in the type system to mark the boundary buys
nothing.

## The wording gate, and why it is revised first

`make wording-inputs-check` hashes every spec-facing header and diffs against
`papers/wording/inputs.sha256`.  It runs on every pull request, and it fails
whenever a header moves without `make wording` having been run -- which needs a
`specgen` built from `papers/wording/specgen-ref`.

Taken literally that makes eight of this phase's steps wait on an LLVM-linked
tool, and it would make any contributor who fixes a typo in a docblock build one
too.  `papers/wording/README.md` already concedes the shape of the problem: the
check "over-reports by construction," because "an edit that changes no wording
still needs a regeneration to say so."  That was an acceptable trade when the
alternative was no check at all.  It is not an acceptable trade as a gate on a
phase whose whole subject is header edits.

So Step 0 revises the check rather than submitting to it, and the phase stops
being a queue behind one tool.  Steps 1 through 7 change headers freely; Step 8
regenerates the wording once, with specgen, after the API decisions have
settled -- which is when regeneration is informative anyway, since a fragment
regenerated three times tells a reviewer nothing the last regeneration does not.

The correctness check does not weaken.  `make wording-check` -- the real one,
which regenerates and diffs -- still has to pass before a paper revision goes
out, and Step 10 runs it.  What changes is that the *staleness* check stops
being a merge blocker for a tree that is knowingly mid-phase.

## Step index

| Step | Branch | Findings | Depends on |
| --- | --- | --- | --- |
| 0 | `p6-step0-wording-gate` | the gate above | — |
| 9 | `p6-step9-docs-build-ci` | D-05 – D-14 | — |
| 1 | `p6-step1-sniff-forward-range` | C-01 | Step 0 |
| 2 | `p6-step2-iconv-error-model` | C-02, C-03, C-05 | Step 0 |
| 3 | `p6-step3-iconv-lossy-skip` | C-04, S-02 | Steps 0, 2 |
| 4 | `p6-step4-null-term-precondition` | C-06 | Step 0 |
| 5 | `p6-step5-transcode-string-result` | C-07 | Step 0 |
| 6 | `p6-step6-utf32-validation` | S-01, C-08, D-15 | Step 0 |
| 7 | `p6-step7-fffd-reservation` | S-03 | Step 0 |
| 8 | `p6-step8-paper-and-wording` | P-01, P-02, S-04, S-05, D-01 – D-04 | Steps 1–7 |
| 10 | `p6-step10-verification` | open verification work | all |

Steps 0 and 9 touch no spec header and can start at once.  Steps 1, 4, 5, 6 and
7 touch disjoint headers and can run in any order or in parallel.  Steps 2 and 3
both rewrite `iconv_bulk.hpp`, so they are sequenced.  Step 8 is last of the
substantive steps because the paper describes what the others decided, and it
is the one step that needs specgen.

## Standing conventions

These are `CLAUDE.md`'s, repeated because a step document should not have to
send the reader back for them.

- **One branch and one worktree per step, rooted from `main`.**  Fetch and
  rebase before starting.  Do not work in a shared or already-dirty worktree.
- **TDD order: runtime test, negative compile test, `constify()` consteval
  test.**  A negative compile test's `PASS_REGULAR_EXPRESSION` must match the
  diagnostic that proves the intended constraint fired, not merely that
  compilation failed.
- **Includes are angle-bracketed and full-path; functions are defined
  out-of-line inside the header; include guards are `#ifndef`, never
  `#pragma once`.**
- **Every file carries `// SPDX-License-Identifier: Apache-2.0 WITH
  LLVM-exception`.**

## Verification

Per step, before the pull request:

```sh
make test            # ctest + pytest
make lint            # clang-format, gersemi, ruff, mypy, clang-tidy
```

`make coverage` for any step that adds a branch.  New code should have
coverage; template instantiations that cannot be reached are not worth fighting
the compiler over, but a *surprising* uncovered line gets a test or a note.

`make wording`, `make wording-check` and `make wording-validate` belong to
Step 8 and Step 10, not to every step.

Step 10 closes what the review left open: a fresh build across configurations
and both module settings, install-and-consume from a clean prefix, the examples
run against the changed headers, `make wording-validate` findings read rather
than counted, `codespell`, and the C-01 through C-05 regressions run against
glibc iconv **and at least one other implementation**.

## Risks

- **Deferring regeneration means the wording is knowingly stale for the length
  of the phase.**  That is the point of Step 0, and it is safe only because
  Step 8 and Step 10 both regenerate and diff.  If the phase is abandoned
  mid-way, `main` is left with fragments that do not match the headers -- so
  Step 8 is not optional, and a partial Phase 6 should be reverted rather than
  left.
- **Step 2 changes a published enumeration.**  `iconv_error` appears in the
  paper's Error Handling section as hand-written prose, not only as generated
  wording.  Step 8 owns that prose; Step 2 must not assume regeneration covers
  it.
- **Step 6 changes behaviour for thirty-nine codecs.**  It was scoped as a
  wording-only step and is not one: moving validation above codec dispatch
  changes which error `whatwg_encode_or_error` reports for a surrogate on every
  codec but UTF-8.  That is the fix, but it is a behavioural diff and wants
  the coverage to match.
