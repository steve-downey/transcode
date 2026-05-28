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

    // --- Single-byte codecs ---

    BENCHMARK_ADVANCED("Single-byte decode: windows-1251 Russian")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ru_mars_windows1251.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::windows_1251>); });
    };

    // --- Multi-byte: Shift-JIS ---

    BENCHMARK_ADVANCED("Multi-byte decode: Shift-JIS Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::shift_jis>); });
    };

    // --- UTF-8 via WHATWG ---

    BENCHMARK_ADVANCED("UTF-8 decode: Japanese (3-byte heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_utf8.txt");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_8>); });
    };

    // --- GB18030 / GBK: 2-byte and 4-byte CJK sequences ---

    BENCHMARK_ADVANCED("GB18030 decode: Chinese (2-byte and 4-byte CJK)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("zh_mars_gb18030.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::gbk>); });
    };

    // --- Big5: Traditional Chinese with dual-codepoint special cases ---

    BENCHMARK_ADVANCED("Big5 decode: Traditional Chinese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("zh_tw_mars_big5.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::big5>); });
    };

    // --- EUC-JP: SS2 (half-width katakana), SS3 (JIS X 0212), 2-byte JIS X 0208 ---

    BENCHMARK_ADVANCED("EUC-JP decode: Japanese (SS2/SS3/2-byte paths)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_eucjp.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::euc_jp>); });
    };

    // --- EUC-KR: 2-byte Korean with 0x7F gap offset ---

    BENCHMARK_ADVANCED("EUC-KR decode: Korean (2-byte)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ko_mars_euckr.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::euc_kr>); });
    };

    // --- ISO-2022-JP: stateful, 6-state machine with ESC sequences ---

    BENCHMARK_ADVANCED("ISO-2022-JP decode: Japanese (stateful, escape sequences)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_iso2022jp.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::iso_2022_jp>); });
    };

    // --- UTF-16: surrogate pair decode, large stream ---

    BENCHMARK_ADVANCED("UTF-16LE decode: large stream (War and Peace)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf16le.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_16le>); });
    };

    BENCHMARK_ADVANCED("UTF-16BE decode: large stream (War and Peace)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf16be.bin");
        meter.measure([&] { return count_elements(data | whatwg_decode<codec::utf_16be>); });
    };
}
