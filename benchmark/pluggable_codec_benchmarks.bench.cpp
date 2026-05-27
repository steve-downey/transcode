// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>
#include <beman/transcode/transcode.hpp>
#include <beman/transcode/transcode.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <span>
#include <string>

namespace {

inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = beman::transcoding::table_codec<latin1_upper>;

const std::string synthetic(4096, '\xC0');

} // namespace

TEST_CASE("Pluggable codec benchmarks", "[benchmark][pluggable]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    BENCHMARK("pluggable table_codec: 4K upper-half") {
        return count_elements(std::span<const char>(synthetic) | decode(latin1_codec{}));
    };

    BENCHMARK("whatwg iso-8859-15: 4K upper-half") {
        return count_elements(std::span<const char>(synthetic) | whatwg_decode<codec::iso_8859_15>);
    };
}
