// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <catch2/catch_all.hpp>

#include <span>

TEST_CASE("smoke benchmark — UTF-8 decode ASCII corpus", "[smoke][benchmark]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("whatwg_decode utf_8 en_mars corpus")(Catch::Benchmark::Chronometer meter) {
        auto input = corpus_span("en_mars_utf8.txt");
        meter.measure([&] { return count_elements(input | whatwg_decode<codec::utf_8>); });
    };
}
