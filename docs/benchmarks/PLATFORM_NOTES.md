# Benchmark Platform Notes

## `encoding_rs` baseline (optional — requires Rust/Cargo)

`encoding_rs` is a Rust library providing WHATWG Encoding Standard compliant
decode/encode.  The benchmark uses the non-streaming (contiguous buffer) API to
avoid heap allocation in the measured region.

### Enabling

The CMake configure step auto-detects `cargo` on `PATH`.  If found, the
`beman.transcode.benchmarks.encoding_rs` target is built automatically during
`make compile`.  If `cargo` is absent, the target is silently skipped.

```bash
# Run encoding_rs benchmarks (requires cargo to be on PATH)
make bench-encoding-rs
```

### Files

| File | Purpose |
|------|---------|
| `benchmark/encoding_rs_bench/Cargo.toml` | Rust crate manifest; depends on `encoding_rs` crate |
| `benchmark/encoding_rs_bench/src/lib.rs` | Thin C-FFI shim (three `extern "C"` functions) |
| `benchmark/encoding_rs_bench/include/encoding_rs_bench/encoding_rs_bench.h` | C header for the shim |
| `benchmark/encoding_rs_benchmarks.bench.cpp` | Catch2 benchmark file |
| `benchmark/encoding_rs_bench/Cargo.lock` | Lock file for reproducible Rust builds |

### Platform availability

| Platform | `cargo` available? | Notes |
|----------|--------------------|-------|
| This machine (WSL2/Linux) | Yes — cargo 1.94.0 | All four benchmarks run |
| macOS | Yes (via Homebrew/rustup) | Expected to work |
| Windows | Yes (via rustup) | Expected to work; `.lib` extension differs but CMake handles it |
| CI (without Rust) | No | Silently skipped; `bench-encoding-rs` target absent |

### Benchmarks provided

- `encoding_rs UTF-8→UTF-8: English (ASCII-heavy)` — WHATWG UTF-8 passthrough
- `encoding_rs UTF-8→UTF-8: Japanese (CJK-heavy)` — WHATWG UTF-8 passthrough
- `encoding_rs Windows-1251→UTF-8: Russian` — legacy encoding decode
- `encoding_rs Shift-JIS→UTF-8: Japanese` — legacy encoding decode

### Results on this machine (Asan build — indicative only)

- English ASCII: ~75 ns (435-byte corpus) — essentially no work; all-ASCII passthrough
- Japanese CJK: ~350 ns (340-byte UTF-8 corpus) — multibyte scan overhead
- Russian Windows-1251→UTF-8: ~810 ns (340-byte corpus) — table lookup
- Japanese Shift-JIS→UTF-8: ~693 ns (340-byte corpus) — table lookup

Contrast with `whatwg_decode<utf_8>` on the same corpora: ≪ 1 µs.
`encoding_rs` is competitive on the ASCII-heavy path and slightly slower on
CJK/legacy paths, consistent with the Rust→C FFI overhead on small corpora.

---

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
