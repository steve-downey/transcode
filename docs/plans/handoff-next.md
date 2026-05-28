# Handoff: P4-Step 2 Complete → P4-Step 3 Next

## Completed

- **P4-Step 1: Pluggable Encode View** — done on `p4-step1-pluggable-encode` branch
- **P4-Step 2: Pluggable Bulk Operations** — done on `p4-step2-pluggable-bulk` branch

## What was done in Step 2

- Modified `include/beman/transcode/detail/bulk_transcode.hpp`:
  - Added `#include <beman/transcode/decode_view.hpp>`, `<beman/transcode/encode_view.hpp>`,
    and `<beman/transcode/detail/codec_concepts.hpp>` to the existing includes
  - Added four new function templates after the WHATWG overloads:
    - `decode_to(Codec, R&&) → std::vector<char32_t>` — constrained on `decode_codec`;
      uses `codec.decode_byte()` fast path when `random_access_decode_codec_type` is satisfied
    - `encode_to(Codec, R&&, Container = std::string)` — constrained on `encode_codec`;
      delegates to `encode(codec)` view
    - `decode_into(Codec, R&&, Output)` — constrained on `decode_codec`;
      same random-access fast path as `decode_to`
    - `encode_into(Codec, R&&, Output)` — constrained on `encode_codec`
  - All four are `constexpr`
- `tests/beman/transcode/pluggable_bulk.test.cpp` — 16 runtime + consteval tests
  using `latin1_codec` (a `table_codec<latin1_upper>` instantiation defined locally);
  tests cover ASCII, upper-half, unmapped bytes, empty input, round-trip, and
  consteval paths
- Updated `tests/beman/transcode/CMakeLists.txt` — registered `beman.transcode.tests.pluggable_bulk`
  executable with `add_test`

## Files modified in Step 2

- `include/beman/transcode/detail/bulk_transcode.hpp` — added pluggable overloads
- `tests/beman/transcode/pluggable_bulk.test.cpp` — new test file
- `tests/beman/transcode/CMakeLists.txt` — added pluggable_bulk test target

## Current State

- `make test` passes: 674 C++ tests + 250 Python tests, all green
- `make lint` passes: mypy, ruff, clang-format, gersemi, codespell, shellcheck all clean

## Branch State

`p4-step2-pluggable-bulk` is ready to merge to `main`.
The user will merge it (along with `p4-step1-pluggable-encode` if not already done)
before starting Step 3.

## Next Step: P4-Step 3 — Pluggable Transcode Pipeline

Read `docs/plans/p4-step3-pluggable-transcode.md` for full details.

**Summary of what Step 3 requires:**

Add a `transcode_view<DecodeCodec, EncodeCodec, R>` view (and pipe adaptor) that
chains `decode(decode_codec) | encode(encode_codec)` to transcode a byte range
from one encoding to another using pluggable codecs. This is the pluggable
analogue of the existing WHATWG `transcode_view`.

**Key files to read before starting Step 3:**

- `include/beman/transcode/transcode_view.hpp` — the existing WHATWG transcode
  view (the pattern to follow for the pluggable version)
- `include/beman/transcode/decode_view.hpp` — `decode(codec)` factory (Step 1 predecessor)
- `include/beman/transcode/encode_view.hpp` — `encode(codec)` factory (Step 1)
- `include/beman/transcode/detail/codec_concepts.hpp` — `decode_codec`, `encode_codec`

**Design note:** The pluggable transcode view can be implemented simply by
composing `decode` and `encode` views — it does not need a hand-rolled iterator.
The main deliverable is the pipe adaptor factory and a `transcode<DC,EC>(range)`
free function.

**Test file:** Create `tests/beman/transcode/pluggable_transcode.test.cpp` using
`latin1_codec` (decode and encode) through the new view, plus consteval tests.

**Register** the new test in `tests/beman/transcode/CMakeLists.txt` following the
pattern of adjacent pluggable test targets.

After `make test` and `make lint` are green, update `docs/plans/handoff-next.md`.
