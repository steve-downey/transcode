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
make wording               # rewrite the fragments from the headers
make wording-check         # fail if the fragments are not what the headers generate
make wording-inputs-check  # fail if a header moved without a regeneration
```

Regenerating needs a `specgen` on `PATH`, built from the revision named in
`specgen-ref`. It also needs the build tree's generated `config_generated.hpp`,
so run `make compile` first in a fresh worktree, or point
`BEMAN_TRANSCODE_BUILD_INCLUDE` at a directory that has it.

## The drift gate, in two halves

specgen is not cheap to obtain in CI -- it links LLVM's Clang front end, so a
job that runs `wording-check` downloads LLVM and builds a tool from source on
every push. That is not a reasonable price for a check on a paper, so the gate
is split.

**`make wording-inputs-check` runs on every pull request**, as a step in the
Makefile workflow. It needs no specgen: it hashes the headers the wording is
generated from -- each document root plus the headers `#include`d inside its
gathered `.syn` region, which is exactly the document extent -- and compares
them against `inputs.sha256`, which `make wording` writes beside the fragments.
A header that changed without a regeneration fails.

What it cannot say is that the wording is *right*. It says the fragments were
generated from these headers and not from others, which is the staleness
question and not the correctness one. It also over-reports by construction: an
edit that changes no wording at all still needs a regeneration to say so.

**`make wording-check` is the real check**, and it runs on demand --
`.github/workflows/wording-drift.yml`, from the Actions tab. It regenerates
every fragment and diffs, then validates. Run it before a paper revision goes
out, and whenever `specgen-ref` changes. When a specgen becomes cheap to obtain
-- a released binary, a container -- putting the `pull_request` trigger back on
that workflow is the whole change.

`inputs.sha256` is generated. So is `wording.mk`. `README.md` and `specgen-ref`
are not, and both `wording-check` and `generate.sh`'s own cleanup know it.

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
