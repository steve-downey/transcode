// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/config_generated.hpp>
#include <beman/transcode/transcode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <ranges>
#include <span>
#include <string>
#include <vector>

namespace {

inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = beman::transcoding::table_codec<latin1_upper>;

const std::string           synthetic(4096, '\xC0');
const std::vector<char32_t> synthetic_cp(4096, U'\x00C0');

} // namespace

TEST_CASE("Pluggable codec streaming views", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    describe({.conversion = "Latin-1 upper half → UTF-32",
              .library    = "beman.transcode",
              .api        = "pluggable decode view (table_codec)"});
    BENCHMARK_ADVANCED("pluggable decode: 4K upper-half bytes -> char32_t")
    (Catch::Benchmark::Chronometer meter) {
        record_input("4K × 0xC0", synthetic.size());
        measure(meter, [&] { return count_elements(std::span<const char>(synthetic) | decode(latin1_codec{})); });
    };

    describe({.conversion = "UTF-32 → Latin-1 upper half",
              .library    = "beman.transcode",
              .api        = "pluggable encode view (table_codec)"});
    BENCHMARK_ADVANCED("pluggable encode: 4K upper-half codepoints -> bytes")
    (Catch::Benchmark::Chronometer meter) {
        record_input("4K × U+00C0", synthetic_cp.size() * sizeof(char32_t));
        measure(meter,
                [&] { return count_elements(std::span<const char32_t>(synthetic_cp) | encode(latin1_codec{})); });
    };

    describe({.conversion = "ISO-8859-15 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    BENCHMARK_ADVANCED("whatwg iso-8859-15 decode: 4K upper-half (comparison)")
    (Catch::Benchmark::Chronometer meter) {
        record_input("4K × 0xC0", synthetic.size());
        measure(meter, [&] {
            return count_elements(std::span<const char>(synthetic) | whatwg::decode<whatwg::encoding::iso_8859_15>);
        });
    };
}

#if BEMAN_TRANSCODE_HAS_RANGES_TO
TEST_CASE("Bulk via view|to", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    describe(
        {.conversion = "ISO-8859-15 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode | ranges::to"});
    BENCHMARK_ADVANCED("view|to decode<iso_8859_15>: Russian Windows-1251 corpus")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return (corpus_span("ru_mars_windows1251.bin") | whatwg::decode<whatwg::encoding::iso_8859_15> |
                    std::ranges::to<std::vector<char32_t>>())
                .size();
        });
    };

    describe({.conversion = "ISO-8859-15 → UTF-32 → ISO-8859-15",
              .library    = "beman.transcode",
              .api        = "decode | to, encode | to"});
    BENCHMARK_ADVANCED("view|to encode<iso_8859_15>: round-trip Russian corpus")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            auto cps = corpus_span("ru_mars_windows1251.bin") | whatwg::decode<whatwg::encoding::iso_8859_15> |
                       std::ranges::to<std::vector<char32_t>>();
            return (cps | whatwg::encode<whatwg::encoding::iso_8859_15> | std::ranges::to<std::string>()).size();
        });
    };

    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode | ranges::to"});
    BENCHMARK_ADVANCED("view|to decode<utf_8>: English Mars corpus")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return (corpus_span("en_mars_utf8.txt") | whatwg::decode<whatwg::encoding::utf_8> |
                    std::ranges::to<std::vector<char32_t>>())
                .size();
        });
    };
}
#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

TEST_CASE("Pluggable transcode view", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    describe({.conversion = "windows-1252 → UTF-8", .library = "beman.transcode", .api = "transcode view"});
    BENCHMARK_ADVANCED("whatwg::transcode<windows_1252, utf_8>: 4K upper-half bytes")
    (Catch::Benchmark::Chronometer meter) {
        record_input("4K × 0xC0", synthetic.size());
        measure(meter, [&] {
            return count_elements(std::span<const char>(synthetic) |
                                  whatwg::transcode<whatwg::encoding::windows_1252, whatwg::encoding::utf_8>);
        });
    };

    describe({.conversion = "UTF-8 → windows-1252", .library = "beman.transcode", .api = "transcode view"});
    BENCHMARK_ADVANCED("whatwg::transcode<utf_8, windows_1252>: English Mars corpus")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return count_elements(corpus_span("en_mars_utf8.txt") |
                                  whatwg::transcode<whatwg::encoding::utf_8, whatwg::encoding::windows_1252>);
        });
    };
}

