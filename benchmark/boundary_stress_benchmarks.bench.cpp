// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/iconv_transcode_view.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <tests/beman/transcode/iconv_mock.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <cstddef>
#include <span>
#include <string>

namespace {

// 64-byte synthetic corpus of ASCII pairs.  Content is irrelevant; size is
// even so mock_iconv_pairwise can consume it completely (2 bytes per unit).
const std::string einval_corpus(64, 'A');

// 48-byte synthetic corpus for E2BIG stress.  mock_iconv_e2big writes 1 byte
// then returns E2BIG, exercising the recovery loop once per output byte.
const std::string e2big_corpus(48, 'X');

// Output buffer shared across mock-iconv benchmark iterations.
// 256 bytes far exceeds any single mock conversion; benchmarks are
// single-threaded so no data-race concern.
alignas(64) std::array<char, 256> mock_out_buf;

} // namespace

// ── Chunked whatwg_decode restart overhead ──────────────────────────────────
//
// Each chunk is a fresh view instantiation: no cross-chunk state is preserved.
// The "hostile" case splits Arabic 2-byte sequences at 3-byte boundaries so
// roughly every other chunk begins with a continuation byte, forcing one
// replacement-char error per split.  The "friendly" case uses 4-byte chunks
// that always align on a 2-byte Arabic code unit boundary.
//
// Baseline (non-chunked) is provided for direct comparison.

TEST_CASE("Chunked whatwg decode: restart overhead", "[benchmark][boundary]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("Chunked whatwg UTF-8→char32: Arabic full corpus (baseline)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ar_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    BENCHMARK_ADVANCED("Chunked whatwg UTF-8→char32: Arabic 4-byte chunks (friendly)")
    (Catch::Benchmark::Chronometer meter) {
        auto data   = load_corpus("ar_mars_utf8.txt");
        auto chunks = chunk_corpus(data, 4); // 4 bytes = 2 complete Arabic 2-byte chars
        meter.measure([&] {
            std::size_t n = 0;
            for (auto chunk : chunks)
                n += count_elements(std::span<const char>(chunk.data(), chunk.size()) | whatwg_decode<codec::utf_8>);
            return n;
        });
    };

    BENCHMARK_ADVANCED("Chunked whatwg UTF-8→char32: Arabic 3-byte chunks (hostile)")
    (Catch::Benchmark::Chronometer meter) {
        auto data   = load_corpus("ar_mars_utf8.txt");
        auto chunks = chunk_corpus(data, 3); // 3 bytes splits 2-byte seqs at boundaries
        meter.measure([&] {
            std::size_t n = 0;
            for (auto chunk : chunks)
                n += count_elements(std::span<const char>(chunk.data(), chunk.size()) | whatwg_decode<codec::utf_8>);
            return n;
        });
    };
}

// ── Mock-iconv EINVAL stress: incomplete sequence recovery ─────────────────
//
// mock_iconv_pairwise requires exactly 2 input bytes per conversion.  When
// given only 1 byte it returns EINVAL, causing the iconv_transcode_view
// iterator to accumulate the byte in its staging buffer before retrying.
// The 64-byte corpus of ASCII pairs drives 32 successful conversions, each
// preceded by one EINVAL staging event.  Tagged [smoke] for a quick sanity
// run of the EINVAL recovery path.

TEST_CASE("Mock-iconv EINVAL stress: pairwise staging", "[benchmark][boundary][smoke]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;
    using namespace beman::transcoding::tests;

    BENCHMARK_ADVANCED("Mock-iconv pairwise EINVAL: 64-byte corpus")
    (Catch::Benchmark::Chronometer meter) {
        iconv_functions          fns{mock_iconv_open, mock_iconv_pairwise, mock_iconv_close};
        std::span<const char>    data{einval_corpus.data(), einval_corpus.size()};
        std::span<char>          out{mock_out_buf};
        meter.measure([&] {
            return count_elements(
                iconv_transcode_view<iconv_functions, std::span<const char>>(data, fns, "X", "X", out));
        });
    };
}

// ── Mock-iconv E2BIG stress: output-buffer exhaustion recovery ─────────────
//
// mock_iconv_e2big writes exactly 1 byte then returns E2BIG regardless of
// output buffer capacity, exercising the "yield one byte and retry" loop once
// per input byte.  The 48-byte corpus drives 48 E2BIG recoveries per
// benchmark iteration.  Tagged [smoke] for a quick sanity run of the E2BIG
// recovery path.

TEST_CASE("Mock-iconv E2BIG stress: buffer exhaustion recovery", "[benchmark][boundary][smoke]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;
    using namespace beman::transcoding::tests;

    BENCHMARK_ADVANCED("Mock-iconv E2BIG: 1 byte/call, 48-byte corpus")
    (Catch::Benchmark::Chronometer meter) {
        iconv_functions          fns{mock_iconv_open, mock_iconv_e2big, mock_iconv_close};
        std::span<const char>    data{e2big_corpus.data(), e2big_corpus.size()};
        std::span<char>          out{mock_out_buf};
        meter.measure([&] {
            return count_elements(
                iconv_transcode_view<iconv_functions, std::span<const char>>(data, fns, "X", "X", out));
        });
    };
}
