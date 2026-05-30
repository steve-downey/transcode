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

    // Large-stream benchmarks: exercises sustained throughput rather than
    // per-call overhead.  Uses War and Peace (~3.2 MiB when downloaded via
    // tools/download_benchmark_corpora.py; falls back to a small excerpt
    // from benchmark/corpus/ when the full corpus is not present).

    BENCHMARK_ADVANCED("UTF-8 decode: large stream (War and Peace, ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    BENCHMARK_ADVANCED("UTF-8 encode round-trip: large stream (War and Peace)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf8.txt");
        meter.measure(
            [&] { return count_elements(data | whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>); });
    };
}
