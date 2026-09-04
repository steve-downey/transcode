# P5-Step 1: specgen Harness and Paper Transclusion

**Branch:** `p5-step1-specgen-harness`
**Depends on:** nothing
**Read first:** `docs/plans/phase5-index.md`, `~/src/specgen/main/docs/user-guide.md`

---

## Goal

Stand up the whole generate → commit → transclude → check loop, and prove it end
to end on one small header, before any wording markup exists.  After this step,
adding wording is only ever "write markup, run `make wording`".

## Context for executing agent

`papers/` is a flat mpark/wg21 layout: `papers/transcode-view.md` beside the
vendored `papers/wg21/` subtree, outputs in `papers/generated/`.
`papers/Makefile` is two lines of TLS environment plus `include wg21/flat.mk`.

Two facts about that framework drive the design:

- `base.mk` builds its pandoc command as
  `pandoc $(DATADIR)/srefs.defs $(filter %.md, $^) -o $@ ...`.  **Every markdown
  prerequisite is an input, concatenated in prerequisite order.**  That is the
  transclusion mechanism (index D3); nothing else is needed.
- `flat.mk` sets `src := $(wildcard *.md)` (less CHANGELOG/LICENSE/README) and
  makes a paper target of each.
  Generated fragments must therefore live in a subdirectory (index D4).

## Tasks

1. **`papers/wording/generate.sh`** — the single pinned invocation (index,
   standing conventions).  It must:
   - default `SPECGEN` to `specgen` and allow an override;
   - export `LD_LIBRARY_PATH=/home/sdowney/install/gcc-16/lib64:...`;
   - take the header list and the parse tail from one place:
     `-std=c++2c --gcc-toolchain=/home/sdowney/install/gcc-16 -I include
     -I .build/build-system/include` with `--no-compile-commands`;
   - run `generate --emit-ir` per spec-facing header, then `render --from-ir`
     with `--backend mpark --split papers/wording --root transcode.syn`;
   - write the printed manifest to `papers/wording/wording.mk` as a make
     variable in document order, e.g.
     `WORDING_MD := wording/transcode.syn.md wording/transcode.errors.md ...`;
   - fail loudly if specgen or the Clang runtime is missing, rather than
     silently emitting nothing.
   Regeneration must be idempotent: two runs produce byte-identical files.
   `--split` does not delete stale files, so the script removes the previous
   `*.md` and `wording.mk` first and rewrites them from the manifest.
2. **`make wording` and `make wording-check`** in the top-level `Makefile`,
   beside `make lint` / `make pytest`.  `wording` runs the script; `wording-check`
   runs it into a scratch directory and `diff -r`s against `papers/wording/`,
   exiting non-zero on any difference.  `wording-check` is what CI runs and what
   every later step runs before committing.
3. **Paper transclusion** in `papers/Makefile`.  After the `flat.mk` include,
   `-include wording/wording.mk` and an explicit rule per output that overrides
   the pattern rule so prerequisite order is ours and not make's:

   ```make
   $(OUTDIR)/transcode-view.html $(OUTDIR)/transcode-view.latex $(OUTDIR)/transcode-view.pdf: \
       $(OUTDIR)/transcode-view.%: transcode-view.md $(WORDING_MD) $(DEPS) | $(OUTDIR)
	   $(PANDOC)
   ```

   Do not rely on make appending explicit prerequisites to an implicit rule's;
   write the recipe.
4. **Prove it.**  Add a `//! \effects`-level docblock and a `\rSec2` section
   marker to `detail/null_term.hpp` — the smallest spec-facing header, which
   already generates a clean class synopsis — enough to produce one real
   fragment.  Build `make -C papers transcode-view.html` and confirm the
   generated clause appears in the output with its `[stable.name]{- .sref}`
   heading intact and no pandoc warning about the sref.
5. **Bibliography placement.**  Fragments are concatenated after
   `transcode-view.md`, and citeproc places the bibliography at the end of the
   document.  Confirm where it lands and, if it lands after the wording, add an
   explicit `::: {#refs}` div at the intended place in `transcode-view.md`.
   Record the answer in the step's commit message; Step 10 depends on it.
6. **`.gitignore` / lint.**  `papers/wording/*.md` is committed (index D2) but
   is generated: exclude it from markdown lint rules that would fight specgen's
   output, and say so in `papers/wording/README.md` (one paragraph: generated,
   do not edit, regenerate with `make wording`).

## Acceptance

- `make wording` twice in a row leaves the tree clean.
- `make wording-check` exits 0 on a clean tree and non-zero after touching a
  fragment by hand.
- `make -C papers transcode-view.pdf` builds and contains the proved clause.
- `make lint` and `make test` green.
- No specgen invocation anywhere but `papers/wording/generate.sh`.

## Notes

- The `-I .build/build-system/include` entry is for `config_generated.hpp`,
  which CMake writes into the build tree.  A fresh clone must therefore
  `make compile` once before `make wording` works; make that a documented
  prerequisite of the target rather than a surprise.
- Keep the fragment directory flat.  `--split` names files from stable names,
  so `transcode.whatwg.decode.md` is a filename, not a path.
