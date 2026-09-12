# P6-Step 0: Revise the Wording Staleness Gate

**Branch:** `p6-step0-wording-gate`
**Depends on:** nothing
**Read first:** `docs/plans/phase6-index.md`, `papers/wording/README.md`,
`Makefile` lines 220-273

---

## Goal

Stop `make wording-inputs-check` from being a merge blocker for drift that is
known, intentional and in progress, without giving up its guard against drift
that is none of those things.

## Why

The check hashes every spec-facing header and diffs against
`papers/wording/inputs.sha256`.  A header that moved without `make wording`
fails the pull request.  `make wording` needs a `specgen` built from
`papers/wording/specgen-ref`, and specgen links LLVM's Clang front end.

So the check's real cost is not the hash.  It is that changing one line of a
docblock obliges the contributor to build a compiler front end, and it obliges
this phase -- eight steps of header edits -- to serialize behind one tool.

`papers/wording/README.md` already states the flaw plainly: the check "over-
reports by construction," because "an edit that changes no wording still needs
a regeneration to say so."  That was the right trade when the alternative was
no check at all, and it is the wrong trade as a gate on a tree that is
knowingly mid-revision.

What the check is genuinely for is catching the header edit that nobody meant
to leave the wording behind.  That purpose survives an explicit opt-out; it
does not survive being switched off.

## Design

Add a pending list.  `papers/wording/PENDING` names the spec-facing headers
that are knowingly ahead of the committed fragments, one repo-relative path per
line, `#` comments allowed.  It is authored, not generated.

`make wording-inputs-check` then behaves as follows:

- a hash mismatch for a header **on** the list is reported and does not fail;
- a hash mismatch for a header **not** on the list fails exactly as it does
  today;
- a header on the list whose hash *matches* is reported as a stale entry, and
  fails, so the list cannot rot into a permanent exemption;
- an empty or absent `PENDING` reproduces today's behaviour byte for byte.

`make wording` clears the file when it regenerates -- the fragments are then
current, so nothing is pending.

`make wording-check`, the real check that regenerates and diffs, **ignores the
list entirely**.  A paper revision still cannot go out against stale wording.
That is what makes the opt-out safe: it defers a staleness report, it does not
defer correctness.

## What to change

- **`Makefile`** lines 231-254.  Read `PENDING`, partition the diff, and choose
  the exit code.  Keep the existing message for an unlisted header; it is a good
  message.  Add a distinct one for a listed header that says the entry must be
  removed by `make wording` before the paper goes out.
- **`papers/wording/generate.sh`**.  `--authored` is the one list of what lives
  in `papers/wording/` without being generated into it, and both `wording-check`
  and generate.sh's own cleanup read it.  `PENDING` is authored, so it goes in
  that list -- otherwise `wording-check` reports it missing from a scratch
  directory and calls the fragments stale.  Have `generate.sh` truncate
  `PENDING` on a real (non-`--out`) run.
- **`.github/workflows/test_makefile.yaml`**.  No change to the step; the
  target's exit code carries the decision.  Add a separate job, or a condition
  on the existing one, that fails when `PENDING` is non-empty on a tag or on a
  paper-release branch -- the point at which deferred is no longer acceptable.
- **`papers/wording/README.md`**.  Document the list under "The drift gate,"
  next to the paragraph that already explains why the check over-reports.  Say
  what puts a name on it and what takes one off.

## Tests

`tests/tools/` holds the Python tool tests that `make pytest` runs; the shell
plumbing here is better covered by a small set of cases driven from the same
place, or by a `bats`-style script if that is already the local habit.  Cover:

- empty/absent `PENDING` — behaves exactly as today, both pass and fail cases
- listed header changed — reports, exits zero
- unlisted header changed — fails, with the existing message
- listed header unchanged — fails as a stale entry
- `make wording` empties the list

## Done when

- The four cases above behave as described.
- `make wording-inputs-check` passes on an unmodified tree with an empty
  `PENDING`, and the hash file is unchanged by this step.
- `papers/wording/README.md` describes the list.
- No fragment under `papers/wording/` is edited by hand.

## Note on specgen

This step does not remove the need for specgen; it removes the need for it
*eight times*.  Step 8 still has to build it from `papers/wording/specgen-ref`
and regenerate.  Whoever picks up Step 8 should start that build early -- it is
the long pole -- and should confirm the tool reproduces the committed fragments
on an unmodified tree before trusting any diff it produces.
