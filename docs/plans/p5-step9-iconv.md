# P5-Step 9: Markup — iconv Adaptors

**Branch:** `p5-step9-iconv`
**Depends on:** Step 8
**Read first:** `docs/plans/p5-step4-errors-concepts-null-term.md` ("the markup loop")

---

## Goal

Wording for `[transcode.iconv]` — or a recorded decision that the iconv
adaptors are described in the paper but not proposed as wording.

## What is different here

- **Decide the scope question first, and record it.**  The paper's "Headers"
  section currently lists `iconv_transcode` in `<transcode>`, but iconv is a
  POSIX interface, is not `constexpr`, and its behaviour is
  implementation-defined across glibc, musl and the BSDs — the paper says so
  itself.  Standardizing a view over it means specifying "whatever the
  implementation's iconv does", which is a legitimate but deliberate choice.
  The options are: full wording; wording conditioned on a
  `__cpp_lib_transcode_iconv` feature-test macro; or no wording, with the
  adaptors presented as implementation experience only.  This is a question for
  SG16, so state a recommendation in the paper rather than settling it silently
  in a header comment.
- **If wording is written**, the error model is `iconv_error`, not
  `whatwg_error`, and `docs/p2728-alignment.md` plus the standing memo on
  unifying the `_or_error` pairs under one error-kind enum are directly
  relevant: do not specify two error vocabularies without saying why in the
  design section.
- **Tests use `iconv_mock.hpp`**, so the wording cannot lean on any particular
  platform's tables; it can only say what the POSIX interface guarantees.

## Acceptance

- Either `papers/wording/transcode.iconv.md` exists and validates, or
  `docs/wording-outline.md` records the decision not to propose wording, with
  the reasoning, and the paper says so where the API surface table claims
  iconv support.
- `make wording-check`, `make lint`, `make test` green.

---

## Outcome (2026-09-08)

**iconv is proposed**, with full wording and no feature-test-macro condition.
`papers/wording/transcode.iconv.md` exists and validates, and with it the
document's findings reach **zero** -- every clause in the tree is generated,
and nothing in `<transcode>` is undescribed.

The scope question is settled in `docs/wording-outline.md` and argued in the
paper, in the design section and in "Questions for SG16".  The short of it: the
objection is that POSIX `iconv` is implementation-defined across glibc, musl
and the BSDs, so a view over it specifies whatever the platform does.  True,
and not disqualifying.  The wording specifies what the *adaptor* guarantees --
one conversion descriptor per `begin`, closed by the iterator that owns it, and
refused input either skipped or surfaced as an `iconv_error` -- and leaves the
conversion to the implementation, as `<locale>` leaves a locale's tables to it.
That guarantee is the reason to propose it: the C interface hands a program a
descriptor it must remember to close, and an adaptor is where that stops being
the program's problem.

**Two error vocabularies, on purpose.**  `iconv_error` is not merged into
`whatwg_error`, and the paper says why rather than leaving it to be noticed.
Step 3c unified the `_or_error` *views* on `transcode_error_kind`, which
selects whether errors are reported or replaced; what an error *is* remains
the codec family's question.  `whatwg_error` names a step in an algorithm this
paper specifies; `iconv_error` names what POSIX reported about a conversion
this library did not perform.  Nothing composes them either -- the iconv
adaptor is byte-to-byte and never appears in a `decode | encode` pipeline -- so
no expression has to reconcile the two.

### Four markup faults, all of them placement

The clause was mostly written when this step started; what was wrong was
where the markers sat.

- **`\omit` above the `struct`, not above the `template`.**  Three closures and
  `iconv_guard` carried `//! \omit` wedged *between* `template <typename
  IconvFns>` and `struct X {`, where it attaches to nothing and the whole type
  renders anyway.  A docblock goes above the entire declaration, template head
  included.  The same fault put a literal `//! \omit` line into the synopsis
  for `materialize_iconv_input`.
- **`\returns` on the declaration, not the definition.**  Step 5's lesson, hit
  twice more: `end()`'s wording sat in the class body.
- **No `\ref` group on `iconv_transcode_or_error_view`.**  Its constructor,
  `begin` and `end` fell through to `[transcode.iconv.iterator]` -- the clause
  that happened to be open -- because nothing routed them back.  A member's
  group is written beside its declaration or it inherits whatever precedes it.
- **Wording that named a private alias.**  `result_t operator*() const` named
  a member of an exposition-only iterator that no reader can resolve.  The
  alias is gone; the type is spelled.

### W1 reaches the borrowed-range specializations

`random_access_whatwg_decode_view` and its encode twin are not separately
specified entities, and neither are the `enable_borrowed_range` partial
specializations *written for them*.  Omitting the views without omitting their
specializations left two declarations in the synopsis naming views that are not
there.

### One specgen defect, the last one standing

An exposition-only use kept a bare `detail::` when its qualifier was written in
full -- which an `enable_borrowed_range` specialization, declared in
`namespace std::ranges`, has no way to avoid.  The same entity written
`detail::X` from inside the namespace rendered correctly, so one concept came
out two ways in one synopsis.  The head of `beman::transcoding::detail::` is
droppable and the whole of it is not, so the dropper stripped the head alone,
and that partial edit -- nested inside the range the exposition-only use wanted
deleted -- won the design 3.4 overlap watermark and suppressed the deletion it
sat inside.

specgen [#86](https://github.com/steve-downey/specgen/issues/86), fixed in
[PR #87](https://github.com/steve-downey/specgen/pull/87).  The first attempt
stopped the dropper descending into any non-droppable namespace qualifier,
which regressed `std::ranges::probe_t` to keep its `std::`; whether a prefix
should go is not a property of the qualifier but of what the name turned into,
so the filter belongs where the two edit sources meet.  **The wording committed
here needs that PR merged**: with the installed specgen, `[transcode.syn]`
still carries one `detail::` and one finding.
