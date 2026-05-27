// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// encoding_rs baseline benchmarks.
//
// encoding_rs is a Rust library (https://github.com/hsivonen/encoding_rs) that
// provides WHATWG Encoding Standard compliant decoding and encoding.  It is
// used here as an external baseline to compare against beman::transcode.
//
// These benchmarks use the non-streaming (contiguous buffer) API, which
// avoids heap allocation in the measured region and allows SIMD acceleration
// on supported platforms.  The Rust crate encoding_rs_bench wraps encoding_rs
// behind a plain C FFI and is compiled as a static library.
//
// Build: enabled only when BEMAN_TRANSCODE_ENCODING_RS_BENCH=ON (cmake) which
// is set automatically when cargo is found.  See benchmark/CMakeLists.txt.

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>

#include <catch2/catch_all.hpp>

#include <encoding_rs_bench/encoding_rs_bench.h>

#include <cstddef>
#include <cstdint>
#include <span>

namespace {

// Pre-allocated output buffer: 4× input length is safe for any encoding →
// UTF-8 conversion (worst case: 1 input byte → 4 output bytes for UTF-8
// surrogates, which encoding_rs maps to U+FFFD = 3 bytes — 4× is generous).
constexpr std::size_t kOutBufSize = 4 * 1024 * 1024;
static char           g_out_buf[kOutBufSize];

} // namespace

TEST_CASE("encoding_rs baselines", "[benchmark][encoding_rs]") {
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("encoding_rs UTF-8→UTF-8: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("en_mars_utf8.txt");
        REQUIRE(src.size() * 4 <= kOutBufSize);
        meter.measure([&] {
            return enc_rs_utf8_to_utf8(reinterpret_cast<const uint8_t*>(src.data()),
                                       src.size(),
                                       reinterpret_cast<uint8_t*>(g_out_buf),
                                       kOutBufSize);
        });
    };

    BENCHMARK_ADVANCED("encoding_rs UTF-8→UTF-8: Japanese (CJK-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("ja_mars_utf8.txt");
        REQUIRE(src.size() * 4 <= kOutBufSize);
        meter.measure([&] {
            return enc_rs_utf8_to_utf8(reinterpret_cast<const uint8_t*>(src.data()),
                                       src.size(),
                                       reinterpret_cast<uint8_t*>(g_out_buf),
                                       kOutBufSize);
        });
    };

    BENCHMARK_ADVANCED("encoding_rs Windows-1251→UTF-8: Russian")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("ru_mars_windows1251.bin");
        REQUIRE(src.size() * 4 <= kOutBufSize);
        meter.measure([&] {
            return enc_rs_windows1251_to_utf8(reinterpret_cast<const uint8_t*>(src.data()),
                                              src.size(),
                                              reinterpret_cast<uint8_t*>(g_out_buf),
                                              kOutBufSize);
        });
    };

    BENCHMARK_ADVANCED("encoding_rs Shift-JIS→UTF-8: Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("ja_mars_shiftjis.bin");
        REQUIRE(src.size() * 4 <= kOutBufSize);
        meter.measure([&] {
            return enc_rs_shift_jis_to_utf8(reinterpret_cast<const uint8_t*>(src.data()),
                                            src.size(),
                                            reinterpret_cast<uint8_t*>(g_out_buf),
                                            kOutBufSize);
        });
    };
}
