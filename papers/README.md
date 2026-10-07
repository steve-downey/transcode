<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Papers

`transcode-view.md` is the mpark/wg21 source for D4246R1, "Transcoding Text
Views". The wg21org conversion is `wg21org/transcode-view.org`; its generated
wording is included from `wg21org/wording/wording.org`.

Build the wg21org paper with:

```sh
make -C papers/wg21org transcode-view.html transcode-view.pdf
```

The generated wording is committed, so those builds do not require specgen.
To regenerate it, build a specgen checkout with GCC 16 (the system compiler is
too old), then name that binary and the GCC installation used by Clang:

```sh
specgen_wg21org=~/src/steve-downey/specgen/wg21org-transcode
make -C "$specgen_wg21org" \
  TOOLCHAIN=gcc-16 CONFIG=RelWithDebInfo compile

SPECGEN="$specgen_wg21org/.build/build-gcc-16/tools/specgen/RelWithDebInfo/specgen" \
SPECGEN_GCC_TOOLCHAIN=~/install/gcc-16 \
BEMAN_TRANSCODE_BUILD_INCLUDE="$PWD/.build/build-gcc-16/include" \
make -C papers/wg21org transcode-wording
```

Use the same variables with `transcode-wording-check` to regenerate into a
temporary directory and verify that the committed Org fragment is current.

The original mpark/wg21 paper is built by the vendored
[mpark/wg21](https://github.com/mpark/wg21) framework in `wg21/`, in that
framework's flat layout: sources here, output under `generated/`.

```sh
make -C papers transcode-view.pdf    # or .html, .latex
```

Refresh the framework with:

```sh
git subtree pull --prefix=papers/wg21 https://github.com/mpark/wg21.git master --squash
```

## `wording/` is generated

The paper's wording clauses are **not written here**. They are generated from
the specgen markup in `include/beman/transcode/` by `wording/generate.sh`, which
writes one mpark/wg21 fragment per clause plus `wording/wording.mk` listing them
in document order. `papers/Makefile` names that list as ordered prerequisites of
the paper, and pandoc concatenates the paper and its wording in that order.

From the repository root:

```sh
make wording           # regenerate the fragments
make wording-check     # fail if the committed fragments are stale
make wording-validate  # report specgen's validation findings per header
```

Edit the markup in the headers, never the fragments. `make wording-check` exists
to make that stick. `wording/generate.sh` needs `beman.specgen` on the `PATH`
(`SPECGEN=<path>` overrides); building the paper does not, which is why the
fragments are committed.

See `docs/plans/phase5-index.md` for the plan this belongs to.
