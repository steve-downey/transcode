// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BENCHMARK_BENCHMARK_FIXTURE_HPP
#define BENCHMARK_BENCHMARK_FIXTURE_HPP

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace beman::transcoding::bench {

// Load a corpus file. Tries BENCHMARK_DATA_DIR/<name> first (larger generated
// files), then falls back to BENCHMARK_CORPUS_DIR/<name> (checked-in small
// files). Caches result in static storage (loaded at most once per process).
// When called while a benchmark runs, records the file, which directory it
// came from, and its size as that benchmark's input.
std::string_view load_corpus(const char* filename);

// Same as load_corpus but returns span<const char> for direct piping to views.
std::span<const char> corpus_span(const char* filename);

// Split data into fixed-size chunks. The last chunk may be smaller.
// Returns empty if data is empty or chunk_size is zero.
std::vector<std::string_view> chunk_corpus(std::string_view data, std::size_t chunk_size);

// SKIP the enclosing test case when filename is in neither corpus directory.
// Use at the top of a test case whose benchmarks need the downloaded corpus,
// so a missing file is reported instead of timed as an empty input.
void require_corpus(const char* filename);

// What a benchmark measures, for the report. Benchmarks with the same
// conversion and input are drawn together, so their libraries and APIs can be
// compared directly.
struct benchmark_description {
    std::string_view conversion; // "Shift_JIS → UTF-8": source and target encodings
    std::string_view library;    // whose code converts: "beman.transcode", "iconv", ...
    std::string_view api;        // how it is called: "whatwg_decode view", "iconv_transcode_to", ...
};

// Describe the benchmark declared next. Call it as the statement immediately
// before BENCHMARK or BENCHMARK_ADVANCED, outside the timed body.
void describe(const benchmark_description& description);

// Record the input of the running benchmark when it is not a corpus file.
// Call it from the benchmark body; corpus_span and load_corpus record their
// own inputs.
void record_input(std::string_view label, std::size_t bytes);

// Everything recorded about one benchmark: the description given before it,
// and the input it read.
struct benchmark_record {
    std::string name;
    std::string conversion;
    std::string library;
    std::string api;
    std::string input;        // corpus filename, or the label given to record_input
    std::string input_source; // "data", "corpus", "synthetic", or "missing"
    std::size_t input_bytes = 0;
    bool        has_input   = false;
};

// Recording hooks, driven by the Catch2 listener in benchmark_fixture.cpp.
// They are public so the fixture tests can drive them without running a
// benchmark.
void                                 begin_benchmark(std::string_view name);
void                                 end_benchmark();
const std::vector<benchmark_record>& benchmark_records();
void                                 clear_benchmark_records();

// The sidecar written beside the Catch2 XML: a JSON object holding the build
// environment and one entry per benchmark record.
std::string metadata_json(const std::vector<benchmark_record>& records);

} // namespace beman::transcoding::bench

#endif // BENCHMARK_BENCHMARK_FIXTURE_HPP
