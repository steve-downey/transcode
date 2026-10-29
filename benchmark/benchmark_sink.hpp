// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BENCHMARK_BENCHMARK_SINK_HPP
#define BENCHMARK_BENCHMARK_SINK_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace beman::transcoding::bench {

// Store a value where the compiler cannot prove it is unused, preventing
// dead-code elimination of the computation that produced it.
template <typename T>
void volatile_sink(const T& value) {
    // Write the address to a volatile pointer to prevent DCE of value's
    // computation. The volatile read-back silences "set but not used", and
    // clearing it afterwards keeps the static from holding the address of a
    // caller's local once this returns (-Wdangling-pointer).
    static const T* volatile p;
    p = &value;
    (void)p;
    p = nullptr;
}

// Reduce one traversed element to an integer so its value can be folded into
// an accumulator. Handles integral and enumeration elements, std::byte, and
// expected-like elements (the value, or the error).
template <typename T>
constexpr std::uintmax_t element_value(const T& x) {
    if constexpr (requires { x.has_value(); }) {
        if (x.has_value())
            return element_value(*x);
        return element_value(x.error());
    } else if constexpr (std::is_enum_v<T>) {
        return static_cast<std::uintmax_t>(static_cast<std::underlying_type_t<T>>(x));
    } else if constexpr (requires { std::to_integer<unsigned char>(x); }) {
        return std::to_integer<unsigned char>(x);
    } else {
        return static_cast<std::uintmax_t>(x);
    }
}

// Count elements in a range. Returns the count as the benchmark result,
// preventing DCE of the range traversal. Every element's value is also folded
// into an accumulator that escapes through volatile_sink at runtime, so the
// computation that produced each value cannot be deleted either.
template <typename Range>
constexpr std::size_t count_elements(Range&& r) {
    std::size_t    n   = 0;
    std::uintmax_t acc = 0;
    for (auto&& x : r) {
        acc += element_value(x);
        ++n;
    }
    if !consteval {
        volatile_sink(acc);
    }
    return n;
}

} // namespace beman::transcoding::bench

#endif // BENCHMARK_BENCHMARK_SINK_HPP
