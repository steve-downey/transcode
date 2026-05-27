// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <catch2/catch_all.hpp>

#include <span>

TEST_CASE("WHATWG legacy codec benchmarks", "[benchmark][whatwg]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("Single-byte decode: windows-1251 Russian")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ru_mars_windows1251.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::windows_1251>); });
    };

    BENCHMARK_ADVANCED("Multi-byte decode: Shift-JIS Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::shift_jis>); });
    };

    BENCHMARK_ADVANCED("UTF-8 decode: Japanese (3-byte heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };
}
