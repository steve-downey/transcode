// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The overhead of choosing the encoding at run time.
//
// whatwg::get_encoding returns an encoding value, one type for every encoding,
// and the views take their encoding as a template argument, so a runtime choice
// has to pass through a switch from the value to the template instantiation.
// whatwg::transcode_string holds that switch.  Each conversion here is run three ways
// on the same input:
//
//   fixed codecs  -- transcode_string's body with both encodings as template
//                    arguments, no dispatch: the floor;
//   by codec      -- transcode_string(src, encoding, encoding): adds the
//                    switch;
//   by label      -- transcode_string(src, label, label): adds two
//                    get_encoding lookups on top.
//
// The difference from the floor is the cost of choosing late.  It is paid once
// per call, not per element, so it is reported against a short input, where it
// is the largest share, and against a whole corpus, where it is amortized.
// Wall time cannot resolve a difference this small; `make
// bench-dispatch-instructions` counts instructions per call under callgrind
// and tabulates each runtime path against the floor.

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_measure.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/transcode_string.hpp>

#include <catch2/benchmark/catch_optimizer.hpp>
#include <catch2/catch_all.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace {

namespace whatwg = beman::transcoding::whatwg;
using whatwg::encoding;

// Hide a value from the optimizer.  get_encoding is constexpr and
// transcode_string's switch is inlinable, so a label or encoding the compiler
// can see is folded to its answer and the benchmark measures nothing.
template <typename T>
const T& opaque(T& value) {
    Catch::Benchmark::keep_memory(&value);
    return value;
}

// What transcode_string does once it has both encodings, with no dispatch.
template <encoding From, encoding To>
std::string transcode_fixed(std::span<const char> src) {
    return whatwg::detail::transcode_encode_all<To>(whatwg::detail::transcode_decode_all<From>(src));
}

// The longest prefix of at most n bytes that ends on a character boundary, so
// the short input decodes without an error at its end.
std::size_t single_byte_prefix(std::span<const char>, std::size_t n) { return n; }

std::size_t utf_8_prefix(std::span<const char> bytes, std::size_t n) {
    // Back up over continuation bytes to the start of the character at n.
    while (n > 0 && n < bytes.size() && (static_cast<unsigned char>(bytes[n]) & 0xC0) == 0x80)
        --n;
    return n;
}

std::size_t shift_jis_prefix(std::span<const char> bytes, std::size_t n) {
    // Walk forward: a lead byte in 0x81-0x9F or 0xE0-0xFC takes a trail byte.
    std::size_t i = 0;
    while (i < bytes.size()) {
        const auto  b     = static_cast<unsigned char>(bytes[i]);
        std::size_t width = ((b >= 0x81 && b <= 0x9F) || (b >= 0xE0 && b <= 0xFC)) ? 2 : 1;
        if (i + width > n)
            break;
        i += width;
    }
    return i;
}

// A header-field-sized piece of input.
constexpr std::size_t short_size = 32;

struct conversion {
    const char* name;       // "windows-1251 → UTF-8, Russian"
    const char* corpus;     // corpus file
    const char* from_label; // labels for the by-label overload
    const char* to_label;
    std::size_t (*prefix)(std::span<const char>, std::size_t);
};

// The three ways of running conversion c, on a short input and on its whole
// corpus.  From and To are template arguments so the floor calls
// transcode_fixed directly, as code that knows its encodings would.
template <encoding From, encoding To>
void dispatch_benchmarks(const conversion& c) {
    using namespace beman::transcoding::bench;

    const std::span<const char> corpus = corpus_span(c.corpus);
    const std::string short_label      = std::string{c.corpus} + ", first " + std::to_string(short_size) + " bytes";

    struct input {
        const char*           size_name;
        std::string           label;
        std::span<const char> bytes;
    };
    const input inputs[] = {
        {"short", short_label, corpus.first(c.prefix(corpus, std::min(corpus.size(), short_size)))},
        {"whole corpus", c.corpus, corpus},
    };

    for (const input& in : inputs) {
        const std::string suffix = std::string{": "} + c.name + ", " + in.size_name;

        describe({.conversion = c.name, .library = "beman.transcode", .api = "fixed codecs"});
        BENCHMARK_ADVANCED("dispatch fixed codecs" + suffix)
        (Catch::Benchmark::Chronometer meter) {
            record_input(in.label, in.bytes.size());
            measure(meter, [&] { return transcode_fixed<From, To>(in.bytes); });
        };

        describe({.conversion = c.name, .library = "beman.transcode", .api = "by codec"});
        BENCHMARK_ADVANCED("dispatch by codec" + suffix)
        (Catch::Benchmark::Chronometer meter) {
            record_input(in.label, in.bytes.size());
            encoding from = From;
            encoding to   = To;
            measure(meter, [&] { return whatwg::transcode_string(in.bytes, opaque(from), opaque(to)); });
        };

        describe({.conversion = c.name, .library = "beman.transcode", .api = "by label"});
        BENCHMARK_ADVANCED("dispatch by label" + suffix)
        (Catch::Benchmark::Chronometer meter) {
            record_input(in.label, in.bytes.size());
            std::string from = c.from_label;
            std::string to   = c.to_label;
            measure(meter, [&] { return whatwg::transcode_string(in.bytes, opaque(from), opaque(to)); });
        };
    }
}

} // namespace

TEST_CASE("Runtime codec dispatch benchmarks", "[benchmark][dispatch]") {
    dispatch_benchmarks<encoding::windows_1251, encoding::utf_8>(
        {"windows-1251 → UTF-8, Russian", "ru_mars_windows1251.bin", "windows-1251", "utf-8", single_byte_prefix});
    dispatch_benchmarks<encoding::utf_8, encoding::windows_1251>(
        {"UTF-8 → windows-1251, Russian", "ru_mars_utf8.txt", "utf-8", "windows-1251", utf_8_prefix});
    dispatch_benchmarks<encoding::shift_jis, encoding::utf_8>(
        {"Shift_JIS → UTF-8, Japanese", "ja_mars_shiftjis.bin", "shift_jis", "utf-8", shift_jis_prefix});
}
