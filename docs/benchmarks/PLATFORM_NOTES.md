# Benchmark Platform Notes

## `std::codecvt` / `std::wstring_convert` (negative baseline)

`std::codecvt_utf8` and `std::wstring_convert` were deprecated in C++17 and
removed in C++26.  The `codecvt_benchmarks.bench.cpp` file compiles on any
platform but produces live benchmarks only where `__has_include(<codecvt>)`
is true.  Otherwise, `SKIP()` is invoked at runtime.

| Platform / STL        | `<codecvt>` present? | Notes                                   |
|-----------------------|----------------------|-----------------------------------------|
| libstdc++ (GCC ≤ 14)  | Yes                  | Deprecated; may emit `-Wdeprecated-declarations` (suppressed in benchmark). |
| libc++ (Clang ≥ 16)   | No                   | Removed; benchmark is SKIP()ed.         |
| MSVC STL (VS 2022)    | Yes                  | Deprecated; present.                    |

This benchmark is a **negative historical baseline**.  The result is expected
to be slower than range-based alternatives (`whatwg_decode`, `decode()`).
`wstring_convert::from_bytes` performs a heap allocation per call, unlike the
zero-allocation range-based views.

Do not treat `codecvt` benchmark regressions as project regressions.
