// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/iconv_bulk.hpp>
#include <beman/transcode/iconv_bulk.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/whatwg/decode_view.hpp>

#include <catch2/catch_all.hpp>

#include <iconv.h>

#include <cerrno>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>

// Naive iconv: the loop a search for "iconv into a std::string" produces when
// nothing else is known, and the paper's "Before: iconv" example.  Open a
// handle, size the string to a guess, call iconv() in a loop, grow the string
// and fix up the output pointer on E2BIG, trim to what was written, close the
// handle.  `initial_factor` is the guess, as a multiple of the input size: 4
// is the paper's example and is enough for every corpus here, so the E2BIG
// path never runs; 1 is the other common guess, and the E2BIG path runs
// several times.
static std::string
naive_iconv_to_string(std::span<const char> data, const char* from, const char* to, std::size_t initial_factor) {
    iconv_t cd = ::iconv_open(to, from);
    if (cd == reinterpret_cast<iconv_t>(-1))
        throw std::runtime_error("iconv_open");

    std::string result;
    result.resize(data.size() * initial_factor);

    char*       inbuf   = const_cast<char*>(data.data());
    std::size_t inleft  = data.size();
    char*       outbuf  = result.data();
    std::size_t outleft = result.size();

    while (inleft > 0) {
        std::size_t r = ::iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
        if (r == static_cast<std::size_t>(-1)) {
            if (errno == E2BIG) {
                std::size_t used = static_cast<std::size_t>(outbuf - result.data());
                result.resize(result.size() * 2);
                outbuf  = result.data() + used;
                outleft = result.size() - used;
            } else if (errno == EILSEQ) {
                // Skip the invalid byte and write '?'.  The corpora are valid,
                // so this never runs; it is here so the loop is the one a
                // program would ship.
                ++inbuf;
                --inleft;
                if (outleft == 0) {
                    std::size_t used = static_cast<std::size_t>(outbuf - result.data());
                    result.resize(result.size() * 2);
                    outbuf  = result.data() + used;
                    outleft = result.size() - used;
                }
                *outbuf++ = '?';
                --outleft;
            } else {
                ::iconv_close(cd);
                throw std::runtime_error("iconv");
            }
        }
    }
    result.resize(static_cast<std::size_t>(outbuf - result.data()));
    ::iconv_close(cd);
    return result;
}

// Raw iconv loop: process the entire corpus through an already-open iconv
// handle into a fixed buffer that is overwritten on every call.  Nothing is
// kept: this is the floor for iconv(3) itself, not a program's use of it.
// Returns the total number of output bytes produced (prevents DCE).
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

// Shift_JIS bytes decoded by beman's WHATWG decoder, as UTF-32LE bytes for
// iconv: least significant byte first, whatever the host's byte order.
static std::string shift_jis_as_utf32le(std::span<const char> shift_jis) {
    using namespace beman::transcoding;
    std::string bytes;
    for (char32_t cp : shift_jis | whatwg::decode<whatwg::encoding::shift_jis>)
        for (int shift = 0; shift < 32; shift += 8)
            bytes.push_back(static_cast<char>((cp >> shift) & 0xFF));
    return bytes;
}

TEST_CASE("iconv baselines", "[benchmark][iconv]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    static char out_buf[4096];

    describe({.conversion = "UTF-8 → UTF-32", .library = "iconv", .api = "iconv(3) loop"});
    BENCHMARK_ADVANCED("Raw iconv: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto    data = corpus_span("en_mars_utf8.txt");
        iconv_t cd   = ::iconv_open("UTF-32LE", "UTF-8");
        measure(meter, [&] { return raw_iconv_process(cd, data, out_buf, sizeof(out_buf)); });
        ::iconv_close(cd);
    };

    describe({.conversion = "UTF-8 → UTF-32", .library = "iconv", .api = "naive iconv(3) into std::string, sized 4x"});
    BENCHMARK_ADVANCED("Naive iconv into std::string (4x): UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        measure(meter, [&] { return naive_iconv_to_string(data, "UTF-8", "UTF-32LE", 4).size(); });
    };

    describe({.conversion = "UTF-8 → UTF-32", .library = "iconv", .api = "naive iconv(3) into std::string, sized 1x"});
    BENCHMARK_ADVANCED("Naive iconv into std::string (1x): UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        measure(meter, [&] { return naive_iconv_to_string(data, "UTF-8", "UTF-32LE", 1).size(); });
    };

    describe({.conversion = "UTF-8 → UTF-32", .library = "iconv", .api = "iconv_transcode_view"});
    BENCHMARK_ADVANCED("iconv_transcode_view: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        measure(meter,
                [&] { return count_elements(data | iconv_transcode("UTF-8", "UTF-32LE", std::span(out_buf))); });
    };

    describe({.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "iconv(3) loop"});
    BENCHMARK_ADVANCED("Raw iconv: Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto    data = corpus_span("ja_mars_shiftjis.bin");
        iconv_t cd   = ::iconv_open("UTF-8", "SHIFT_JIS");
        measure(meter, [&] { return raw_iconv_process(cd, data, out_buf, sizeof(out_buf)); });
        ::iconv_close(cd);
    };

    describe(
        {.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "naive iconv(3) into std::string, sized 4x"});
    BENCHMARK_ADVANCED("Naive iconv into std::string (4x): Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        measure(meter, [&] { return naive_iconv_to_string(data, "SHIFT_JIS", "UTF-8", 4).size(); });
    };

    describe(
        {.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "naive iconv(3) into std::string, sized 1x"});
    BENCHMARK_ADVANCED("Naive iconv into std::string (1x): Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        measure(meter, [&] { return naive_iconv_to_string(data, "SHIFT_JIS", "UTF-8", 1).size(); });
    };

    describe({.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "iconv_transcode_view"});
    BENCHMARK_ADVANCED("iconv_transcode_view: Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        measure(meter,
                [&] { return count_elements(data | iconv_transcode("SHIFT_JIS", "UTF-8", std::span(out_buf))); });
    };
}

