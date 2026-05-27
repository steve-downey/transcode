# P3-Step 5: Legacy WHATWG Codec Benchmarks

**Branch:** `p3-step5-legacy-whatwg-benchmarks`
**Depends on:** [p3-step4-utf-benchmarks.md](p3-step4-utf-benchmarks.md)
**Read first:** `docs/plans/phase3-handoff.md` and `docs/plans/handoff-next.md`

---

## Goal

Extend native `beman::transcode` measurement beyond UTF into representative
single-byte and multi-byte WHATWG codecs, plus the pluggable codec protocol.

## Context for Executing Agent

Library API for legacy codecs:
```cpp
// WhatWG enum-based (closed set):
bytes | whatwg_decode<codec::windows_1251>  // single-byte table O(1)
bytes | whatwg_decode<codec::shift_jis>     // multi-byte stateless
bytes | whatwg_decode<codec::gb18030>       // multi-byte stateful
bytes | whatwg_decode<codec::iso_2022_jp>   // stateful with escape sequences

// Pluggable codec protocol (open extension):
#include <beman/transcode/detail/table_codec.hpp>
#include <beman/transcode/decode_view.hpp>
// table_codec<std::array<char32_t,128>> — ASCII-compatible single-byte
// full_table_codec<std::array<char32_t,256>> — non-ASCII-compatible
bytes | decode(my_table_codec{})
```

Corpora:
- `ru_mars_windows1251.bin` — Russian in windows-1251 (single-byte)
- `ja_mars_shiftjis.bin` — Japanese in Shift-JIS (multi-byte)
- `ja_mars_utf8.txt` — Japanese UTF-8 (3-byte sequences)
- Fallback versions in `benchmark/corpus/`

Fixture API: `corpus_span("filename")`, `count_elements(range)`

## Deliverables

1. `benchmark/whatwg_benchmarks.bench.cpp` — WhatWG legacy codec cases
2. `benchmark/pluggable_codec_benchmarks.bench.cpp` — pluggable protocol cases
3. Registration in `benchmark/CMakeLists.txt`

## Benchmark Cases

### whatwg_benchmarks.bench.cpp

- "Single-byte decode: windows-1251 Russian" — `ru_mars_windows1251.bin`
- "Multi-byte decode: Shift-JIS Japanese" — `ja_mars_shiftjis.bin`
- "UTF-8 decode: Japanese (3-byte heavy)" — `ja_mars_utf8.txt`

### pluggable_codec_benchmarks.bench.cpp

For the pluggable protocol, use synthetic data since BLP-59001 and EBCDIC
don't have real-world corpora in the repo:

```cpp
// 4K buffer of bytes 0x80-0xBF (exercises the table lookup path)
std::string synthetic(4096, '\xC0');

inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();
using latin1_codec = table_codec<latin1_upper>;

BENCHMARK("pluggable table_codec: 4K upper-half") {
    return count_elements(std::span<const char>(synthetic) | decode(latin1_codec{}));
};
```

Compare against equivalent WhatWG codec to prove zero overhead:
```cpp
BENCHMARK("whatwg iso-8859-15: 4K upper-half") {
    return count_elements(std::span<const char>(synthetic) | whatwg_decode<codec::iso_8859_15>);
};
```

## Procedure

1. Create worktree branch `p3-step5-legacy-whatwg-benchmarks` from `main`
2. Create `benchmark/whatwg_benchmarks.bench.cpp`
3. Create `benchmark/pluggable_codec_benchmarks.bench.cpp`
4. Register in `benchmark/CMakeLists.txt`
5. Run `make compile`
6. Run `make test`
7. Run `make lint`
8. Run `make bench`
9. Commit, merge to main (non-ff), push both remotes
10. Update `docs/plans/handoff-next.md`

## Verification

```bash
make compile
make test
make lint
make bench
```

## Handoff to Step 6

Write to `docs/plans/handoff-next.md`:
- Step 5 is done
- Next: read `p3-step6-iconv-baselines.md`
- New files: `benchmark/whatwg_benchmarks.bench.cpp`,
  `benchmark/pluggable_codec_benchmarks.bench.cpp`
- Note the observed single-byte vs multi-byte performance delta
- Note whether pluggable codec matches built-in WhatWG performance
