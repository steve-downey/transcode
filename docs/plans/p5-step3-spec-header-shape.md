# P5-Step 3: Spec-Facing Header Shape

**Branch:** `p5-step3-spec-header-shape` (expect several sub-branches)
**Depends on:** Step 2 (the outline says what belongs where)
**Read first:** `docs/plans/phase5-index.md`, `~/src/specgen/main/docs/architecture.md` §1, §3.4, §3.5, §9

---

## Goal

Make each spec-facing header's set of main-file declarations equal to the
specification surface of the standard header it stands for, so that specgen's
synopsis *is* the synopsis and its coverage roster *is* the API.  This is the
refactoring the phase depends on and the only step that changes semantics-free
C++ at scale.

## Context for executing agent

specgen processes only declarations whose location is in the **main file**.
Anything reached through `#include` is invisible: it does not enter the
synopsis, the coverage roster, or the leakage report.  That single rule is the
lever.  Implementation that moves into `detail/` headers disappears from the
wording for free; implementation left in the spec-facing header must be
individually `\omit`ted or `\expos`ed, forever.

Three concrete problems in today's layout:

1. **Public entities live under `detail/`.**  `legacy_byte_range` and
   `unicode_scalar_range` (`detail/concepts.hpp`), `whatwg_error` and
   `iconv_error` (`detail/error.hpp`), `sniff_encoding` (`detail/sniff.hpp`),
   `transcode_string` (`detail/transcode_string.hpp`), `transcode` and
   `pluggable_transcode` (`detail/transcode_view.hpp`), `null_term_view` and
   `views::null_term` (`detail/null_term.hpp`) are all part of the proposed
   API but sit in the implementation directory.  The directory name is not the
   problem — the namespace is `beman::transcoding` for all of them, which is
   correct — but the file placement is what decides which specgen document they
   land in.
2. **Implementation lives in spec-facing headers.**  `whatwg_decode_view.hpp` is
   1,772 lines, of which the `namespace detail` block (the
   `random_access_decode_codec` concept, `random_access_decode_table()`, the
   table selection machinery) and the ~800-line `iterator::load()` codec state
   machines are implementation.  `whatwg_encode_view.hpp` (1,323 lines) has the
   same shape.  Every one of those is a declaration specgen must be told to
   ignore, and every `detail::` qualifier they leave in a spec-visible signature
   is a leakage error (index D7).
3. **The umbrella declares nothing.**  `transcode.hpp` is includes only, so
   there is no document whose root fragment is the `<transcode>` synopsis
   (index D1).

## Tasks

Work header family by header family; each is its own commit and can be its own
branch.  After each, `make test` and `make wording-check` must still pass.

1. **Promote the public entities out of `detail/`.**  New headers under
   `include/beman/transcode/`: `error.hpp`, `concepts.hpp`, `null_term.hpp`,
   `sniff.hpp`, `transcode_string.hpp`, `transcode_view.hpp`.  Move the
   declarations; leave the private helpers behind in `detail/`.  Update every
   `#include` (angle brackets, full path, per `CLAUDE.md`), the module interface
   `transcode.cppm`, and the CMake `FILE_SET` lists in
   `include/beman/transcode/CMakeLists.txt`.  No forwarding shims: this library
   is pre-release, and a shim is a second spelling to keep working.
2. **Thin the WHATWG views.**  Move `namespace detail` out of
   `whatwg_decode_view.hpp` and `whatwg_encode_view.hpp` into
   `detail/whatwg_codec_select.hpp` (or per-codec headers where one already
   exists), and push each codec's `load()` state machine down to the
   `detail::<codec>_decode_one` / `_encode_one` functions that `big5.hpp`,
   `gb18030.hpp`, `euc_jp.hpp` and friends already establish as the pattern.
   The iterator's `load()` should end up dispatching, not deciding.  This is the
   largest single piece of work in the phase; it is also a straightforward
   mechanical refactor with the existing test suite as the safety net, and it
   improves the headers independently of the paper.
