// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/detail/transcode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

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

    BENCHMARK("pluggable decode: 4K upper-half bytes -> char32_t") {
        return count_elements(std::span<const char>(synthetic) | decode(latin1_codec{}));
    };

    BENCHMARK("pluggable encode: 4K upper-half codepoints -> bytes") {
        return count_elements(std::span<const char32_t>(synthetic_cp) | encode(latin1_codec{}));
    };

    BENCHMARK("whatwg iso-8859-15 decode: 4K upper-half (comparison)") {
        return count_elements(std::span<const char>(synthetic) | whatwg_decode<codec::iso_8859_15>);
    };
}

TEST_CASE("Bulk via view|to", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK("view|to decode<iso_8859_15>: Russian Windows-1251 corpus") {
        return (corpus_span("ru_mars_windows1251.bin") | whatwg_decode<codec::iso_8859_15> |
                std::ranges::to<std::vector<char32_t>>())
            .size();
    };

    BENCHMARK("view|to encode<iso_8859_15>: round-trip Russian corpus") {
        auto cps = corpus_span("ru_mars_windows1251.bin") | whatwg_decode<codec::iso_8859_15> |
                   std::ranges::to<std::vector<char32_t>>();
        return (cps | whatwg_encode<codec::iso_8859_15> | std::ranges::to<std::string>()).size();
    };

    BENCHMARK("view|to decode<utf_8>: English Mars corpus") {
        return (corpus_span("en_mars_utf8.txt") | whatwg_decode<codec::utf_8> |
                std::ranges::to<std::vector<char32_t>>())
            .size();
    };
}

TEST_CASE("Pluggable transcode view", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK("transcode<windows_1252, utf_8>: 4K upper-half bytes") {
        return count_elements(std::span<const char>(synthetic) | transcode<codec::windows_1252, codec::utf_8>);
    };

    BENCHMARK("transcode<utf_8, windows_1252>: English Mars corpus") {
        return count_elements(corpus_span("en_mars_utf8.txt") | transcode<codec::utf_8, codec::windows_1252>);
    };
}

TEST_CASE("Cross-encoding via view|to", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK("Shift-JIS -> UTF-8 via view|to") {
        auto cps = corpus_span("ja_mars_shiftjis.bin") | whatwg_decode<codec::shift_jis> |
                   std::ranges::to<std::vector<char32_t>>();
        return (cps | whatwg_encode<codec::utf_8> | std::ranges::to<std::string>()).size();
    };

    BENCHMARK("EUC-JP -> Shift-JIS via view|to") {
        auto cps =
            corpus_span("ja_mars_eucjp.bin") | whatwg_decode<codec::euc_jp> | std::ranges::to<std::vector<char32_t>>();
        return (cps | whatwg_encode<codec::shift_jis> | std::ranges::to<std::string>()).size();
    };

    BENCHMARK("Shift-JIS -> UTF-8 streaming (transcode pipe)") {
        return count_elements(corpus_span("ja_mars_shiftjis.bin") | transcode<codec::shift_jis, codec::utf_8>);
    };

    BENCHMARK("EUC-JP -> Shift-JIS streaming (transcode pipe)") {
        return count_elements(corpus_span("ja_mars_eucjp.bin") | transcode<codec::euc_jp, codec::shift_jis>);
    };
}

TEST_CASE("Large corpus cross-encoding via view|to", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK("Shift-JIS -> UTF-8 via view|to: Genji (2.3 MB)") {
        auto cps = corpus_span("genji_monogatari_shiftjis.bin") | whatwg_decode<codec::shift_jis> |
                   std::ranges::to<std::vector<char32_t>>();
        return (cps | whatwg_encode<codec::utf_8> | std::ranges::to<std::string>()).size();
    };

    BENCHMARK("EUC-JP -> Shift-JIS via view|to: Genji (2.3 MB)") {
        auto cps = corpus_span("genji_monogatari_eucjp.bin") | whatwg_decode<codec::euc_jp> |
                   std::ranges::to<std::vector<char32_t>>();
        return (cps | whatwg_encode<codec::shift_jis> | std::ranges::to<std::string>()).size();
    };

    BENCHMARK("GB18030 -> Big5 via view|to: Journey to the West") {
        auto cps = corpus_span("xiyouji_gb18030.bin") | whatwg_decode<codec::gb18030> |
                   std::ranges::to<std::vector<char32_t>>();
        return (cps | whatwg_encode<codec::big5> | std::ranges::to<std::string>()).size();
    };

    BENCHMARK("Shift-JIS -> UTF-8 streaming: Genji (2.3 MB)") {
        return count_elements(corpus_span("genji_monogatari_shiftjis.bin") |
                              transcode<codec::shift_jis, codec::utf_8>);
    };
}
