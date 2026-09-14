# P6-Step 8: The Paper, and One Regeneration

**Branch:** `p6-step8-paper-and-wording`
**Depends on:** Steps 1 through 7
**Findings:** P-01, P-02, S-04, S-05, D-01, D-02, D-03, D-04
**Read first:** `docs/plans/phase6-index.md`, `papers/wording/README.md`,
`docs/wording-outline.md`

---

## Goal

Bring D4246R1 into line with what Steps 1-7 decided, resolve the POSIX-in-an-
ISO-synopsis problem, and regenerate the wording once, at the end, when
regenerating means something.

## Before anything else: build specgen

`make wording` needs a `specgen` built from the revision named in
`papers/wording/specgen-ref` (currently the branch `main`).  It links LLVM's
Clang front end, so this is the long pole -- start it first.

Then, **before touching a header or a fragment**, prove the tool on the tree as
it stands: `make wording-check` must pass.  If it does not, the local specgen
differs from the one that produced the committed fragments, and every diff this
step produces would mix wording change with tool drift.  Resolve that first;
`papers/wording/README.md` has the two failure modes that are already known
(`-resource-dir`, `SPECGEN_GCC_TOOLCHAIN`).

Steps 1-7 will have left entries in `papers/wording/PENDING` (see Step 0).
`make wording` clears it.  The file must be empty when this step ends.

## P-02: an ISO synopsis that names a POSIX type

`iconv_transcode_view.hpp:38-41` declares:

```cpp
struct iconv_functions {
    iconv_t (*open)(const char* tocode, const char* fromcode);
    ...
};
```

and `transcode.hpp:49-52` includes that header *inside* the gathered
`\rSec2[transcode.syn]` region, so `iconv_t` reaches
`papers/wording/transcode.syn.md` as a declaration of the proposed
`<transcode>`.  `iconv_t` is not an ISO C++ type and `<iconv.h>` is not an ISO
C++ header, and the proposal does not say how the name exists on a conforming
implementation that is not POSIX.

**Decision (see the phase index): document it as implementation-defined.**  No
header split, no CMake option, no new wording root.

- Wording on `iconv_functions` saying its handle type is implementation-defined
  and names the type the implementation's `iconv` conversion descriptors have;
  on POSIX that is `iconv_t`.
- Wording on `[transcode.iconv]` as a whole saying the clause is provided only
  where the implementation supplies such a facility.
- Do **not** leave the reader to infer this from the absence of a definition.
  The whole finding is that a reviewer opening the synopsis meets an undefined
  name; the fix is that they meet a defined term instead.

Then sharpen the ask.  The paper's "Questions for SG16" already says the author
recommends keeping the adaptor in this paper through design review and
splitting the wording only if the group wants separate progression.  That is
the right position; it is currently phrased as an aside.  Make it an explicit
question, and give the group the fact that motivates it -- that the adaptor's
handle type cannot be specified in ISO terms without either an opaque handle or
a separate POSIX-conditional document.

## P-01: the optional-iconv story

Three places say iconv is optional or absent, and the build says otherwise:

- `README.md:3` — "header-only"; the library links `Iconv::Iconv`.
- `README.md` — the MSVC portability claim, which holds only if a compatible
  iconv is supplied separately.
- `papers/transcode-view.md:38` — "an optional `iconv`-based adaptor".

`CMakeLists.txt:128` is `find_package(Iconv REQUIRED)`, unconditionally, and
`beman_install_library(... DEPENDENCIES Iconv)` propagates it to consumers.

Narrow the claims to match the build.  iconv is a required platform dependency
of the reference implementation; the WHATWG half is portable *as source* but is
not separately buildable today.  Say that, rather than saying either less or
more.  Whether to make it separable is the SG16 question above, and the honest
version of this paragraph is what makes that question legible.

## S-04: prose in the synopsis

`papers/wording/transcode.syn.md:462-490` carries substantial semantic prose
after the declarations, much of it duplicated in the dedicated semantic
clauses.  Run `make wording-validate` and decide whether specgen is attaching
those remarks to synopsis declarations on purpose.

If it is not, the fix is in the *headers* -- relocate the source annotations so
each normative rule has one home -- not in the fragments, which are generated.
If it is, say so in `docs/wording-outline.md` so the next reader does not
re-open it.

## S-05: claims narrower than their evidence

Each of these is stronger than what is nearby to support it.

- `papers/transcode-view.md:51` — "It allocated on every call and dispatched
  through a virtual interface to do so; the C89 `mbstowcs` outruns it by
  roughly a factor of two."  `make bench-codecvt` exists and
  `docs/benchmarks/PLATFORM_NOTES.md:114-132` describes the harness.  Run it
  and either cite the platform, toolchain and standard library the factor was
  measured on, or drop the number and keep the direction.
- `papers/transcode-view.md:347` — "four browser engines".  Verify the count or
  write "major browser engines".
- Broad iconv-adoption claims — qualify to what is actually known.
- `README.md:6-9` — "all ISO-8859 and Windows code pages" exceeds the
  WHATWG-listed set.  "WHATWG-listed ISO-8859 and Windows variants."

A claim with a citation is worth more to this paper than a claim with a bigger
number, and the benchmark harness is already built.

## The drift items

- **D-01** — `papers/transcode-view.md:13-14` has two `email:` keys under one
  author.  The renderer warns `Duplicate key: .author[0].email` and discards
  one.  Keep one address, or use the renderer's supported form for several.
- **D-02** — the feature-test macro placeholder is spelled `202XXXL` in one
  place and `20XXXXL` in another.  Pick `202XXXL` and use it everywhere; grep,
  do not fix the two the review names and stop.
- **D-03** — `README.md:291-294` and `papers/parts/tail.md:125-128` say
  `sniff_encoding` detects UTF-8/16/32 BOMs.  It detects UTF-8, UTF-16BE and
  UTF-16LE, and the `codec` enum has no UTF-32 entry to return.  Remove UTF-32
  from the claim.  Adding UTF-32 codecs is a feature, not a documentation fix,
  and is out of scope here.
- **D-04** — `papers/README.md:7` says D4246R0; the paper is D4246R1.
  `README.md:235-242` says P2728R12; the tail and
  `docs/p2728-alignment.md` say P2728R13.  Fix both, and define each cited
  revision in one place so the next bump is one edit.

## Carried in from earlier steps

These do not regenerate.  They are hand-written prose about things Steps 2, 5
and 7 changed:

- **From Step 2** — `iconv_error` gains `open_failed` and `system_error`.  The
  enumeration is printed in the paper's Error Handling section, and line 530
  currently claims the enum already describes a descriptor that would not open.
  It will, now; make the sentence true rather than deleting it.
- **From Step 5** — `transcode_string`'s `codec` overload returns
  `optional<string>`.  Fix the prose at line ~403 ("If the target codec has no
  WHATWG encoder, the result is empty") and the API Surface Comparison table.
- **From Step 7** — the U+FFFD reservation question, if Step 7 handed it over
  rather than landing it.

## Order of work

1. Build specgen; `make wording-check` on an unmodified tree.
2. Header wording changes (P-02, S-04 if it lands in headers).
3. `make wording`; `PENDING` empties.
4. Paper and README prose (P-01, S-05, D-01 – D-04, the carried items).
5. `make wording-check`, `make wording-inputs-check`, `make wording-validate`.
6. Render the paper; the only known warning was D-01, and it should be gone.

## Done when

- The synopsis defines every name it uses.
- No claim in the README or the paper is broader than its evidence.
- `papers/wording/PENDING` is empty and `make wording-check` passes.
- The paper renders without warnings.
