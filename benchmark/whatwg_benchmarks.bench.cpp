// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/config_generated.hpp>
#include <beman/transcode/detail/tables/big5.hpp>
#include <beman/transcode/detail/tables/euc_kr.hpp>
#include <beman/transcode/detail/tables/gbk.hpp>
#include <beman/transcode/detail/tables/shift_jis.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <ranges>
#include <span>
#include <vector>

TEST_CASE("WHATWG legacy codec benchmarks", "[benchmark][whatwg]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    // --- Single-byte codecs ---

    describe({.conversion = "windows-1251 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("Single-byte decode: windows-1251 Russian")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ru_mars_windows1251.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::windows_1251>); });
    };

    // --- Multi-byte: Shift-JIS ---

    describe({.conversion = "Shift_JIS → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("Multi-byte decode: Shift-JIS Japanese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_shiftjis.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::shift_jis>); });
    };

    // --- UTF-8 via WHATWG ---

    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("UTF-8 decode: Japanese (3-byte heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_utf8.txt");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::utf_8>); });
    };

    // --- GB18030 / GBK: 2-byte and 4-byte CJK sequences ---

    describe({.conversion = "GBK → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("GB18030 decode: Chinese (2-byte and 4-byte CJK)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("zh_mars_gb18030.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::gbk>); });
    };

    // --- Big5: Traditional Chinese with dual-codepoint special cases ---

    describe({.conversion = "Big5 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("Big5 decode: Traditional Chinese")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("zh_tw_mars_big5.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::big5>); });
    };

    // --- EUC-JP: SS2 (half-width katakana), SS3 (JIS X 0212), 2-byte JIS X 0208 ---

    describe({.conversion = "EUC-JP → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("EUC-JP decode: Japanese (SS2/SS3/2-byte paths)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_eucjp.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::euc_jp>); });
    };

    // --- EUC-KR: 2-byte Korean with 0x7F gap offset ---

    describe({.conversion = "EUC-KR → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("EUC-KR decode: Korean (2-byte)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ko_mars_euckr.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::euc_kr>); });
    };

    // --- ISO-2022-JP: stateful, 6-state machine with ESC sequences ---

    describe({.conversion = "ISO-2022-JP → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("ISO-2022-JP decode: Japanese (stateful, escape sequences)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("ja_mars_iso2022jp.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::iso_2022_jp>); });
    };

    // --- UTF-16: surrogate pair decode, large stream ---

    describe({.conversion = "UTF-16LE → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("UTF-16LE decode: large stream (War and Peace)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf16le.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::utf_16le>); });
    };

    describe({.conversion = "UTF-16BE → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("UTF-16BE decode: large stream (War and Peace)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("war_and_peace_utf16be.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::utf_16be>); });
    };
}

// The novels come from tools/download_benchmark_corpora.py and have no
// checked-in fallback, so this test case skips when they are absent.
TEST_CASE("WHATWG legacy codec benchmarks: large CJK novels", "[benchmark][whatwg]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    require_corpus("genji_monogatari_shiftjis.bin");
    require_corpus("genji_monogatari_eucjp.bin");
    require_corpus("xiyouji_gb18030.bin");
    require_corpus("xiyouji_big5.bin");

    describe({.conversion = "Shift_JIS → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("Shift-JIS decode: Tale of Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_shiftjis.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::shift_jis>); });
    };

    describe({.conversion = "EUC-JP → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("EUC-JP decode: Tale of Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("genji_monogatari_eucjp.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::euc_jp>); });
    };

    // Shift_JIS encode on its own, from the novel's code points.  They are
    // decoded here, outside any benchmark, so only the encode is timed; the
    // iconv twin in iconv_benchmarks.bench.cpp records the same input.
    std::vector<char32_t> genji_code_points;
    for (char32_t cp : corpus_span("genji_monogatari_shiftjis.bin") | whatwg::decode<whatwg::encoding::shift_jis>)
        genji_code_points.push_back(cp);

    describe({.conversion = "UTF-32 → Shift_JIS", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("Shift-JIS encode: Tale of Genji (from UTF-32)")
    (Catch::Benchmark::Chronometer meter) {
        record_input("genji_monogatari_shiftjis.bin as UTF-32", genji_code_points.size() * sizeof(char32_t));
        measure(meter,
                [&] { return count_elements(genji_code_points | whatwg::encode<whatwg::encoding::shift_jis>); });
    };

#if BEMAN_TRANSCODE_HAS_RANGES_TO
    describe({.conversion = "UTF-32 → Shift_JIS", .library = "beman.transcode", .api = "encode_to"});
    BENCHMARK_ADVANCED("Shift-JIS encode_to: Tale of Genji (from UTF-32)")
    (Catch::Benchmark::Chronometer meter) {
        record_input("genji_monogatari_shiftjis.bin as UTF-32", genji_code_points.size() * sizeof(char32_t));
        measure(meter, [&] { return whatwg::encode_to<whatwg::encoding::shift_jis>(genji_code_points).size(); });
    };
#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

    describe({.conversion = "GB18030 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("GB18030 decode: Journey to the West (624 KB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("xiyouji_gb18030.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::gb18030>); });
    };

    describe({.conversion = "Big5 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("Big5 decode: Journey to the West (624 KB)")
    (Catch::Benchmark::Chronometer meter) {
        auto data = corpus_span("xiyouji_big5.bin");
        measure(meter, [&] { return count_elements(data | whatwg::decode<whatwg::encoding::big5>); });
    };
}

// Encoding into a multi-byte legacy codec is a reverse lookup from code point
// to index pointer.  The Mars corpus is 340 bytes, far too small to time a
// per-code-point cost, so the first benchmark repeats it, and the second
// encodes every code point in index jis0208 once, which spreads the lookups
// over the whole index rather than the common kana and kanji rows.
TEST_CASE("WHATWG Shift_JIS encode benchmarks", "[benchmark][whatwg]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    // Built here, not in the benchmark body: reading the corpus there would
    // record the corpus file as the input, and the first input recorded is
    // the one that counts.
    std::vector<char32_t> mars;
    {
        std::vector<char32_t> once;
        for (char32_t cp : corpus_span("ja_mars_shiftjis.bin") | whatwg::decode<whatwg::encoding::shift_jis>)
            once.push_back(cp);
        while (mars.size() < 256 * 1024)
            mars.insert(mars.end(), once.begin(), once.end());
    }

    describe({.conversion = "UTF-32 → Shift_JIS", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("Shift-JIS encode: Japanese (Mars corpus repeated to 256K code points)")
    (Catch::Benchmark::Chronometer meter) {
        const auto& cps = mars;
        record_input("ja_mars_shiftjis.bin repeated", cps.size() * sizeof(char32_t));
        measure(meter, [&] { return count_elements(cps | whatwg::encode<whatwg::encoding::shift_jis>); });
    };

    describe({.conversion = "UTF-32 → Shift_JIS", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("Shift-JIS encode: every code point in index jis0208 once")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<char32_t> cps;
        for (char32_t cp : detail::tables::shift_jis)
            if (cp != 0)
                cps.push_back(cp);
        std::ranges::sort(cps);
        auto dup = std::ranges::unique(cps);
        cps.erase(dup.begin(), dup.end());
        record_input("index jis0208 code points", cps.size() * sizeof(char32_t));
        measure(meter, [&] { return count_elements(cps | whatwg::encode<whatwg::encoding::shift_jis>); });
    };
}

// The same one-of-each sweep for the other multi-byte encoders.  Each walks
// its whole index, so a per-code-point cost that grows with the index shows
// up here before it shows up on text.
TEST_CASE("WHATWG multi-byte encode sweeps", "[benchmark][whatwg]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    auto distinct = [](const auto& table) {
        std::vector<char32_t> cps;
        for (char32_t cp : table)
            if (cp != 0)
                cps.push_back(cp);
        std::ranges::sort(cps);
        auto dup = std::ranges::unique(cps);
        cps.erase(dup.begin(), dup.end());
        return cps;
    };

    describe({.conversion = "UTF-32 → EUC-JP", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("EUC-JP encode: every code point in index jis0208 once")
    (Catch::Benchmark::Chronometer meter) {
        auto cps = distinct(detail::tables::shift_jis);
        record_input("index jis0208 code points", cps.size() * sizeof(char32_t));
        measure(meter, [&] { return count_elements(cps | whatwg::encode<whatwg::encoding::euc_jp>); });
    };

    describe({.conversion = "UTF-32 → EUC-KR", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("EUC-KR encode: every code point in index euc-kr once")
    (Catch::Benchmark::Chronometer meter) {
        auto cps = distinct(detail::tables::euc_kr);
        record_input("index euc-kr code points", cps.size() * sizeof(char32_t));
        measure(meter, [&] { return count_elements(cps | whatwg::encode<whatwg::encoding::euc_kr>); });
    };

    describe({.conversion = "UTF-32 → GBK", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("GBK encode: every code point in index gb18030 once")
    (Catch::Benchmark::Chronometer meter) {
        auto cps = distinct(detail::tables::gbk);
        record_input("index gb18030 code points", cps.size() * sizeof(char32_t));
        measure(meter, [&] { return count_elements(cps | whatwg::encode<whatwg::encoding::gbk>); });
    };

    describe({.conversion = "UTF-32 → Big5", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("Big5 encode: every code point in index Big5 once")
    (Catch::Benchmark::Chronometer meter) {
        auto cps = distinct(detail::tables::big5);
        record_input("index Big5 code points", cps.size() * sizeof(char32_t));
        measure(meter, [&] { return count_elements(cps | whatwg::encode<whatwg::encoding::big5>); });
    };

    // Decoded outside the benchmark body, so the recorded input is the UTF-32
    // that is encoded, not the corpus file it came from.
    std::vector<char32_t> russian;
    for (char32_t cp : corpus_span("ru_mars_windows1251.bin") | whatwg::decode<whatwg::encoding::windows_1251>)
        russian.push_back(cp);

    describe({.conversion = "UTF-32 → windows-1251", .library = "beman.transcode", .api = "whatwg::encode view"});
    BENCHMARK_ADVANCED("Single-byte encode: windows-1251 Russian")
    (Catch::Benchmark::Chronometer meter) {
        const auto& cps = russian;
        record_input("ru_mars_windows1251.bin decoded", cps.size() * sizeof(char32_t));
        // The single-byte encode view is random-access, so counting its
        // elements never dereferences one; summing the bytes forces the lookups.
        measure(meter, [&] {
            std::size_t sum = 0;
            for (char c : cps | whatwg::encode<whatwg::encoding::windows_1251>)
                sum += static_cast<unsigned char>(c);
            return sum;
        });
    };
}
