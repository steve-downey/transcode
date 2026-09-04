# P5-Step 3b: Push the Codec State Machines Down

**Branch:** `p5-step3b-codec-pushdown`
**Depends on:** Step 3
**Read first:** `docs/plans/phase5-index.md`, `docs/plans/p5-step3-spec-header-shape.md`

---

## Goal

Take the codec implementation out of the two WHATWG view headers, so that the
specification headers are thin enough for D1's one-document-per-proposed-header
shape (Step 3 task 4), and so that the headers read like specification rather
than like a decoder.

## What was done

The per-table branch collapse.  Each of the four `iterator::load()` bodies
dispatched on the codec with 27 near-identical branches — one per single-byte
table, differing only in which table they named.  The selection function that
turns them into one branch already existed and simply was not used here.  With
`detail::single_byte_table_codec` and a reference-returning
`detail::single_byte_decode_table()`, all four collapse to a single branch:

- `whatwg_decode_view.hpp` 1,645 → 1,416 lines
- `whatwg_encode_view.hpp` 1,246 → 1,141 lines

The collapse was verified by matching branch bodies modulo the table name rather
than by eye, which is how the one non-uniform branch was found: in
`whatwg_decode_or_error_view`'s `load()`, `windows_1252` calls
`std::unreachable()` on a decode error, because the windows-1252 table has no
holes and a byte can never fail to map.  It is kept verbatim ahead of the
general branch.  Folding it in would replace undefined behaviour with an error
return — defensible, but a semantic decision rather than a mechanical one, so it
is left for whoever wants to make it deliberately.

## What was not done, and the order to do it in

The stateful codecs — ISO-2022-JP (156 lines per `load()`), UTF-16 (57), and
GB18030's replay buffer (51) — are still inline.  Extracting them means giving
each a decoder object that owns the state the iterator currently holds
(`iso2022jp_state_`, `iso2022jp_output_state_`, `iso2022jp_output_flag_`,
`iso2022jp_lead_`, `pending_`, `pending_count_`, `gb_replay_*`), and teaching
the iterator's `operator==` to compare it.  That is real work, and it is worth
doing.

**But not before the `_or_error` unification.**  `whatwg_decode_view`'s `load()`
and `whatwg_decode_or_error_view`'s `load()` are 306 and 328 lines, and **71% of
their lines are identical**; they differ only in whether a failure becomes
`U+FFFD` or `unexpected(error)`.  `docs/p2728-alignment.md` has an open action
item to unify each view with its `_or_error` twin under one
error-kind-parameterized template, which deletes one of the two copies outright.
Extracting the state machines first means extracting them twice and then
throwing half the work away.

So the order is: unify the `_or_error` pairs, then extract the state machines
from the one remaining `load()` per direction.  The unification is also what
Steps 6 and 7 need settled before they write those clauses
(`docs/wording-outline.md`, "Open, and deliberately not settled here"), so it is
on the critical path either way.

## What this means for Step 3 task 4

Nothing is blocked.  The merge into one document per proposed header was already
recorded as waiting on the pushdown, and the pushdown now waits on the
unification.  None of it gates the markup steps: `load()` is a private member,
private members are filtered out of every synopsis, and no clause extracts a
private body.  Steps 4 through 9 can proceed against the headers as they stand.

## Acceptance

- `make test` green (711 of 711 at the time of writing).
- `make wording-check` clean.
- No behavioural change: the collapse is body-identical per branch, and the one
  branch that was not identical is preserved verbatim.
