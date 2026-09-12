# P6-Step 9: Documentation, Build and CI Drift

**Branch:** `p6-step9-docs-build-ci`
**Depends on:** nothing — start alongside Step 0
**Findings:** D-05 through D-13, plus D-14 (found while planning)
**Read first:** `docs/plans/phase6-index.md`

---

## Goal

Fix the things a new contributor trips over in the first hour, none of which
touch a spec-facing header.

## Why together

These are nine unrelated items.  They share a branch because each is small,
none needs a design decision, and none can conflict with the header work.
Splitting them into nine pull requests would cost more review than it saves.

Keep them as separate *commits*, though.  A single squashed "docs fixes"
commit is unbisectable, and D-12 in particular could break an install.

## The items

### D-05 — the build instructions name the wrong framework and the wrong standard

`README.md` and `CONTRIBUTING.md` say GoogleTest.  The tests are Catch2 3.x
(`tests/beman/transcode/CMakeLists.txt`), and every test file includes
`<catch2/catch_all.hpp>`.  `CONTRIBUTING.md` lines 39-47, 70-77 and 94-101 also
show manual configuration examples requesting C++17 or C++20; the target
requires C++23 (`CMakeLists.txt:111`, `:121`).

Replace GoogleTest with Catch2 and make every example request C++23.  Grep for
both; the review names three line ranges, and there may be more.

### D-06 — the compiler matrix disagrees with the configure check

`README.md:570-580` advertises Clang 17 and 18.  `CMakeLists.txt:22-29` refuses
anything below Clang 19, with this reason:

> Clang >= 19 is required (18 lacks `__cpp_concepts >= 202002L` needed by
> libstdc++ `std::expected`)

So the real constraint is a standard-library interaction, not a compiler
version -- Clang 18 against libc++ is not what the message is about.  The file
already prefers a feature probe over a version test for `ranges::to`
(`CMakeLists.txt:67-77`); do the same here.  Probe for what is actually needed,
then make the README report whatever the probe accepts rather than a version
number maintained by hand.

If the probe turns out to be awkward, keep the version check and fix only the
README -- but say in the message that the bound is about libstdc++, because the
current wording reads as though Clang 18 is broken.

### D-07 — `table_codec` is documented as public API and is not

`README.md:165-186` presents `beman::transcoding::table_codec` as provided
public API and links `examples/custom_single_byte_decoder.cpp`.  The type lives
under `detail/`, and `transcode.hpp:56-61` says so explicitly:

> The single-byte views, the generated label table and the table-driven codec
> engine are not proposed (`docs/wording-outline.md`, "Not proposed at all"),
> and being outside the region is how they say so.

The linked example does not use `table_codec` either -- it decodes bytes by
hand.  So the README documents a type the proposal excludes, and points at an
example that demonstrates neither.

Remove the public claim.  Then make the example earn its name: rewrite
`examples/custom_single_byte_decoder.cpp` to implement
`random_access_decode_codec_type` and run it through `decode_view`, which is
the extension point the proposal *does* offer.  An example titled "custom
single byte decoder" that uses no library API is the part of this worth fixing
properly rather than deleting.

### D-08 — ICU does not expose the iconv API

`README.md:188-195` groups ICU with implementations exposing
`iconv_open`/`iconv`/`iconv_close`.  ICU's converter API is the `ucnv_*` family.
Remove it from the list, or describe it as a separate backend that would need
an adapter.

### D-09 — a tracked file named `-`

`./-` is tracked and holds an old `::: wording` synopsis snapshot.  It came in
with `94a32d2c` ("Print the enumerator meanings as tables, now that specgen
can") -- a redirection that captured stdout into a file whose name was meant to
be a dash argument.

`git rm -- ./-`.  Then find the invocation: check `papers/wording/generate.sh`
and the `wording*` targets in the `Makefile` for a `>` or a `-o` whose argument
can go missing.  A file that appeared once will appear again.

Add a hygiene check.  `.pre-commit-config.yaml` is the cheap place: a small
`check-added-large-files`-style local hook, or `check-illegal-windows-names`
style, that rejects a repository-root file whose name is not a plausible
filename.

### D-10 — `docs/plans/handoff-next.md` reads as current

It reports 707 C++ tests (711 register now), a paper update that has since
happened, and merge instructions for a branch that is gone.  It describes an
earlier state and does not say so.

Give it a dated header saying what it was and when it was true, or delete it.
Prefer the header: the phase documents around it are all historical too, and
consistency is worth more than one fewer file.

### D-11 — "Manual Lint" runs the ordinary lint

`.github/workflows/test_makefile.yaml:86-95` runs `make lint`, which the
preceding `Lint` step already ran.  `make lint-manual` exists — `Makefile:281`,
"Run all manual tools in pre-commit".  Call it.

### D-12 — module packaging is untested and the interface unit has no licence

Two things:

`include/beman/transcode/transcode.cppm` has no
`// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.  Every other file
does.  Add it.

And in module mode, `include/beman/transcode/CMakeLists.txt:3-15` installs five
files: `codec.hpp`, `config.hpp`, `transcode.hpp`, `todo.hpp` and the generated
`config_generated.hpp`.  But `transcode.cppm` does:

```cpp
#define BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT
export {
#include <beman/transcode/transcode.hpp>
}
```

and with that macro defined, `transcode.hpp` takes the `#else` branch and
includes the whole header graph — roughly seventy files, none of them
installed.  A consumer that only *imports* the built module is fine; a consumer
whose build system rebuilds the interface unit from the installed
`FILE_SET CXX_MODULES` is not.

Test it before fixing it, so the fix is aimed: configure with
`BEMAN_TRANSCODE_USE_MODULES=ON`, install to a clean prefix, and build a small
consumer against it.  If the interface unit is rebuilt, install every header
its include graph needs — which is the non-module list already in the `else`
branch, so the two branches converge.

### D-14 — the gitleaks hook is broken (not in the review)

Found while linting this phase's own documents.  `pre-commit run gitleaks`
fails on any file, including unmodified ones:

```text
Error: unknown command "git" for "gitleaks"
```

The pinned hook invokes a `gitleaks git` subcommand that the resolved binary
does not have — a version drift between `.pre-commit-config.yaml` and what
gitleaks now ships.  So `make lint` has a permanently failing hook, which
teaches everyone to read past a red result.  That is worse than not having the
hook.

Pin the `rev` to a version whose CLI matches the invocation, or update the
invocation.  Check whether CI hits this too; if CI passes, the two are running
different versions and that is the thing to fix.

### D-13 — shebang on line 2

`infra/cmake/telemetry.sh` has the SPDX comment first and `#!/usr/bin/env bash`
second, which shellcheck flags and which would matter if anything ever executed
it directly.  Shebang first, SPDX second.

## Repository hygiene

Also on this branch:

- Decide about `papers/baseline-check.md` — an untracked D4246R0 snapshot of
  the paper, apparently kept as a rendering baseline.  Either commit it with a
  line saying what it is for, or delete it.  An untracked near-copy of the
  paper is a trap for anyone grepping.
- Prune the four stale worktrees (`add-clang-tidy`,
  `codex/gb18030-e5e5-conformance`, `codex/remove-utf16-encode-paths`,
  `codex/scalar-value-precondition`).  All four are ancestors of `main`.

## Done when

- Every item above is a separate commit with its finding ID in the subject.
- `make test`, `make lint` and `make lint-manual` pass.
- The module install-and-consume test exists and passes, or D-12 is documented
  as still open with the test that demonstrates it.