TEST_CASE("Cross-encoding via view|to", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

#if BEMAN_TRANSCODE_HAS_RANGES_TO
    describe({.conversion = "Shift_JIS → UTF-8", .library = "beman.transcode", .api = "decode | to, encode | to"});
    BENCHMARK_ADVANCED("Shift-JIS -> UTF-8 via view|to")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            auto cps = corpus_span("ja_mars_shiftjis.bin") | whatwg::decode<whatwg::encoding::shift_jis> |
                       std::ranges::to<std::vector<char32_t>>();
            return (cps | whatwg::encode<whatwg::encoding::utf_8> | std::ranges::to<std::string>()).size();
        });
    };

    describe({.conversion = "EUC-JP → Shift_JIS", .library = "beman.transcode", .api = "decode | to, encode | to"});
    BENCHMARK_ADVANCED("EUC-JP -> Shift-JIS via view|to")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            auto cps = corpus_span("ja_mars_eucjp.bin") | whatwg::decode<whatwg::encoding::euc_jp> |
                       std::ranges::to<std::vector<char32_t>>();
            return (cps | whatwg::encode<whatwg::encoding::shift_jis> | std::ranges::to<std::string>()).size();
        });
    };
#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

    describe({.conversion = "Shift_JIS → UTF-8", .library = "beman.transcode", .api = "transcode view"});
    BENCHMARK_ADVANCED("Shift-JIS -> UTF-8 streaming (transcode pipe)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return count_elements(corpus_span("ja_mars_shiftjis.bin") |
                                  whatwg::transcode<whatwg::encoding::shift_jis, whatwg::encoding::utf_8>);
        });
    };

    describe({.conversion = "EUC-JP → Shift_JIS", .library = "beman.transcode", .api = "transcode view"});
    BENCHMARK_ADVANCED("EUC-JP -> Shift-JIS streaming (transcode pipe)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return count_elements(corpus_span("ja_mars_eucjp.bin") |
                                  whatwg::transcode<whatwg::encoding::euc_jp, whatwg::encoding::shift_jis>);
        });
    };
}

TEST_CASE("Large corpus cross-encoding via view|to", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    require_corpus("genji_monogatari_shiftjis.bin");
    require_corpus("genji_monogatari_eucjp.bin");
    require_corpus("xiyouji_gb18030.bin");

#if BEMAN_TRANSCODE_HAS_RANGES_TO
    describe({.conversion = "Shift_JIS → UTF-8", .library = "beman.transcode", .api = "decode | to, encode | to"});
    BENCHMARK_ADVANCED("Shift-JIS -> UTF-8 via view|to: Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            auto cps = corpus_span("genji_monogatari_shiftjis.bin") | whatwg::decode<whatwg::encoding::shift_jis> |
                       std::ranges::to<std::vector<char32_t>>();
            return (cps | whatwg::encode<whatwg::encoding::utf_8> | std::ranges::to<std::string>()).size();
        });
    };

    describe({.conversion = "EUC-JP → Shift_JIS", .library = "beman.transcode", .api = "decode | to, encode | to"});
    BENCHMARK_ADVANCED("EUC-JP -> Shift-JIS via view|to: Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            auto cps = corpus_span("genji_monogatari_eucjp.bin") | whatwg::decode<whatwg::encoding::euc_jp> |
                       std::ranges::to<std::vector<char32_t>>();
            return (cps | whatwg::encode<whatwg::encoding::shift_jis> | std::ranges::to<std::string>()).size();
        });
    };

    describe({.conversion = "GB18030 → Big5", .library = "beman.transcode", .api = "decode | to, encode | to"});
    BENCHMARK_ADVANCED("GB18030 -> Big5 via view|to: Journey to the West")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            auto cps = corpus_span("xiyouji_gb18030.bin") | whatwg::decode<whatwg::encoding::gb18030> |
                       std::ranges::to<std::vector<char32_t>>();
            return (cps | whatwg::encode<whatwg::encoding::big5> | std::ranges::to<std::string>()).size();
        });
    };
#endif // BEMAN_TRANSCODE_HAS_RANGES_TO

    describe({.conversion = "Shift_JIS → UTF-8", .library = "beman.transcode", .api = "transcode view"});
    BENCHMARK_ADVANCED("Shift-JIS -> UTF-8 streaming: Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return count_elements(corpus_span("genji_monogatari_shiftjis.bin") |
                                  whatwg::transcode<whatwg::encoding::shift_jis, whatwg::encoding::utf_8>);
        });
    };

    describe({.conversion = "EUC-JP → Shift_JIS", .library = "beman.transcode", .api = "transcode view"});
    BENCHMARK_ADVANCED("EUC-JP -> Shift-JIS streaming: Genji (2.3 MB)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            return count_elements(corpus_span("genji_monogatari_eucjp.bin") |
                                  whatwg::transcode<whatwg::encoding::euc_jp, whatwg::encoding::shift_jis>);
        });
    };
}
