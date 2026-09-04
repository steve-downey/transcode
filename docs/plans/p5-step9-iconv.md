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