TEST_CASE("iconv bulk operations", "[benchmark][iconv]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    describe({.conversion = "UTF-8 → UTF-32", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("en_mars_utf8.txt");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "UTF-8", "UTF-32LE").size(); });
    };

    describe({.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: Shift-JIS to UTF-8 Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "SHIFT_JIS", "UTF-8").size(); });
    };

    describe({.conversion = "UTF-8 → UTF-32", .library = "iconv", .api = "iconv_transcode_into"});
    BENCHMARK_ADVANCED("iconv_transcode_into: UTF-8 to UTF-32LE English")
    (Catch::Benchmark::Chronometer meter) {
        auto        data = corpus_span("en_mars_utf8.txt");
        std::string sink;
        sink.reserve(data.size() * 4);
        measure(meter, [&] {
            sink.clear();
            iconv_transcode_into(data, "UTF-8", "UTF-32LE", std::back_inserter(sink));
            return sink.size();
        });
    };

    describe({.conversion = "EUC-JP → Shift_JIS", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: EUC-JP to Shift-JIS Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_eucjp.bin");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "EUC-JP", "SHIFT_JIS").size(); });
    };

    describe(
        {.conversion = "EUC-JP → Shift_JIS", .library = "iconv", .api = "naive iconv(3) into std::string, sized 4x"});
    BENCHMARK_ADVANCED("Naive iconv into std::string (4x): EUC-JP to Shift-JIS Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_eucjp.bin");
        measure(meter, [&] { return naive_iconv_to_string(data, "EUC-JP", "SHIFT_JIS", 4).size(); });
    };
}

TEST_CASE("iconv large corpus", "[benchmark][iconv]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    require_corpus("genji_monogatari_shiftjis.bin");
    require_corpus("genji_monogatari_eucjp.bin");
    require_corpus("xiyouji_gb18030.bin");

    static char out_buf[4096];

    describe({.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: Shift-JIS → UTF-8, Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_shiftjis.bin");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "SHIFT_JIS", "UTF-8").size(); });
    };

    describe(
        {.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "naive iconv(3) into std::string, sized 4x"});
    BENCHMARK_ADVANCED("Naive iconv into std::string (4x): Shift-JIS → UTF-8, Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_shiftjis.bin");
        measure(meter, [&] { return naive_iconv_to_string(data, "SHIFT_JIS", "UTF-8", 4).size(); });
    };

    describe({.conversion = "Shift_JIS → UTF-8", .library = "iconv", .api = "iconv_transcode_view"});
    BENCHMARK_ADVANCED("iconv_transcode_view: Shift-JIS → UTF-8, Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_shiftjis.bin");
        measure(meter,
                [&] { return count_elements(data | iconv_transcode("SHIFT_JIS", "UTF-8", std::span(out_buf))); });
    };

    describe({.conversion = "GB18030 → UTF-8", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: GB18030 → UTF-8, Journey to the West (624 KB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("xiyouji_gb18030.bin");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "GB18030", "UTF-8").size(); });
    };

    describe({.conversion = "EUC-JP → Shift_JIS", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: EUC-JP → Shift-JIS, Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_eucjp.bin");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "EUC-JP", "SHIFT_JIS").size(); });
    };

    describe({.conversion = "EUC-JP → UTF-32", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: EUC-JP → UTF-32LE, Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_eucjp.bin");
        measure(meter, [&] { return iconv_transcode_to<std::string>(data, "EUC-JP", "UTF-32LE").size(); });
    };

    // Shift_JIS encode on its own, from the novel's code points.  They are
    // decoded here, outside any benchmark, by the same decoder as the
    // beman.transcode twin in whatwg_benchmarks.bench.cpp, so both encode
    // the same input.
    const std::string genji_utf32le = shift_jis_as_utf32le(corpus_span("genji_monogatari_shiftjis.bin"));

    describe({.conversion = "UTF-32 → Shift_JIS", .library = "iconv", .api = "iconv_transcode_to"});
    BENCHMARK_ADVANCED("iconv_transcode_to: UTF-32LE → Shift-JIS, Genji")
    (Catch::Benchmark::Chronometer meter) {
        record_input("genji_monogatari_shiftjis.bin as UTF-32", genji_utf32le.size());
        measure(meter, [&] { return iconv_transcode_to<std::string>(genji_utf32le, "UTF-32LE", "SHIFT_JIS").size(); });
    };
}
