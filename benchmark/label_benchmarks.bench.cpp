// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The cost of naming an encoding at run time.  A program that takes its
// encoding from a Content-Type header or a command-line option pays for
// get_encoding(label).  These benchmarks time the lookup alone; the switch
// from the codec value to the codec's template instantiation, and the whole
// runtime path against fixed codecs, are in dispatch_benchmarks.bench.cpp.

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <beman/transcode/label.hpp>
#include <beman/transcode/codec.hpp>
#include <beman/transcode/detail/labels.hpp>

#include <catch2/benchmark/catch_optimizer.hpp>
#include <catch2/catch_all.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using beman::transcoding::codec;

// Hide a value from the optimizer.  get_encoding is constexpr, so a label the
// compiler can see is folded to its answer and the benchmark times nothing.
template <typename T>
const T& opaque(T& value) {
    Catch::Benchmark::keep_memory(&value);
    return value;
}

std::optional<codec> lookup(std::string& label) { return beman::transcoding::get_encoding(opaque(label)); }

} // namespace

TEST_CASE("Label lookup benchmarks", "[benchmark][whatwg][label]") {
    using namespace beman::transcoding;
    using namespace beman::transcoding::bench;

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
}
