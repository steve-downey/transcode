// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <catch2/catch_all.hpp>

#include <span>

TEST_CASE("UTF benchmarks", "[benchmark][utf]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("UTF-8 decode: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    BENCHMARK_ADVANCED("UTF-8 decode: Arabic (multibyte-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ar_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    BENCHMARK_ADVANCED("UTF-8 encode round-trip: English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        meter.measure(
            [&] { return count_elements(data | whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>); });
    };
}
