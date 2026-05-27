// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Thin C-FFI shim around encoding_rs for use in beman.transcode benchmarks.
//
// All functions use non-streaming (contiguous buffer) mode, which avoids
// heap allocation inside the measured region.  The caller supplies a
// pre-allocated output buffer; the return value is the number of bytes
// written.  SIZE_MAX signals that the output buffer was too small.
//
// Panic is set to 'abort' in Cargo.toml so that no unwinding can cross
// the FFI boundary.

use encoding_rs::*;

/// Decode a UTF-8 byte slice to UTF-8 (WHATWG decode: BOM stripping +
/// replacement-character substitution for invalid sequences).
/// Returns bytes written, or usize::MAX if output buffer too small.
#[no_mangle]
pub unsafe extern "C" fn enc_rs_utf8_to_utf8(
    src: *const u8,
    src_len: usize,
    dst: *mut u8,
    dst_len: usize,
) -> usize {
    let input = std::slice::from_raw_parts(src, src_len);
    let output = std::slice::from_raw_parts_mut(dst, dst_len);
    let mut decoder = UTF_8.new_decoder_without_bom_handling();
    let (result, _read, written, _had_errors) =
        decoder.decode_to_utf8(input, output, true);
    if result == CoderResult::OutputFull {
        usize::MAX
    } else {
        written
    }
}

/// Decode a Windows-1251 byte slice to UTF-8.
/// Returns bytes written, or usize::MAX if output buffer too small.
#[no_mangle]
pub unsafe extern "C" fn enc_rs_windows1251_to_utf8(
    src: *const u8,
    src_len: usize,
    dst: *mut u8,
    dst_len: usize,
) -> usize {
    let input = std::slice::from_raw_parts(src, src_len);
    let output = std::slice::from_raw_parts_mut(dst, dst_len);
    let mut decoder = WINDOWS_1251.new_decoder_without_bom_handling();
    let (result, _read, written, _had_errors) =
        decoder.decode_to_utf8(input, output, true);
    if result == CoderResult::OutputFull {
        usize::MAX
    } else {
        written
    }
}

/// Decode a Shift-JIS byte slice to UTF-8.
/// Returns bytes written, or usize::MAX if output buffer too small.
#[no_mangle]
pub unsafe extern "C" fn enc_rs_shift_jis_to_utf8(
    src: *const u8,
    src_len: usize,
    dst: *mut u8,
    dst_len: usize,
) -> usize {
    let input = std::slice::from_raw_parts(src, src_len);
    let output = std::slice::from_raw_parts_mut(dst, dst_len);
    let mut decoder = SHIFT_JIS.new_decoder_without_bom_handling();
    let (result, _read, written, _had_errors) =
        decoder.decode_to_utf8(input, output, true);
    if result == CoderResult::OutputFull {
        usize::MAX
    } else {
        written
    }
}
