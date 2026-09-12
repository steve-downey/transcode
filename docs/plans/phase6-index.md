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
| S-01 | **Reopened — see the note below.**  The provisional decision was "keep the precondition, drop the promises."  It rests on a reading of which spec layer performs surrogate substitution, and that reading is being checked before Step 6 runs. |
| S-03 | **Document the reservation.**  U+FFFD is reserved as the unmapped-byte signal, the extensibility cost is stated, and the question of a better signal goes to SG16 rather than being answered here. |

C-07 is source-breaking: it changes a return type.  That is cheap now and
expensive after LEWG review, which is the argument for doing it in this phase
rather than the next.

### S-01 is reopened

The decision above was made on the reading that WHATWG's encoder algorithm
takes scalar values, so a surrogate is outside its domain and this library
should say so.  That reading may be wrong in a way that matters.

WHATWG specifies the encoder as taking a *code point* stream, and per Infra a
code point includes surrogates.  But `TextEncoder` is declared
`encode(optional USVString input)`, and Web IDL's DOMString-to-USVString
conversion replaces each lone surrogate with U+FFFD *before* the encoder
algorithm ever runs.  So the substitution is specified -- the question is
whether it is specified as part of the encoding facility or as coercion in the
JavaScript binding.

If it is part of the facility, then surrogate-to-U+FFFD is defined behaviour
this library should implement and specify, the WPT vectors in
`tests/beman/transcode/wpt_encoder_surrogates.test.cpp` are normative rather
than inapplicable, and it is the precondition that has to go.  That is the
opposite of the decision recorded above, and it would mean the implementation
was right all along and only the wording was wrong.

The cases may also split.  A value above U+10FFFF is not a Unicode code point
at all, so no conversion is defined for it in any layer -- and `char32_t` can
hold one where a JavaScript string cannot, so C++ faces an input the web
platform never has to answer for.  Surrogates and out-of-range values may
therefore need different answers, which would also settle whether
`whatwg_error::surrogate_code_point` and `out_of_range` are reachable on
encode at all.

This is being researched against the Encoding Standard, Infra, Web IDL, the
Unicode Standard and the WPT source before Step 6 runs.  Step 6 is on hold; no
other step depends on it.

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
| 6 | `p6-step6-scalar-precondition` | S-01 | Step 0 |
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
- **Step 6 removes tests.**  Deleting coverage to match a narrowed contract is
  correct here and looks like regression in a diff.  The commit message
  enumerates what went and why.
