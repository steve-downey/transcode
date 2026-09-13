# P6-Step 4: `null_term_view` Says What It Requires

**Branch:** `p6-step4-null-term-precondition`
**Depends on:** Step 0
**Finding:** C-06
**Read first:** `docs/plans/phase6-index.md`

---

## Goal

Give `null_term_view` the precondition it has always had, and stop its
default constructor producing an object whose `begin()` is a wild pointer.

## Why

`null_sentinel_t::operator==` is specified as `\returns *it == 0`
(`null_term.hpp:20`).  Reaching that comparison requires dereferencing the
iterator, so iteration reads element after element until it finds a zero.
Nothing anywhere says a zero has to be there.

`views::null_term` accepts any pointer and any array (`null_term.hpp:54`,
`:57`), and its `\remarks` at lines 82-87 explains at length why a non-pointer
is ill-formed -- "so that a range carrying no terminator is a diagnosed error
and not a silent one" -- while saying nothing about a pointer that carries no
terminator either.  That is the case the wording should cover, and it is the
one it skips.  An array without a zero element is the same hazard through the
other overload.

Separately:

```cpp
I ptr_;                                   // line 35
constexpr null_term_view() = default;     // line 38
constexpr I begin() const;                // line 41 — returns ptr_
```

For `I = const char*`, `= default` default-initializes, so `ptr_` holds an
indeterminate value and `begin()` hands it to the caller.  The class is a
`view`, and `view` requires `default_initializable`, so the constructor cannot
simply be deleted without taking `null_term_view` out of the range adaptor
world -- but nothing requires the value to be garbage.

## What to change

`include/beman/transcode/null_term.hpp`.

- **Value-initialize.**  `I ptr_{};` at line 35.  For a pointer this is null;
  for a general `contiguous_iterator` it is that type's value-initialized
  state.  One character, and the indeterminate read is gone.
- **Specify the precondition.**  An `\expects` on the converting constructor
  (line 103) requiring that a zero-valued element be reachable from `ptr` by
  repeated increment, and that every element from `ptr` to that one inclusive
  be within the lifetime of a single object.  The second half matters as much
  as the first: a terminator that exists in some *later* object is not a
  terminator.
- **Specify the singular state.**  A `\remarks` on the default constructor
  saying the resulting view is singular: it can be assigned to and destroyed,
  and `begin()` on it has no reachable terminator, so iterating it is
  undefined.  Say it once, here, rather than repeating it on `begin`.
- **Carry it to the adaptor.**  The `views::null_term` `\remarks` at lines
  82-87 gains a sentence pointing at the constructor's precondition, so a
  reader who arrives through the adaptor -- which is how the paper's examples
  arrive -- is told.

Do not add a runtime check.  This is a precondition on an unbounded scan; the
only honest check is the scan itself.

## Tests

`tests/beman/transcode/null_term.test.cpp`.

- A default-constructed `null_term_view<const char*>` has `begin() == nullptr`.
  That is the value-initialization fix, and it is checkable.
- `constify()` the same, so the guarantee holds in constant evaluation.
- Round-trip an ordinary C string and an array with an interior zero, to pin
  that the terminator is the *first* zero and not the last element.

There is no negative compile test here: a missing terminator is a precondition,
not a constraint, and the type system cannot see it.  `CLAUDE.md` asks for a
negative compile test for every *constraint*; this step adds none, and the
existing `null_term_reject_range_fail.cpp` already covers the constraint that
does exist.

## Done when

- `ptr_` is value-initialized and a test proves it.
- The constructor, the default constructor and `views::null_term` each say what
  they require.
- `make test` and `make lint` pass.
