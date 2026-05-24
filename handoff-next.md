# Handoff: beman.transcode — Step 17 (Data Tooling)

## Project

`beman.transcode` — C++20 header-only library for Unicode transcoding
using ranges and views. Beman Project incubation for a C++29 standard
proposal.

- GitHub: `https://github.com/steve-downey/transcode.git` (remote `origin`)
- Bloomberg: `bbgithub.dev.bloomberg.com:sdowney/transcode.git` (remote `bbgithub`)
- Working directory: `/home/sdowney/src/steve-downey/transcode/transcode`
- Two remotes; always push both: `git push origin <branch> && git push bbgithub <branch>`
- Merge to main: `git merge --no-ff`

## Current State

**70 tests pass** (`make test`). Steps 0–16 complete on `main`.

The single-byte table-driven decoder infrastructure exists
(`detail/single_byte.hpp` + one hand-written table `windows_1252.hpp`).
The task now is to build the tooling that generates tables for ALL
remaining codecs from the WHATWG normative data.

## What To Do Next — Step 17

**Read the detailed plan:**
```
docs/plans/step17-iso-8859-tables.md
```

This is a **tooling/data step**, not a library code step. There is
no RED/GREEN TDD cycle. The deliverables are:

1. Python scripts in `tools/`
2. Pristine WHATWG data in `docs/whatwg/` with provenance metadata
3. Generated table files in `data/tables/`
4. Existing tests still pass (`make test`), lint clean (`make lint`)

### Directory layout (create all of these)

```
docs/whatwg/          ← pristine upstream; never edited after download
docs/whatwg/SOURCE.md ← human-readable provenance
docs/whatwg/source.bib ← BibTeX citation
data/tables/          ← our generated/derived artifacts (.hpp + .bin)
tools/                ← Python scripts
```

None of these directories exist yet. Create them.

### What the scripts do

**`tools/download_indexes.py`** — downloads all WHATWG `index-*.txt`
files (22 single-byte + 7 multi-byte) plus `encodings.json` into
`docs/whatwg/`. Writes `SOURCE.md` and `source.bib` with:
- Source URL for each file
- ISO-8601 download date
- SHA-256 checksums
- License (CC-BY 4.0 for data, BSD-3-Clause for code portions)
- Attribution to WHATWG Encoding Standard

**`tools/generate_tables.py`** — reads `docs/whatwg/index-*.txt`,
generates for each single-byte codec:
- A `.bin` file (128 × 4 bytes, little-endian uint32, 0 = unmapped)
- A `.hpp` file (constexpr array, same pattern as the hand-written
  `windows_1252.hpp` in `include/beman/transcode/detail/tables/`)

Output goes to `data/tables/`. Multi-byte generation is deferred.

### Python environment

- Python 3.13 available via `uv run python`
- Scripts must use stdlib only (no pip dependencies)
- `curl` is available at `/usr/bin/curl` if needed

### Commit sequence

1. Branch: `git checkout -b step17-data-tooling`
2. Write + run `tools/download_indexes.py` → commit `docs/whatwg/` +
   `tools/download_indexes.py`
3. Write + run `tools/generate_tables.py` → commit `data/tables/` +
   `tools/generate_tables.py`
4. `make test` (70 tests still pass) + `make lint` (clean)
5. Push both remotes
6. `git checkout main && git merge --no-ff step17-data-tooling`
7. Push main to both remotes
8. Update `docs/plans/phase2-checklist.md` — mark step 17 `[x]`

### Important details

- `make lint` runs `pre-commit run -a`. It checks C++/CMake formatting
  but not Python. Still run it to confirm nothing broke.
- The generated `.hpp` files ARE C++ and WILL be checked by
  clang-format. Make sure they pass (proper indentation, line length).
- The hand-written `include/beman/transcode/detail/tables/windows_1252.hpp`
  stays in place for now; step 18 will replace it with the generated
  version.
- No `Co-Authored-By` trailers in commits.

## Coding Rules (abbreviated)

- License header on every file: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- Generated headers: add `// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py`
- Include guards: path-based, uppercase, `_` for `/` and `.`
- Namespace for tables: `beman::transcoding::detail::tables`
- Full rules in `CLAUDE.md`

## Build Commands

```bash
make test      # build + run all tests
make lint      # clang-format + gersemi + codespell + gitleaks
make compile   # build only
```

## After Step 17

Step 18 will use the generated tables to add all 21 remaining
single-byte decoders in one pass. The full plan is in
`docs/plans/phase2-checklist.md` and `docs/plans/phase2-index.md`.
