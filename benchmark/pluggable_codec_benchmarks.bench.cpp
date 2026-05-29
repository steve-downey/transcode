// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/detail/bulk_transcode.hpp>
#include <beman/transcode/detail/bulk_transcode.hpp>
#include <beman/transcode/detail/transcode_view.hpp>
#include <beman/transcode/encode_view.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <catch2/catch_all.hpp>

#include <array>
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

TEST_CASE("Pluggable bulk operations", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK("decode_to<iso_8859_15>: Russian Windows-1251 corpus") {
        return decode_to<codec::iso_8859_15>(corpus_span("ru_mars_windows1251.bin")).size();
    };

    BENCHMARK("encode_to<iso_8859_15>: round-trip Russian corpus") {
        auto cps = decode_to<codec::iso_8859_15>(corpus_span("ru_mars_windows1251.bin"));
        return encode_to<codec::iso_8859_15>(cps).size();
    };

    BENCHMARK("decode_to<utf_8>: English Mars corpus") {
        return decode_to<codec::utf_8>(corpus_span("en_mars_utf8.txt")).size();
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
