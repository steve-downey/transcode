# Phase 4: API Surface Gap Closure

## Context

The API Surface Matrix in the README identifies gaps where the pluggable
codec and iconv backends lack operations that the WHATWG backend provides.
Custom codecs are second-class citizens (no encode views, no bulk ops), and
iconv has no bulk path (15-20x overhead vs raw iconv for "transcode this
string").

This phase closes those gaps so all three backends present matching named
interfaces.

## Step Index

| Step | Branch | Deliverable | Depends on |
|------|--------|-------------|-----------|
| 1 | `p4-step1-pluggable-encode` | Pluggable encode view + encode_or_error | — |
| 2 | `p4-step2-pluggable-bulk` | Bulk decode_to/encode_to/into for pluggable codecs | Step 1 |
| 3 | `p4-step3-pluggable-transcode` | transcode pipeline for pluggable codecs | Step 1 |
| 4 | `p4-step4-iconv-bulk` | iconv_transcode_to / iconv_transcode_into | — |
| 5 | `p4-step5-iconv-null-term` | null_term support for iconv views | — |
| 6 | `p4-step6-error-coherence` | Unify or document error enum naming | Steps 1-5 |

Steps 1-3 are the pluggable codec family (sequential dependency).
Steps 4-5 are the iconv family (independent of pluggable work).
Step 6 ties up naming coherence after all new APIs exist.

## Standing Conventions

Same as Phase 3 — see `docs/plans/phase3-handoff.md` for branch discipline,
coding rules, testing requirements, and commit conventions.
