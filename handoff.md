# Handoff: beman.transcode — Phase 2

## Project

`beman.transcode` — C++20 header-only library for Unicode transcoding using ranges and views.
Beman Project incubation for a C++29 standard proposal.

- GitHub: `https://github.com/steve-downey/transcode.git` (remote `origin`)
- Bloomberg: `bbgithub.dev.bloomberg.com:sdowney/transcode.git` (remote `bbgithub`)
- Working directory: `/home/sdowney/src/steve-downey/transcode/transcode`
- Two remotes; always push both: `git push origin <branch> && git push bbgithub <branch>`
- Merge to main: `git merge --no-ff`

## Current State

53 tests pass (`make test`). Steps 0–13 complete on `main`.

What exists: UTF-8 decoder (`whatwg_decode_view`), iconv transcoder (real + mock), pipe syntax for all views, error-reporting `_or_error` variants.

## What To Do Next

**Read the checklist first:**
```
docs/plans/phase2-checklist.md
```

It shows which steps are done and which remain. Then read the overview:
```
docs/plans/phase2-index.md
```

When starting a step, read ONLY its detailed plan:
```
docs/plans/step<N>-<slug>.md
```

Each step plan is self-contained — you do not need to read the other step files.

## TDD Process

Each step is a separate branch: `step<N>-<slug>`, branched from `main`.

1. Write failing tests (RED) → commit → push both remotes
2. Implement (GREEN) → `make test` (all pass) → `make lint` → commit → push both remotes
3. `git checkout main && git merge --no-ff step<N>-...` → push both remotes
4. Update `docs/plans/phase2-checklist.md` — mark completed items `[x]`

## Coding Rules (abbreviated)

- Include guards: `INCLUDE_BEMAN_TRANSCODE_*_HPP` (path-based, uppercase)
- Includes: angle brackets only, full path from include root
- Test files: include primary header twice (idempotent check)
- Functions: out-of-line in headers with full qualification
- `constexpr` everything that can be
- License: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- No `Co-Authored-By` trailers in commits
- Full rules in `CLAUDE.md`

## Build Commands

```bash
make test      # build + run all tests
make lint      # clang-format + gersemi + codespell + gitleaks
make compile   # build only
```
