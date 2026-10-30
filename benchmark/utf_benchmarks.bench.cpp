// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <catch2/catch_all.hpp>

#include <span>

TEST_CASE("UTF benchmarks", "[benchmark][utf]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("UTF-8 decode: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::utf_8>); });
    };

    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("UTF-8 decode: Arabic (multibyte-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ar_mars_utf8.txt");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::utf_8>); });
    };

    describe({.conversion = "UTF-8 → UTF-32 → UTF-8",
              .library    = "beman.transcode",
              .api        = "whatwg::decode | whatwg::encode views"});
    BENCHMARK_ADVANCED("UTF-8 encode round-trip: English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        measure(meter, [&] {
            return count_elements(data | whatwg::decode<whatwg::encoding::utf_8> |
                                  whatwg::encode<whatwg::encoding::utf_8>);
        });
    };

    // Large-stream benchmarks: exercises sustained throughput rather than
    // per-call overhead.  Uses War and Peace (~3.2 MiB when downloaded via
    // tools/download_benchmark_corpora.py; falls back to a small excerpt
    // from benchmark/corpus/ when the full corpus is not present).

    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("UTF-8 decode: large stream (War and Peace, ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf8.txt");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::utf_8>); });
    };

    describe({.conversion = "UTF-8 → UTF-32 → UTF-8",
              .library    = "beman.transcode",
              .api        = "whatwg::decode | whatwg::encode views"});
    BENCHMARK_ADVANCED("UTF-8 encode round-trip: large stream (War and Peace)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf8.txt");
        measure(meter, [&] {
            return count_elements(data | whatwg::decode<whatwg::encoding::utf_8> |
                                  whatwg::encode<whatwg::encoding::utf_8>);
        });
    };
}
