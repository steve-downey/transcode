// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// simdutf ceiling baseline benchmarks.
//
// simdutf is a C++ library that uses SIMD instructions to achieve maximum
// throughput for Unicode transcoding.  It represents the performance ceiling
// for hardware-accelerated UTF conversion on the target machine.
//
// These benchmarks are aligned with P3-Step 4 UTF benchmark cases (same corpus
// files, equivalent decode operations) to allow direct comparison.  simdutf
// operates on contiguous buffers, so no lazy-range overhead is measured.
//
// Build: enabled only when cmake is configured with
//   -DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON
// See benchmark/CMakeLists.txt for FetchContent details.

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>

#include <simdutf.h>

#include <catch2/catch_all.hpp>

#include <cstddef>
#include <span>

namespace {

// Pre-allocated output buffers — sized for the largest expected corpus.
// UTF-8 → UTF-32: worst case is 1 input byte → 1 char32_t (ASCII), so
// input_len * sizeof(char32_t) output bytes suffice.  4 MiB covers any
// realistic benchmark corpus.
constexpr std::size_t kMaxCodepoints = 4 * 1024 * 1024;
static char32_t       g_out_utf32[kMaxCodepoints];
static char16_t       g_out_utf16[kMaxCodepoints];

} // namespace

TEST_CASE("simdutf ceiling baselines", "[benchmark][simdutf]") {
    using namespace beman::transcoding::bench;

    // --- UTF-8 → UTF-32 (matches whatwg_decode<codec::utf_8> output type) ---

    BENCHMARK_ADVANCED("simdutf UTF-8→UTF-32: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("en_mars_utf8.txt");
        REQUIRE(src.size() <= kMaxCodepoints);
        meter.measure([&] { return simdutf::convert_utf8_to_utf32(src.data(), src.size(), g_out_utf32); });
    };

    BENCHMARK_ADVANCED("simdutf UTF-8→UTF-32: Arabic (multibyte-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("ar_mars_utf8.txt");
        REQUIRE(src.size() <= kMaxCodepoints);
        meter.measure([&] { return simdutf::convert_utf8_to_utf32(src.data(), src.size(), g_out_utf32); });
    };

    // --- UTF-8 validation (sub-operation of decode; fastest simdutf path) ---

    BENCHMARK_ADVANCED("simdutf UTF-8 validate: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("en_mars_utf8.txt");
        meter.measure([&] { return simdutf::validate_utf8(src.data(), src.size()); });
    };

    BENCHMARK_ADVANCED("simdutf UTF-8 validate: Arabic (multibyte-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("ar_mars_utf8.txt");
        meter.measure([&] { return simdutf::validate_utf8(src.data(), src.size()); });
    };

    // --- UTF-8 → UTF-16 (common output format in many runtimes) ---

    BENCHMARK_ADVANCED("simdutf UTF-8→UTF-16: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("en_mars_utf8.txt");
        REQUIRE(src.size() <= kMaxCodepoints);
        meter.measure([&] { return simdutf::convert_utf8_to_utf16le(src.data(), src.size(), g_out_utf16); });
    };

    BENCHMARK_ADVANCED("simdutf UTF-8→UTF-16: Arabic (multibyte-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto src = corpus_span("ar_mars_utf8.txt");
        REQUIRE(src.size() <= kMaxCodepoints);
        meter.measure([&] { return simdutf::convert_utf8_to_utf16le(src.data(), src.size(), g_out_utf16); });
    };
}
