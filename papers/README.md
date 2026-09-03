<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Papers

`transcode-view.md` is the WG21 paper (D4246R0, "Transcoding Text Views"). It is
built by the vendored [mpark/wg21](https://github.com/mpark/wg21) framework in
`wg21/`, in that framework's flat layout: sources here, output under
`generated/`.

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