3. **Remove `detail::` from spec-visible signatures.**  After (2), audit every
   remaining `detail::` in a class-scope or namespace-scope declaration of a
   spec-facing header.  Each is either promoted (it is really specification) or
   destined for `\expos` in Steps 4-9.  Record which, in the outline from
   Step 2; do not add the markup yet.
4. **Make the umbrella a real document.**  `transcode.hpp` becomes the
   spec-facing header for `<transcode>`: it carries the `\rSec2[transcode.syn]`
   gathered-synopsis region (architecture §3.4 — the region runs to a matching
   `/// END [transcode.syn]` fence) and the clause markers, with the family
   headers included beneath it.  Decide here, and record, whether the entities
   themselves move into `transcode.hpp` (D1's preferred shape: one document, one
   synopsis) or stay in family headers with a gathered synopsis assembled per
   family (D1's fallback).  Prefer the former; the measurement to make the call
   is how many lines the spec-facing surface is once (1) and (2) are done.
   Index U4 (a deduction guide corrupts a gathered `.syn` region) does not
   apply: `null_term_view`'s is the only deduction guide in the library, and it
   is in the other header.
5. **Keep the parse working.**  `transcode.hpp` must still parse standalone for
   specgen, which needs `-I .build/build-system/include` for
   `config_generated.hpp`.  Verify with the Step 1 script, not by eye.

## Outcome, and what is deferred

Tasks 1 and 3 are done and verified; task 2 is half done; task 4 is deferred,
for a reason task 2 measured.

- **Task 2, done:** the `namespace detail` blocks left both WHATWG headers
  (`detail/whatwg_decode_select.hpp`, `detail/whatwg_encode_select.hpp`), and
  `enum class codec` moved to its own specification header, `codec.hpp`, which
  broke the cycle that kept the selection machinery pinned next to the views.
- **Task 2, deferred:** the codec state machines are still inline.  Two
  `iterator::load()` bodies are 862 of `whatwg_decode_view.hpp`'s 1,645 lines
  (52%) and 505 of `whatwg_encode_view.hpp`'s 1,246 (41%).  Pushing them down
  to the `detail::<codec>_decode_one` functions the CJK codecs already use is
  about 1,370 lines of mechanical extraction.  It does **not** gate the wording:
  `load()` is a private member, private members are filtered out of the
  synopsis, and no clause extracts a private body.  It is worth doing for the
  headers' own sake, as its own step, and it is what makes task 4 viable.
- **Task 4, deferred pending that.**  D1 wants one document per proposed
  standard header so the root fragment is a real `<transcode>` synopsis.  Merged
  as the headers stand today, that file is about 5,000 lines, over half of it
  private state machines.  After the pushdown it is roughly 2,500 — the size
  specgen already handles (`beman.optional` is 2,145).  So the decision is: the
  merge is right, and it waits on the pushdown rather than being forced now.

## Acceptance

- `make test` green at every commit, including the negative compile tests.
- `make lint` green; no `#pragma once`, no quoted includes, no relative paths.
- The module build (`BEMAN_TRANSCODE_USE_MODULES=ON`) still configures and
  builds.
- `specgen generate --emit-ir include/beman/transcode/transcode.hpp ...` exits 0.
- `render --validate` on each spec-facing header reports **no `detail` leakage
  findings**.  Coverage findings are expected and are Steps 4-9's work; `ranges`
  findings are index U1's known noise.
- `docs/wording-outline.md` updated with the promoted/exposed/omitted decision
  for every entity.

## Notes

- Header moves are the kind of change that quietly breaks the benchmark tree and
  the installtest.  Grep `benchmark/`, `installtest/`, `tools/`, `tests/` for the
  old paths, not just `include/`.
- If (2) turns out to be more than a week of work, it can be deferred per codec:
  a codec whose `load()` is still inline costs one `\omit` marker and nothing
  else.  The refactor is worth doing for its own sake, but it is not on the
  critical path for any single clause.
