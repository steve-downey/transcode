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
docs/whatwg/           ← pristine upstream; never edited after download
docs/whatwg/SOURCE.md  ← human-readable provenance
docs/whatwg/source.bib ← BibTeX citation
data/tables/           ← our generated/derived artifacts (.hpp + .bin)
tools/                 ← Python scripts
tools/tests/           ← pytest tests for the scripts
```

None of these directories exist yet. Create them.

### Python quality requirements

The scripts are adjunct code but must be production quality:

- **Type annotations:** full hints on all functions, no `Any` escapes
- **Testing:** `tools/tests/` with pytest, covering parsing logic and
  known-mapping spot checks
- **Formatting + linting:** ruff (format + lint)
- **Type checking:** mypy in strict mode

**Changes to project config (do these first):**

1. Add to `pyproject.toml` `[dependency-groups] dev`:
   `"mypy>=1.15"`, `"pytest>=8.0"`, `"ruff>=0.11"`

2. Add `[tool.ruff]` and `[tool.mypy]` sections to `pyproject.toml`

3. Add ruff pre-commit hook to `.pre-commit-config.yaml`:
   ```yaml
     - repo: https://github.com/astral-sh/ruff-pre-commit
       rev: v0.11.13
       hooks:
       - id: ruff
         args: [--fix]
       - id: ruff-format
   ```

4. Run `uv sync` to install new deps

### What the scripts do

**`tools/download_indexes.py`** — typed Python script (stdlib only for
runtime deps). Downloads all WHATWG `index-*.txt` files (22 single-byte
+ 7 multi-byte) plus `encodings.json` into `docs/whatwg/`. Writes
`SOURCE.md` and `source.bib` with:
- Source URL for each file
- ISO-8601 download date
- SHA-256 checksums
- License (CC-BY 4.0 for data, BSD-3-Clause for code portions)
- Attribution to WHATWG Encoding Standard

**`tools/generate_tables.py`** — typed Python script (stdlib only for
runtime deps). Reads `docs/whatwg/index-*.txt`, generates for each
single-byte codec:
- A `.bin` file (128 × 4 bytes, little-endian uint32, 0 = unmapped)
- A `.hpp` file (constexpr array, same pattern as the hand-written
  `windows_1252.hpp` in `include/beman/transcode/detail/tables/`)

Output goes to `data/tables/`. Multi-byte generation is deferred.

### Python environment

- Python 3.13 available via `uv run python`
- Scripts use stdlib only for runtime (test/lint deps are dev-only)
- `curl` is available at `/usr/bin/curl` if needed
- Run tests: `uv run pytest tools/tests/`
- Type check: `uv run mypy tools/`

### Commit sequence

1. Branch: `git checkout -b step17-data-tooling`
2. Add ruff/mypy/pytest to pyproject.toml + pre-commit config → commit
3. Write `tools/download_indexes.py` + `tools/tests/test_download.py`
4. Run download script → commit `docs/whatwg/` + scripts + tests
5. Write `tools/generate_tables.py` + `tools/tests/test_generate.py`
6. Run generate script → commit `data/tables/` + scripts + tests
7. `uv run pytest tools/tests/` (pass) + `uv run mypy tools/` (clean)
8. `make test` (70 C++ tests still pass) + `make lint` (clean,
   now includes ruff)
9. Push both remotes
10. `git checkout main && git merge --no-ff step17-data-tooling`
11. Push main to both remotes
12. Update `docs/plans/phase2-checklist.md` — mark step 17 `[x]`

### Important details

- `make lint` runs `pre-commit run -a`. After adding the ruff hook,
  it checks BOTH C++/CMake AND Python formatting/linting.
- The generated `.hpp` files ARE C++ and WILL be checked by
  clang-format. Make sure they pass (proper indentation, line length).
- The hand-written `include/beman/transcode/detail/tables/windows_1252.hpp`
  stays in place for now; step 18 will replace it with the generated
  version.
- No `Co-Authored-By` trailers in commits.
- Scripts must pass `mypy --strict` — no untyped defs, no implicit
  optionals.

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
