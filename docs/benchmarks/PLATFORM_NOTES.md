# Benchmark Platform Notes

## `simdutf` ceiling baseline (optional — requires explicit CMake opt-in)

`simdutf` uses SIMD intrinsics (SSE4, AVX2, AVX-512, NEON, etc.) to achieve
maximum throughput for UTF-8 decode, validate, and transcode operations.  It
represents the performance ceiling against which the composable
`beman::transcode` range-based approach is compared.

### Enabling

`simdutf` is **off by default** because it requires network access on the
first configure (FetchContent downloads the repo).  To enable:

```bash
# Re-configure with simdutf enabled (downloads on first run)
cd .build/build-system
uv run cmake -DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON .

# Then run
make bench-simdutf
```

### Files

| File | Purpose |
|------|---------|
| `benchmark/simdutf_benchmarks.bench.cpp` | Catch2 benchmark file |
| `benchmark/CMakeLists.txt` | FetchContent wiring under `BEMAN_TRANSCODE_BENCHMARK_SIMDUTF` option |

### Platform availability

| Platform | Notes |
|----------|-------|
| Any platform with CMake + network access | Downloads simdutf v5.6.4 on first configure |
| CI (without `-DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON`) | Silently skipped; `bench-simdutf` target absent |
| WSL2/Linux | AVX2 available; SIMD fast-paths active |
| macOS / arm64 | NEON fast-paths active |

### Benchmarks provided

- `simdutf UTF-8→UTF-32: English (ASCII-heavy)` — aligned with Step 4 `whatwg_decode<utf_8>` English case
- `simdutf UTF-8→UTF-32: Arabic (multibyte-heavy)` — aligned with Step 4 `whatwg_decode<utf_8>` Arabic case
- `simdutf UTF-8 validate: English` — pure validation pass (fastest simdutf path)
- `simdutf UTF-8 validate: Arabic` — multibyte validation
- `simdutf UTF-8→UTF-16: English` — common runtime output format
- `simdutf UTF-8→UTF-16: Arabic` — common runtime output format

### Expected comparison

`simdutf` routinely achieves 3–20× the throughput of scalar implementations on
multibyte-heavy text, and often saturates memory bandwidth on ASCII-heavy text.
The gap measured here quantifies the "mechanical sympathy cost" of using
standard C++ range composition instead of hand-tuned SIMD intrinsics.

---

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
