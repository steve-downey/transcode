// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BENCHMARK_BENCHMARK_FIXTURE_HPP
#define BENCHMARK_BENCHMARK_FIXTURE_HPP

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace beman::transcoding::bench {

// Load a corpus file. Tries BENCHMARK_DATA_DIR/<name> first (larger generated
// files), then falls back to BENCHMARK_CORPUS_DIR/<name> (checked-in small
// files). Caches result in static storage (loaded at most once per process).
std::string_view load_corpus(const char* filename);

// Same as load_corpus but returns span<const char> for direct piping to views.
std::span<const char> corpus_span(const char* filename);

// Split data into fixed-size chunks. The last chunk may be smaller.
// Returns empty if data is empty or chunk_size is zero.
std::vector<std::string_view> chunk_corpus(std::string_view data, std::size_t chunk_size);

} // namespace beman::transcoding::bench

#endif // BENCHMARK_BENCHMARK_FIXTURE_HPP
