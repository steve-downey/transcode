// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The cost of naming an encoding at run time.  A program that takes its
// encoding from a Content-Type header or a command-line option pays for
// whatwg::get_encoding(label) and then for a switch from the encoding value to
// the codec's template instantiation.  These benchmarks time both, alone and
// inside whatwg::transcode_string, against the same conversion with the
// encodings fixed at compile time, so the difference is the overhead of
// choosing late.

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/labels.hpp>
#include <beman/transcode/whatwg/transcode_string.hpp>

#include <catch2/benchmark/catch_optimizer.hpp>
#include <catch2/catch_all.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace whatwg = beman::transcoding::whatwg;
using whatwg::encoding;

// Hide a value from the optimizer.  get_encoding is constexpr and
// transcode_string's switch is inlinable, so a label or encoding the compiler
// can see is folded to its answer and the benchmark times nothing.
template <typename T>
const T& opaque(T& value) {
    Catch::Benchmark::keep_memory(&value);
    return value;
}

std::optional<encoding> lookup(std::string& label) { return whatwg::get_encoding(opaque(label)); }

// What transcode_string does once it has both encodings, with no dispatch.
template <encoding From, encoding To>
std::string transcode_fixed(std::span<const char> src) {
    return whatwg::detail::transcode_encode_all<To>(whatwg::detail::transcode_decode_all<From>(src));
}

} // namespace

TEST_CASE("Label lookup benchmarks", "[benchmark][whatwg][label]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

    // --- get_encoding on its own ---

    describe({.conversion = "label → codec", .library = "beman.transcode", .api = "get_encoding"});
    BENCHMARK_ADVANCED("get_encoding: canonical name (utf-8)")
    (Catch::Benchmark::Chronometer meter) {
        std::string label = "utf-8";
        measure(meter, [&] { return lookup(label); });
    };

    describe({.conversion = "label → codec", .library = "beman.transcode", .api = "get_encoding"});
    BENCHMARK_ADVANCED("get_encoding: alias with case and whitespace ( Latin1 )")
    (Catch::Benchmark::Chronometer meter) {
        std::string label = " \tLatin1\r\n";
        measure(meter, [&] { return lookup(label); });
    };

    describe({.conversion = "label → codec", .library = "beman.transcode", .api = "get_encoding"});
    BENCHMARK_ADVANCED("get_encoding: unknown label")
    (Catch::Benchmark::Chronometer meter) {
        std::string label = "x-no-such-encoding";
        measure(meter, [&] { return lookup(label); });
    };

    // Every label the Encoding Standard defines, so no one position in the
    // table's binary search stands for all of them.  Divide by the label count
    // in the name for the mean cost of one lookup.
    std::vector<std::string> all_labels;
    for (const auto& entry : detail::label_table)
        all_labels.emplace_back(entry.label);

    describe({.conversion = "label → codec", .library = "beman.transcode", .api = "get_encoding"});
    BENCHMARK_ADVANCED("get_encoding: every WHATWG label (" + std::to_string(all_labels.size()) + " lookups)")
    (Catch::Benchmark::Chronometer meter) {
        measure(meter, [&] {
            std::size_t found = 0;
            for (auto& label : all_labels)
                found += lookup(label).has_value();
            return found;
        });
    };

    // --- transcode_string: fixed codecs, runtime codecs, labels ---
    //
    // Each conversion is timed three ways.  The fixed codecs are the floor;
    // the codec overload adds the switch to the codec's instantiation; the
    // label overload adds two get_encoding calls on top.  A short input shows
    // the overhead as a share of a small conversion, where it is largest; the
    // whole corpus shows it amortized.

    const auto corpus = corpus_span("ru_mars_windows1251.bin");
    // A header-field-sized piece.  windows-1251 is single-byte, so any cut is
    // on a character boundary.
    const auto short_input = corpus.first(std::min<std::size_t>(corpus.size(), 32));

    struct input {
        const char*           name;
        std::span<const char> bytes;
    };
    for (const input in : {input{"32 bytes", short_input}, input{"whole corpus", corpus}}) {
        const std::string suffix = std::string{" windows-1251 → UTF-8, Russian, "} + in.name;

        describe({.conversion = "windows-1251 → UTF-8", .library = "beman.transcode", .api = "fixed codecs"});
        BENCHMARK_ADVANCED("transcode_string fixed codecs:" + suffix)
        (Catch::Benchmark::Chronometer meter) {
            measure(meter, [&] { return transcode_fixed<encoding::windows_1251, encoding::utf_8>(in.bytes); });
        };

        describe(
            {.conversion = "windows-1251 → UTF-8", .library = "beman.transcode", .api = "transcode_string codec"});
        BENCHMARK_ADVANCED("transcode_string by codec:" + suffix)
        (Catch::Benchmark::Chronometer meter) {
            encoding from = encoding::windows_1251;
            encoding to   = encoding::utf_8;
            measure(meter, [&] { return whatwg::transcode_string(in.bytes, opaque(from), opaque(to)); });
        };

        describe(
            {.conversion = "windows-1251 → UTF-8", .library = "beman.transcode", .api = "transcode_string label"});
        BENCHMARK_ADVANCED("transcode_string by label:" + suffix)
        (Catch::Benchmark::Chronometer meter) {
            std::string from = "windows-1251";
            std::string to   = "utf-8";
            measure(meter, [&] { return whatwg::transcode_string(in.bytes, opaque(from), opaque(to)); });
        };
    }
}
