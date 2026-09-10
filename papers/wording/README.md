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

## The drift gate

specgen is not available in CI at a reasonable price. It links LLVM's Clang
front end, so a job that runs `wording-check` downloads LLVM and builds a tool
this repository does not otherwise depend on, on every push. A workflow that
did exactly that was written, run, and removed; what it taught is below.

**`make wording-inputs-check` runs on every pull request**, as a step in the
Makefile workflow. It needs no specgen: it hashes the headers the wording is
generated from -- each document root plus the headers `#include`d inside its
gathered `.syn` region, which is exactly the document extent -- and compares
them against `inputs.sha256`, which `make wording` writes beside the fragments.
A header that changed without a regeneration fails.

What it cannot say is that the wording is *right*. It says the fragments were
generated from these headers and not from others, which is the staleness
question and not the correctness one. It over-reports by construction, too: an
edit that changes no wording still needs a regeneration to say so. That is the
trade for a check that costs nothing.

**`make wording-check` is the real check, and it is local.** Run it before a
paper revision goes out, and whenever `specgen-ref` changes. It regenerates
every fragment and diffs; `papers/wording/generate.sh --validate` then answers
the other question, whether a clause describes an entity the reader cannot see.

### If you try to put it back in CI

Two things sank it, and both are worth knowing before the next attempt.

**The parse fails.** specgen builds and runs, then dies on
`/usr/include/wchar.h: fatal error: 'stddef.h' file not found`. A specgen built
against an unpacked LLVM tarball does not find Clang's own builtin headers: the
resource directory is derived from the running binary's path, not from the LLVM
it links. The parse tail needs `-resource-dir` pointing into that install, or
`SPECGEN_GCC_TOOLCHAIN`, or both. It works on a developer machine because Clang
finds the system GCC by itself.

**CodeQL rejects the shape.** Checking out a second repository at a ref read
from a file, building it, and running it, in a job that can write the Actions
cache, is `actions/cache-poisoning/poisonable-step` -- three high-severity
alerts, and correctly so: a specgen revision is code, and this repository would
be executing it with default-branch privileges.

A released specgen binary, or a container image with one in it, avoids both at
once. That is the thing to wait for.

## What is committed and why

The fragments are committed rather than built on demand so that the paper
builds without specgen, and so that a change to the wording shows up as a diff
in a pull request instead of only as a different PDF. `wording.mk` is generated
too: it lists the fragments in document order, which is the order
`papers/Makefile` hands them to pandoc, which is the order they appear in the
paper.

`inputs.sha256` is generated too. `README.md` and `specgen-ref` are not:
`generate.sh --authored` is the one list saying which is which, and both its own
cleanup and `wording-check`'s diff read it, so a third authored file breaks
neither.

`generate.sh` also strips mpark's `.sref` class from stable names under this
paper's own roots. Those clauses are not in the working draft, so `.sref` warns
once each at paper-build time and links every cross-reference to a `c++draft`
page that does not exist. That is
[specgen#94](https://github.com/steve-downey/specgen/issues/94): the flag that
does this upstream, `render --new-root`, landed for
[#89](https://github.com/steve-downey/specgen/issues/89) but takes only one
name, and this paper proposes two headers whose documents cross-reference each
other. The strip comes out when the flag can be given twice.

## Adding a clause

Mark up the header, add a `\rSec` marker to `transcode.hpp` (or
`null_term.hpp`) where the clause belongs in the tree, run `make wording`, and
commit what changes. `docs/wording-outline.md` is the map of which clause
specifies what; keep it in step.
