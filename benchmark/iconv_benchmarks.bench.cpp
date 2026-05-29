// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/iconv_bulk.hpp>
#include <beman/transcode/iconv_bulk.hpp>
#include <beman/transcode/iconv_real.hpp>

#include <catch2/catch_all.hpp>

#include <iconv.h>

#include <cstddef>
#include <span>
#include <string>

// Raw iconv loop: process the entire corpus through an already-open iconv
// handle.  Returns the total number of output bytes produced (prevents DCE).
static std::size_t raw_iconv_process(iconv_t cd, std::span<const char> data, char* out_buf, std::size_t out_buf_size) {
    // Reset converter state between benchmark iterations.
    ::iconv(cd, nullptr, nullptr, nullptr, nullptr);

    const char* in     = data.data();
    std::size_t inleft = data.size();
    std::size_t total  = 0;

    while (inleft > 0) {
        char*       out     = out_buf;
        std::size_t outleft = out_buf_size;
        ::iconv(cd, const_cast<char**>(&in), &inleft, &out, &outleft);
        total += out_buf_size - outleft;
    }
    return total;
}

TEST_CASE("iconv baselines", "[benchmark][iconv]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    static char out_buf[4096];

    BENCHMARK_ADVANCED("Raw iconv: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto    data = corpus_span("en_mars_utf8.txt");
        iconv_t cd   = ::iconv_open("UTF-32LE", "UTF-8");
        meter.measure([&] { return raw_iconv_process(cd, data, out_buf, sizeof(out_buf)); });
        ::iconv_close(cd);
    };

    BENCHMARK_ADVANCED("iconv_transcode_view: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | iconv_transcode("UTF-8", "UTF-32LE", std::span(out_buf))); });
    };

    BENCHMARK_ADVANCED("Raw iconv: Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto    data = corpus_span("ja_mars_shiftjis.bin");
        iconv_t cd   = ::iconv_open("UTF-8", "SHIFT_JIS");
        meter.measure([&] { return raw_iconv_process(cd, data, out_buf, sizeof(out_buf)); });
        ::iconv_close(cd);
    };

    BENCHMARK_ADVANCED("iconv_transcode_view: Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        meter.measure(
            [&] { return count_elements(data | iconv_transcode("SHIFT_JIS", "UTF-8", std::span(out_buf))); });
    };
}

TEST_CASE("iconv bulk operations", "[benchmark][iconv]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("iconv_transcode_to: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        meter.measure([&] { return iconv_transcode_to<std::string>(data, "UTF-8", "UTF-32LE").size(); });
    };

    BENCHMARK_ADVANCED("iconv_transcode_to: Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        meter.measure([&] { return iconv_transcode_to<std::string>(data, "SHIFT_JIS", "UTF-8").size(); });
    };

    BENCHMARK_ADVANCED("iconv_transcode_into: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto        data = corpus_span("en_mars_utf8.txt");
        std::string sink;
        sink.reserve(data.size() * 4);
        meter.measure([&] {
            sink.clear();
            iconv_transcode_into(data, "UTF-8", "UTF-32LE", std::back_inserter(sink));
            return sink.size();
        });
    };

    BENCHMARK_ADVANCED("iconv_transcode_to: EUC-JP to Shift-JIS Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_eucjp.bin");
        meter.measure([&] { return iconv_transcode_to<std::string>(data, "EUC-JP", "SHIFT_JIS").size(); });
    };
}
