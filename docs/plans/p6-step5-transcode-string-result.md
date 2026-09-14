# P6-Step 5: `transcode_string` Stops Signalling Failure With Emptiness

**Branch:** `p6-step5-transcode-string-result`
**Depends on:** Step 0
**Finding:** C-07
**Read first:** `docs/plans/phase6-index.md`

---

## Goal

Make the two `transcode_string` overloads agree, and make an empty result mean
an empty result.

## Why

The `codec` overload returns `std::string`, and its docblock says
(`transcode_string.hpp:74-75`):

> The result is empty when `to` names an encoding the WHATWG Encoding Standard
> defines no encoder for.

The implementation delivers that by falling off the end of the `switch`
(`transcode_string.hpp:316-317`):

```cpp
default:
    break; // replacement, x_user_defined: no encoder defined
```

An empty `std::string` is also what a successful conversion of empty input
returns, and what a conversion whose every character was unmappable returns.
Three different outcomes, one value, no way to tell them apart.

The rest of the library does not agree with this overload.  The label overload
(`:329-336`) returns `std::optional<std::string>` and reserves `nullopt` for a
label it cannot resolve -- so asking for a *resolvable* label naming an
encoder-less codec gives an **engaged** optional holding an empty string, which
reads as success.  And the compile-time equivalent, `transcode<From, To>`,
rejects an encoder-less target outright: the paper says it is "for `To` codecs
that have a WHATWG encoder."  The same mistake is a compile error in one
spelling, a `nullopt` in another, and a silent empty string in the third.

Returning `optional` from the `codec` overload makes all three consistent and
costs callers a `*`.  `expected` would carry a reason, but there is only one
reason -- the target has no encoder -- and the target is a value the caller
passed in, so the error adds nothing the caller does not already know.

## What to change

`include/beman/transcode/transcode_string.hpp`.

- Change the `codec` overload's return type to `std::optional<std::string>`.
- **Reject before decoding.**  Test `to` against the encoder-less set at the
  top of the function and return `nullopt` immediately.  Decoding the whole
  input and then discovering there is nowhere to put it is work for a result
  that is thrown away.
- The encoder-less set is `codec::utf_16be`, `codec::utf_16le`,
  `codec::replacement` and `codec::x_user_defined`.  The first two are
  decode-only because the Encoding Standard defines no UTF-16 encoder; the
  comment at line 317 names only the last two, which is the second bug in that
  line.  Prefer a named helper -- `constexpr bool has_whatwg_encoder(codec)` --
  over an inline condition, because Step 7 and the paper both want to name the
  same set.
- Delete the `default: break;`.  With the set tested up front, every remaining
  enumerator has an encoder, and a `switch` with no `default` gets a compiler
  warning when a codec is added -- which is the diagnostic you want.
- Update the `\returns` docblock: `nullopt` when `to` has no encoder, and the
  converted bytes otherwise.
- The label overload keeps its signature and simply propagates.  Its `\returns`
  gains the second `nullopt` case.

## Callers

- `README.md` — the `transcode_string` usage.
- `papers/transcode-view.md` — the runtime-label example at line ~678 already
  writes `if (auto utf8 = transcode_string(bytes, "shift_jis", "utf-8"))`, so
  it needs no change; the prose at line ~403 ("If the target codec has no
  WHATWG encoder, the result is empty") does.
- `examples/` — check each file; grep rather than assume.
- `tests/beman/transcode/transcode_string.test.cpp` and
  `transcode_string_label.test.cpp`.

## Tests

- `nullopt` for each of the four encoder-less targets, through both overloads.
- An engaged optional holding an empty string for empty input to a target that
  *does* have an encoder -- the case that is currently indistinguishable, and
  the reason for the change.
- The existing round-trip cases, updated to dereference.

## Done when

- Both overloads return `optional` and agree on what `nullopt` means.
- Empty input and no-encoder are distinguishable, with a test for each.
- `make test` and `make lint` pass.

## Note

This is a source-breaking change to a public entry point.  It is the right time
for it -- the paper is at R1 and has not been through LEWG -- but say so in the
commit message and hand it to Step 8 for the paper's API table.
