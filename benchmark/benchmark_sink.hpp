// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BENCHMARK_BENCHMARK_SINK_HPP
#define BENCHMARK_BENCHMARK_SINK_HPP

#include <cstddef>

namespace beman::transcoding::bench {

// Store a value where the compiler cannot prove it is unused, preventing
// dead-code elimination of the computation that produced it.
template <typename T>
void volatile_sink(const T& value) {
    // Write the address to a volatile pointer to prevent DCE of value's
    // computation. The volatile read-back silences "set but not used".
    static const T* volatile p;
    p = &value;
    (void)p;
}

// Count elements in a range. Returns the count as the benchmark result,
// preventing DCE of the range traversal.
template <typename Range>
constexpr std::size_t count_elements(Range&& r) {
    std::size_t n = 0;
    for ([[maybe_unused]] auto&& x : r)
        ++n;
    return n;
}

} // namespace beman::transcoding::bench

#endif // BENCHMARK_BENCHMARK_SINK_HPP
