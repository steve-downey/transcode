// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BENCHMARK_BENCHMARK_MEASURE_HPP
#define BENCHMARK_BENCHMARK_MEASURE_HPP

#include <benchmark/benchmark_fixture.hpp>

#include <catch2/benchmark/catch_chronometer.hpp>
#include <catch2/benchmark/catch_optimizer.hpp>

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace beman::transcoding::bench {

// Time fn with meter, as meter.measure(fn) does.  Use it for every timed body
// so that count mode can count instructions too.
//
// In count mode (see count_runs), the first call for each benchmark first runs
// fn once untimed and uncounted, to warm up lazy setup such as iconv_open,
// then runs it count_runs() times with callgrind collecting, and dumps the
// counts under the benchmark's name.  Later calls for the same benchmark, and
// every call outside count mode, only time.
template <std::invocable Fn>
void measure(Catch::Benchmark::Chronometer meter, Fn&& fn);

namespace detail {

template <std::invocable Fn>
void invoke_kept(Fn& fn) {
    if constexpr (std::is_void_v<std::invoke_result_t<Fn&>>)
        fn();
    else
        Catch::Benchmark::deoptimize_value(fn());
}

} // namespace detail

template <std::invocable Fn>
void measure(Catch::Benchmark::Chronometer meter, Fn&& fn) {
    if (std::size_t runs = begin_count(); runs != 0) {
        detail::invoke_kept(fn);
        start_counting();
        for (std::size_t i = 0; i < runs; ++i)
            detail::invoke_kept(fn);
        stop_counting();
    }
    meter.measure(std::forward<Fn>(fn));
}

} // namespace beman::transcoding::bench

#endif // BENCHMARK_BENCHMARK_MEASURE_HPP
