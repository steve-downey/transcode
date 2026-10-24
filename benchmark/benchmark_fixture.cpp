// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>

#include <benchmark/benchmark_commit.hpp>

#include <catch2/catch_config.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#if __has_include(<unistd.h>)
    #include <unistd.h>
#endif

namespace beman::transcoding::bench {

namespace {

struct corpus_file {
    std::string content;
    std::string source; // "data", "corpus", or "missing"
};

struct recorder {
    std::vector<benchmark_record>        records;
    std::optional<benchmark_description> pending;
    std::optional<std::size_t>           current;
};

std::mutex& state_mutex() {
    static std::mutex mtx;
    return mtx;
}

recorder& state() {
    static recorder r;
    return r;
}

// Callers hold state_mutex().
void record_current_input(std::string_view input, std::string_view source, std::size_t bytes) {
    recorder& r = state();
    if (!r.current)
        return;
    benchmark_record& rec = r.records[*r.current];
    if (rec.has_input)
        return;
    rec.input        = input;
    rec.input_source = source;
    rec.input_bytes  = bytes;
    rec.has_input    = true;
}

const corpus_file& cached_load(const char* filename) {
    static std::map<std::string, corpus_file> cache;
    std::lock_guard                           lock(state_mutex());

    auto it = cache.find(filename);
    if (it == cache.end()) {
        auto try_read = [](const std::string& path) -> std::string {
            std::ifstream f(path, std::ios::binary);
            if (!f)
                return {};
            return {std::istreambuf_iterator<char>(f), {}};
        };

        // Try generated data dir first, then fall back to checked-in corpus.
        corpus_file file{try_read(std::string(BENCHMARK_DATA_DIR) + "/" + filename), "data"};
        if (file.content.empty())
            file = {try_read(std::string(BENCHMARK_CORPUS_DIR) + "/" + filename), "corpus"};
        if (file.content.empty())
            file.source = "missing";
        it = cache.emplace(filename, std::move(file)).first;
    }
    record_current_input(filename, it->second.source, it->second.content.size());
    return it->second;
}

std::string json_string(std::string_view s) {
    std::string out = "\"";
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20)
                out += std::format("\\u{:04x}", static_cast<unsigned>(c));
            else
                out += c;
        }
    }
    out += '"';
    return out;
}

std::string compiler_id() {
#if defined(__clang__)
    return "clang " __clang_version__;
#elif defined(__GNUC__)
    return "gcc " __VERSION__;
#elif defined(_MSC_VER)
    return std::format("msvc {}", _MSC_FULL_VER);
#else
    return "unknown";
#endif
}

std::vector<std::string> sanitizers() {
    std::vector<std::string> found;
#if defined(__SANITIZE_ADDRESS__)
    found.emplace_back("address");
#elif defined(__has_feature)
    #if __has_feature(address_sanitizer)
    found.emplace_back("address");
    #endif
#endif
#if defined(__SANITIZE_THREAD__)
    found.emplace_back("thread");
#elif defined(__has_feature)
    #if __has_feature(thread_sanitizer)
    found.emplace_back("thread");
    #endif
#endif
    return found;
}

bool optimized() {
#if defined(__OPTIMIZE__)
    return true;
#else
    return false;
#endif
}

std::string host_name() {
#if __has_include(<unistd.h>)
    char buf[256] = {};
    if (::gethostname(buf, sizeof(buf) - 1) == 0)
        return buf;
#endif
    return {};
}

std::string cpu_model() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string   line;
    while (std::getline(cpuinfo, line)) {
        if (line.starts_with("model name")) {
            auto colon = line.find(':');
            if (colon == std::string::npos)
                break;
            auto value = line.substr(colon + 1);
            value.erase(0, value.find_first_not_of(' '));
            return value;
        }
    }
    return {};
}

std::string utc_timestamp() {
    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::format("{:%Y-%m-%dT%H:%M:%SZ}", now);
}

// Where to write the sidecar: $BEMAN_TRANSCODE_BENCH_META if set, otherwise
// beside the first reporter's --out file with its extension replaced by
// .meta.json. Empty when there is nowhere sensible to write.
std::filesystem::path metadata_path(const Catch::IConfig* config) {
    if (const char* env = std::getenv("BEMAN_TRANSCODE_BENCH_META"); env != nullptr && *env != '\0')
        return env;
    const auto* full = dynamic_cast<const Catch::Config*>(config);
    if (full == nullptr)
        return {};
    // The processed specs carry a plain --out as well as reporter::out=FILE.
    // "-" and the %debug/%stdout/%stderr streams are not files.
    for (const auto& spec : full->getProcessedReporterSpecs()) {
        const auto& out = spec.outputFilename;
        if (!out.empty() && out != "-" && !out.starts_with("%"))
            return std::filesystem::path(out).replace_extension(".meta.json");
    }
    return {};
}

class metadata_listener : public Catch::EventListenerBase {
  public:
    using Catch::EventListenerBase::EventListenerBase;

