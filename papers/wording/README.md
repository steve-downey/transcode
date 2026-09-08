# Generated wording fragments

**Every `.md` file in this directory is generated. Do not edit them.**

They are the Wording section of `papers/transcode-view.md`, rendered from the
marked-up headers in `include/beman/transcode/` by
[`beman.specgen`](https://github.com/steve-downey/specgen). The wording is not
maintained alongside the code; it is produced from it, which is the point. A
sentence about what `whatwg_decode_view::begin` returns lives in a `//!`
docblock beside the definition, and reaches the paper from there.

## Regenerating

```sh
make wording          # rewrite the fragments from the headers
make wording-check    # fail if the committed fragments are not what the headers generate
```

`make wording-check` is the drift gate, and it runs in CI
(`.github/workflows/wording-drift.yml`). It fails on a header edited without
regenerating, and on a fragment edited by hand -- both are the same diff to it.

Regenerating needs a `specgen` on `PATH`, built from the revision named in
`specgen-ref`. It also needs the build tree's generated `config_generated.hpp`,
so run `make compile` first in a fresh worktree, or point
`BEMAN_TRANSCODE_BUILD_INCLUDE` at a directory that has it.

## What is committed and why

The fragments are committed rather than built on demand so that the paper
builds without specgen, and so that a change to the wording shows up as a diff
in a pull request instead of only as a different PDF. `wording.mk` is generated
too: it lists the fragments in document order, which is the order
`papers/Makefile` hands them to pandoc, which is the order they appear in the
paper.

`generate.sh` also strips mpark's `.sref` class from stable names under this
paper's own roots. Those clauses are not in the working draft, so `.sref` warns
once each at paper-build time and links every cross-reference to a `c++draft`
page that does not exist. That is
[specgen#89](https://github.com/steve-downey/specgen/issues/89), and the strip
comes out when it lands.

## Adding a clause

Mark up the header, add a `\rSec` marker to `transcode.hpp` (or
`null_term.hpp`) where the clause belongs in the tree, run `make wording`, and
commit what changes. `docs/wording-outline.md` is the map of which clause
specifies what; keep it in step.
