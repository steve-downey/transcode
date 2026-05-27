// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>

#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace beman::transcoding::bench {

namespace {

std::string& cached_load(const char* filename) {
    static std::mutex              mtx;
    static std::map<std::string, std::string> cache;
    std::lock_guard                lock(mtx);

    auto it = cache.find(filename);
    if (it != cache.end())
        return it->second;

    auto try_read = [](const std::string& path) -> std::string {
        std::ifstream f(path, std::ios::binary);
        if (!f)
            return {};
        return {std::istreambuf_iterator<char>(f), {}};
    };

    // Try generated data dir first, then fall back to checked-in corpus.
    std::string content =
        try_read(std::string(BENCHMARK_DATA_DIR) + "/" + filename);
    if (content.empty())
        content = try_read(std::string(BENCHMARK_CORPUS_DIR) + "/" + filename);

    auto [iter, ok] = cache.emplace(filename, std::move(content));
    return iter->second;
}

} // namespace

std::string_view load_corpus(const char* filename) { return cached_load(filename); }

std::span<const char> corpus_span(const char* filename) {
    const std::string& s = cached_load(filename);
    return {s.data(), s.size()};
}

std::vector<std::string_view> chunk_corpus(std::string_view data,
                                           std::size_t      chunk_size) {
    if (chunk_size == 0 || data.empty())
        return {};
    std::vector<std::string_view> chunks;
    for (std::size_t offset = 0; offset < data.size(); offset += chunk_size)
        chunks.push_back(data.substr(offset, chunk_size));
    return chunks;
}

} // namespace beman::transcoding::bench