    void benchmarkPreparing(Catch::StringRef name) override;
    void benchmarkEnded(const Catch::BenchmarkStats<>& stats) override;
    void benchmarkFailed(Catch::StringRef name) override;
    void testRunEnded(const Catch::TestRunStats& stats) override;
};

void metadata_listener::benchmarkPreparing(Catch::StringRef name) {
    begin_benchmark(std::string_view(name.data(), name.size()));
}

void metadata_listener::benchmarkEnded(const Catch::BenchmarkStats<>&) { end_benchmark(); }

void metadata_listener::benchmarkFailed(Catch::StringRef) { end_benchmark(); }

void metadata_listener::testRunEnded(const Catch::TestRunStats&) {
    const auto& records = benchmark_records();
    if (records.empty())
        return;
    auto path = metadata_path(m_config);
    if (path.empty())
        return;
    std::ofstream out(path, std::ios::binary);
    out << metadata_json(records);
    if (!out)
        std::fprintf(stderr, "warning: could not write benchmark metadata to %s\n", path.string().c_str());
}

} // namespace

CATCH_REGISTER_LISTENER(metadata_listener)

std::string_view load_corpus(const char* filename) { return cached_load(filename).content; }

std::span<const char> corpus_span(const char* filename) {
    const std::string& s = cached_load(filename).content;
    return {s.data(), s.size()};
}

std::vector<std::string_view> chunk_corpus(std::string_view data, std::size_t chunk_size) {
    if (chunk_size == 0 || data.empty())
        return {};
    std::vector<std::string_view> chunks;
    for (std::size_t offset = 0; offset < data.size(); offset += chunk_size)
        chunks.push_back(data.substr(offset, chunk_size));
    return chunks;
}

void require_corpus(const char* filename) {
    if (cached_load(filename).source == "missing")
        SKIP("corpus file " << filename
                            << " not found; run `uv run python tools/download_benchmark_corpora.py` to fetch it");
}

void describe(const benchmark_description& description) {
    std::lock_guard lock(state_mutex());
    state().pending = description;
}

void record_input(std::string_view label, std::size_t bytes) {
    std::lock_guard lock(state_mutex());
    record_current_input(label, "synthetic", bytes);
}

void begin_benchmark(std::string_view name) {
    std::lock_guard lock(state_mutex());
    recorder&       r = state();

    std::size_t index = 0;
    while (index < r.records.size() && r.records[index].name != name)
        ++index;
    if (index == r.records.size()) {
        r.records.emplace_back();
        r.records.back().name = name;
    }

    if (r.pending) {
        benchmark_record& rec = r.records[index];
        rec.conversion        = r.pending->conversion;
        rec.library           = r.pending->library;
        rec.api               = r.pending->api;
        r.pending.reset();
    }
    r.current = index;
}

void end_benchmark() {
    std::lock_guard lock(state_mutex());
    state().current.reset();
}

const std::vector<benchmark_record>& benchmark_records() { return state().records; }

void clear_benchmark_records() {
    std::lock_guard lock(state_mutex());
    state() = recorder{};
}

std::string metadata_json(const std::vector<benchmark_record>& records) {
    std::string out = "{\n";
    out += "  \"format\": \"beman.transcode.bench-meta/1\",\n";
    out += "  \"environment\": {\n";
    out += "    \"timestamp\": " + json_string(utc_timestamp()) + ",\n";
    out += "    \"host\": " + json_string(host_name()) + ",\n";
    out += "    \"cpu\": " + json_string(cpu_model()) + ",\n";
    out += "    \"compiler\": " + json_string(compiler_id()) + ",\n";
    out += "    \"build_config\": " + json_string(BENCHMARK_BUILD_CONFIG) + ",\n";
    out += "    \"commit\": " + json_string(BENCHMARK_GIT_COMMIT) + ",\n";
    out += std::string("    \"optimized\": ") + (optimized() ? "true" : "false") + ",\n";
    out += "    \"sanitizers\": [";
    auto sans = sanitizers();
    for (std::size_t i = 0; i < sans.size(); ++i)
        out += (i ? ", " : "") + json_string(sans[i]);
    out += "],\n";
    out +=
        std::format("    \"catch2\": \"{}.{}.{}\"\n", CATCH_VERSION_MAJOR, CATCH_VERSION_MINOR, CATCH_VERSION_PATCH);
    out += "  },\n";
    out += "  \"benchmarks\": [";
    for (std::size_t i = 0; i < records.size(); ++i) {
        const auto& rec = records[i];
        out += i ? ",\n" : "\n";
        out += "    {\"name\": " + json_string(rec.name);
        out += ", \"conversion\": " + json_string(rec.conversion);
        out += ", \"library\": " + json_string(rec.library);
        out += ", \"api\": " + json_string(rec.api);
        if (rec.has_input) {
            out += ", \"input\": " + json_string(rec.input);
            out += ", \"input_source\": " + json_string(rec.input_source);
            out += std::format(", \"input_bytes\": {}", rec.input_bytes);
        }
        out += "}";
    }
    out += records.empty() ? "]\n" : "\n  ]\n";
    out += "}\n";
    return out;
}

} // namespace beman::transcoding::bench
