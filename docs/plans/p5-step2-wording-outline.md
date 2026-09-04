# P5-Step 2: Wording Outline and Stable Names

**Branch:** `p5-step2-wording-outline`
**Depends on:** nothing (do it in parallel with Step 1)
**Read first:** `docs/plans/phase5-index.md`

---

## Goal

Decide the clause tree once, in a document, so that nine markup steps do not
each invent a stable name.  No code changes.

## Context for executing agent

specgen's `\rSec<depth>[stable.name]{Title}` markers are what build the document
tree, `--split` names one fragment per top-level section from its stable name,
and the mpark backend renders each stable name as a `{- .sref}` span.  The names
are therefore simultaneously the clause identifiers in the paper, the fragment
filenames, and the ordering key of the transclusion list.  Changing one later
renames a file and a heading and a cross-reference at once.

The names must also read like the working draft's.  Look at how
`[range.adaptors]`, `[text.encoding]` and `[char.traits]` are spelled before
inventing anything.

## Tasks

1. Write `docs/wording-outline.md` containing, for each proposed clause:
   stable name, title, depth, the header it is generated from, and the entities
   it covers.  Proposed starting point, to be argued with rather than accepted:

   | Stable name | Title | Covers |
   |---|---|---|
   | `transcode.general` | General | scope, terms |
   | `transcode.syn` | Header `<transcode>` synopsis | root fragment |
   | `transcode.errors` | Error types | `whatwg_error`, `iconv_error` |
   | `transcode.reqs` | Range requirements | `legacy_byte_range`, `unicode_scalar_range` |
   | `transcode.codec` | Encodings | `enum class codec`, `get_encoding`, `sniff_encoding` |
   | `transcode.whatwg.decode` | Class template `whatwg_decode_view` | the four decode views and closures |
   | `transcode.whatwg.encode` | Class template `whatwg_encode_view` | the four encode views and closures |
   | `transcode.codec.reqs` | Codec requirements | `decode_codec`, `encode_codec`, `flushable_decode_codec` |
   | `transcode.decode` | Class template `decode_view` | pluggable decode views |
   | `transcode.encode` | Class template `encode_view` | pluggable encode views |
   | `transcode.pipeline` | Transcoding pipelines | `transcode`, `pluggable_transcode` |
   | `transcode.string` | Eager transcoding | `transcode_string` |
   | `transcode.iconv` | iconv adaptors | the iconv views and bulk helpers |
   | `null.term` | Header `<null_term>` synopsis | `null_term_view`, `views::null_term` |

2. Fix the **fragment order**, which is the order the clauses appear in the
   paper.  It is the document order specgen prints in its manifest, which is the
   order of the `\rSec` markers in the header — so the outline is also a
   statement about where markers go in the source.
3. Decide the **depths**.  `\rSec2` for the clause, `\rSec3` for its subclauses
   is the natural fit for a paper whose own sections are `##`; note the
   interaction with `base_heading_level = 2` (index U2).
4. Record which entities are **exposition-only** (`\expos`) and which are
   `\omit`ted outright, per family.  This is the input Step 3 refactors toward
   and Steps 4-9 mark up; disagreeing about it later means editing headers
   twice.
5. Cross-check the outline against the paper's existing "API Surface
   Comparison" table: every ✅ in the WHATWG, pluggable and iconv columns must
   land in some clause, or be explicitly listed as not-proposed.

## Acceptance

- `docs/wording-outline.md` exists, is linked from `docs/plans/phase5-index.md`,
  and every row names a header and a set of entities.
- No entity in the API surface table is unaccounted for.
- `make lint` green.
