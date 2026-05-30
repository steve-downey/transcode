// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Standard library transcoding baseline benchmarks:
//   - mbstowcs (C89, still current): locale-dependent multibyte→wide conversion
//   - std::codecvt / std::wstring_convert (deprecated C++17, removed C++26)
//
// Platform notes:
//   - libstdc++ (GCC): <codecvt> present through at least C++23; may warn with
//     -Wdeprecated-declarations (suppressed here via diagnostic pragmas).
//   - libc++ (Clang): <codecvt> removed as of libc++ 16; SKIP() path is taken.
//   - MSVC STL: deprecated but present through at least VS 2022.

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_sink.hpp>

#include <catch2/catch_all.hpp>

#include <clocale>
#include <cstdlib>
#include <cstring>
#include <vector>

TEST_CASE("mbstowcs baselines", "[benchmark][codecvt]") {
    using namespace beman::transcoding::bench;

    const char* prev_locale = std::setlocale(LC_CTYPE, "en_US.UTF-8");
    if (prev_locale == nullptr) {
        SKIP("en_US.UTF-8 locale not available");
        return;
    }

    BENCHMARK_ADVANCED("mbstowcs UTF-8→wchar_t: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto                 span = corpus_span("en_mars_utf8.txt");
        std::string          input(span.data(), span.size());
        std::vector<wchar_t> output(input.size() + 1);
        meter.measure([&] { return std::mbstowcs(output.data(), input.c_str(), output.size()); });
    };

    BENCHMARK_ADVANCED("mbstowcs UTF-8→wchar_t: Japanese (CJK-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto                 span = corpus_span("ja_mars_utf8.txt");
        std::string          input(span.data(), span.size());
        std::vector<wchar_t> output(input.size() + 1);
        meter.measure([&] { return std::mbstowcs(output.data(), input.c_str(), output.size()); });
    };

    std::setlocale(LC_CTYPE, prev_locale);
}

#if __has_include(<codecvt>)

    // Suppress deprecation warnings: the whole point of this benchmark is to
    // measure the deprecated API as a historical negative baseline.
    #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #elif defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #endif

    #include <codecvt>
    #include <locale>
    #include <string>

TEST_CASE("codecvt baselines", "[benchmark][codecvt]") {
    using namespace beman::transcoding::bench;

    BENCHMARK_ADVANCED("codecvt UTF-8→char32_t: English (ASCII-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto                                                        span = corpus_span("en_mars_utf8.txt");
        std::string                                                 input(span.data(), span.size());
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        meter.measure([&] { return converter.from_bytes(input).size(); });
    };

    BENCHMARK_ADVANCED("codecvt UTF-8→char32_t: Japanese (CJK-heavy)")
    (Catch::Benchmark::Chronometer meter) {
        auto                                                        span = corpus_span("ja_mars_utf8.txt");
        std::string                                                 input(span.data(), span.size());
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        meter.measure([&] { return converter.from_bytes(input).size(); });
    };
}

    #if defined(__clang__)
        #pragma clang diagnostic pop
    #elif defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif

#else // !__has_include(<codecvt>)

TEST_CASE("codecvt baselines (skipped: <codecvt> not available)", "[benchmark][codecvt]") {
    SKIP("<codecvt> header is absent on this platform/STL; benchmark skipped");
}

#endif // __has_include(<codecvt>)
