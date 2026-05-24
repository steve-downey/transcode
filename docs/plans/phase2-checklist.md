# Phase 2 Progress Checklist

Mark items `[x]` as they complete. Read this file first when resuming work.

**Current state:** Steps 0–27 complete. 363 C++ + 91 Python tests pass. On `main`.

---

## Step 14: `codec::replacement` (`step14-replacement-codec`)

- [x] Create branch `step14-replacement-codec` from `main`
- [x] Write failing tests (RED)
- [x] Commit RED: `"step14: replacement codec — tests (RED)"`
- [x] Push RED to both remotes
- [x] Implement `if constexpr` dispatch + replacement logic (GREEN)
- [x] `make test` — all pass
- [x] `make lint` — clean
- [x] Commit GREEN: `"step14: replacement codec — implement dispatch + replacement (GREEN)"`
- [x] Push GREEN to both remotes
- [x] Merge to main: `git checkout main && git merge --no-ff step14-replacement-codec`
- [x] Push main to both remotes

## Step 15: `codec::x_user_defined` (`step15-x-user-defined`)

- [x] Create branch `step15-x-user-defined` from `main`
- [x] Write failing tests (RED)
- [x] Commit RED + push both remotes
- [x] Implement `detail/x_user_defined.hpp` + dispatch (GREEN)
- [x] `make test` + `make lint`
- [x] Commit GREEN + push both remotes
- [x] Merge to main + push both remotes

## Step 16: Single-byte infra + `windows_1252` (`step16-single-byte-infra`)

- [x] Create branch `step16-single-byte-infra` from `main`
- [x] Write failing tests (RED)
- [x] Commit RED + push both remotes
- [x] Implement `detail/single_byte.hpp` + `detail/tables/windows_1252.hpp` + dispatch (GREEN)
- [x] `make test` + `make lint`
- [x] Commit GREEN + push both remotes
- [x] Merge to main + push both remotes

## Step 17: Data tooling (`step17-data-tooling`)

- [x] Create branch `step17-data-tooling` from `main`
- [x] Add ruff, mypy, pytest to pyproject.toml + pre-commit config
- [x] Write `tools/download_indexes.py` (typed, stdlib only)
- [x] Write `tools/tests/test_download.py`
- [x] Download all WHATWG index files → `docs/whatwg/` (pristine)
- [x] Write `docs/whatwg/SOURCE.md` (provenance, license, checksums)
- [x] Write `docs/whatwg/source.bib` (BibTeX citation)
- [x] Commit pristine upstream data + provenance + script + tests
- [x] Write `tools/generate_tables.py` (typed, stdlib only)
- [x] Write `tools/tests/test_generate.py`
- [x] Generate tables → `data/tables/`
- [x] Commit generated/derived data + script + tests
- [x] `uv run pytest tools/tests/` + `uv run mypy tools/`
- [x] `make test` + `make lint` (includes ruff)
- [x] Push both remotes + merge to main

## Step 18: All single-byte decoders (`step18-all-single-byte-decode`)

21 remaining single-byte codecs. Each needs: enum value, table include,
dispatch in both `load()` functions, tests.

- [x] Create branch from `main`
- [x] Add all 27 enum values + includes + dispatch branches
- [x] Replace hand-written `windows_1252.hpp` with WHATWG-correct generated version
- [x] Tests for representative codecs (RED → GREEN)
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

Tables placed in `include/beman/transcode/detail/tables/` (proper include
tree, not `data/tables/`). iso-8859-8-i shares iso-8859-8 table.

Codecs: IBM866, ISO-8859-2, ISO-8859-3, ISO-8859-4, ISO-8859-5,
ISO-8859-6, ISO-8859-7, ISO-8859-8, ISO-8859-8-I, ISO-8859-10,
ISO-8859-13, ISO-8859-14, ISO-8859-15, ISO-8859-16, KOI8-R, KOI8-U,
macintosh, windows-874, windows-1250, windows-1251, windows-1253,
windows-1254, windows-1255, windows-1256, windows-1257, windows-1258,
x-mac-cyrillic.

(ISO-8859-8-I shares ISO-8859-8's table; counted as one codec pair.)

## Step 19: Single-byte encoder infra + all encoders (`step19-single-byte-encode`)

- [x] Create branch from `main`
- [x] Write failing tests (RED) + push both remotes
- [x] Add `unmapped_codepoint` to `whatwg_error` enum
- [x] Add `unicode_scalar_range` concept to `detail/concepts.hpp`
- [x] Add `single_byte_encode_one()` to `detail/single_byte.hpp`
- [x] Create `whatwg_encode_view.hpp` (28 single-byte codec arms)
- [x] `make test` + `make lint` + `make coverage` (100% on new files)
- [x] Push both remotes + merge to main

## Step 20: UTF-8 encoder (`step20-utf8-encoder`)

- [x] Create branch from `main`
- [x] Write failing tests + negative compile test (RED)
- [x] Implement `detail/utf8_encode.hpp` + `whatwg_encode_view.hpp`
- [x] Add `unicode_scalar_range` concept
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 21: UTF-16 decode + encode (`step21-utf16`)

Algorithmic (no table). UTF-16BE and UTF-16LE, both directions.

- [x] Create branch from `main`
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode for both byte orders
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 22: GBK decode + encode (`step22-gbk`)

- [x] Create branch from `main`
- [x] Generate GBK tables (extend `generate_tables.py`)
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 23: gb18030 decode + encode (`step23-gb18030`)

- [x] Create branch from `main`
- [x] Generate gb18030 tables + ranges
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 24: Big5 decode + encode (`step24-big5`)

- [x] Create branch from `main`
- [x] Generate Big5 tables
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 25: Shift_JIS decode + encode (`step25-shift-jis`)

- [x] Create branch from `main`
- [x] Generate Shift_JIS tables
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 26: EUC-JP decode + encode (`step26-euc-jp`)

- [x] Create branch from `main`
- [x] Generate EUC-JP tables (JIS X 0208 + 0212)
- [x] Tests (RED → GREEN)
- [x] Implement decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 27: ISO-2022-JP decode + encode (`step27-iso-2022-jp`)

Stateful encoding (escape sequences).

- [x] Create branch from `main`
- [x] Tests (RED → GREEN)
- [x] Implement stateful decode + encode
- [x] `make test` + `make lint`
- [x] Push both remotes + merge to main

## Step 28: EUC-KR decode + encode (`step28-euc-kr`)

- [ ] Create branch from `main`
- [ ] Generate EUC-KR tables
- [ ] Tests (RED → GREEN)
- [ ] Implement decode + encode
- [ ] `make test` + `make lint`
- [ ] Push both remotes + merge to main

## Step 29: Round-trip composition (`step29-roundtrip`)

Comprehensive round-trip tests for all implemented codecs.

- [ ] Create branch from `main`
- [ ] Write composition tests for all codec pairs
- [ ] `make test` + `make lint`
- [ ] Push both remotes + merge to main

---

## Notes

- Each step's detailed plan: `docs/plans/step<N>-<slug>.md`
- Phase 2 overview: `docs/plans/phase2-index.md`
- TDD convention: RED = `"step<N>: <desc> (RED)"`, GREEN = `"step<N>: <desc> (GREEN)"`
- Push both remotes: `git push origin <branch> && git push bbgithub <branch>`
- No `Co-Authored-By` trailers in this project
- Pristine upstream: `docs/whatwg/` (WHATWG), `docs/wpt/` (WPT) — each
  with SOURCE.md (human-readable provenance) + source.bib (BibTeX)
- Derived/processed data: `data/` (tables, test vectors, etc.)
