// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// C header for the encoding_rs_bench Rust crate.
// Functions use non-streaming (contiguous buffer) mode — no heap allocation
// inside the call.  Returns bytes written to dst, or SIZE_MAX on output-full.

#ifndef ENCODING_RS_BENCH_INCLUDE_ENCODING_RS_BENCH_H
#define ENCODING_RS_BENCH_INCLUDE_ENCODING_RS_BENCH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Decode UTF-8 → UTF-8 (WHATWG: BOM strip + U+FFFD substitution).
size_t enc_rs_utf8_to_utf8(const uint8_t* src, size_t src_len, uint8_t* dst, size_t dst_len);

/// Decode Windows-1251 → UTF-8.
size_t enc_rs_windows1251_to_utf8(const uint8_t* src, size_t src_len, uint8_t* dst, size_t dst_len);

/// Decode Shift-JIS → UTF-8.
size_t enc_rs_shift_jis_to_utf8(const uint8_t* src, size_t src_len, uint8_t* dst, size_t dst_len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ENCODING_RS_BENCH_INCLUDE_ENCODING_RS_BENCH_H
