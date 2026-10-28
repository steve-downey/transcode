// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <catch2/catch_all.hpp>

#include <span>

TEST_CASE("smoke benchmark — UTF-8 decode ASCII corpus", "[smoke][benchmark]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("whatwg::decode utf_8 en_mars corpus")(Catch::Benchmark::Chronometer meter) {
        auto input = corpus_span("en_mars_utf8.txt");
        measure(meter, [&] { return count_elements(input | whatwg::decode<whatwg::encoding::utf_8>); });
    };
}
