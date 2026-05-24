# Phase 2 Progress Checklist

Mark items `[x]` as they complete. Read this file first when resuming work.

**Current state:** Steps 0–16 complete. 70 tests pass. On `main`.

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

## Step 17: ISO-8859 tables (`step17-iso-8859-tables`)

- [ ] Create branch `step17-iso-8859-tables` from `main`
- [ ] Write failing tests (RED)
- [ ] Commit RED + push both remotes
- [ ] Create table headers + dispatch (GREEN)
- [ ] `make test` + `make lint`
- [ ] Commit GREEN + push both remotes
- [ ] Merge to main + push both remotes

## Step 18: UTF-8 encoder (`step18-utf8-encoder`)

- [ ] Create branch `step18-utf8-encoder` from `main`
- [ ] Write failing tests + negative compile test (RED)
- [ ] Commit RED + push both remotes
- [ ] Implement `detail/utf8_encode.hpp` + `whatwg_encode_view.hpp` + `unicode_scalar_range` concept (GREEN)
- [ ] `make test` + `make lint`
- [ ] Commit GREEN + push both remotes
- [ ] Merge to main + push both remotes

## Step 19: Round-trip composition (`step19-roundtrip`)

- [ ] Create branch `step19-roundtrip` from `main`
- [ ] Write composition tests (RED — depends on step 18 encoder existing)
- [ ] Commit RED + push both remotes
- [ ] Tests should pass immediately if encoder is correct (GREEN = no new library code)
- [ ] `make test` + `make lint`
- [ ] Commit GREEN + push both remotes
- [ ] Merge to main + push both remotes

---

## Notes

- Each step's detailed plan: `docs/plans/step<N>-<slug>.md`
- Phase 2 overview: `docs/plans/phase2-index.md`
- TDD convention: RED commit msg = `"step<N>: <desc> (RED)"`, GREEN = `"step<N>: <desc> (GREEN)"`
- Push both remotes: `git push origin <branch> && git push bbgithub <branch>`
- No `Co-Authored-By` trailers in this project
