// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_fixture.hpp>

#include <benchmark/benchmark_sink.hpp>
#include <benchmark/benchmark_sink.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

using beman::transcoding::bench::begin_benchmark;
using beman::transcoding::bench::benchmark_records;
using beman::transcoding::bench::chunk_corpus;
using beman::transcoding::bench::clear_benchmark_records;
using beman::transcoding::bench::corpus_span;
using beman::transcoding::bench::count_elements;
using beman::transcoding::bench::describe;
using beman::transcoding::bench::end_benchmark;
using beman::transcoding::bench::load_corpus;
using beman::transcoding::bench::metadata_json;
using beman::transcoding::bench::record_input;
using beman::transcoding::bench::require_corpus;
using beman::transcoding::bench::volatile_sink;
using beman::transcoding::tests::constify;

TEST_CASE("chunk_corpus splits data into fixed-size chunks", "[bench][fixture]") {
    SECTION("exact multiple") {
        auto chunks = chunk_corpus("abcd", 2);
        REQUIRE(chunks.size() == 2u);
        CHECK(chunks[0] == "ab");
        CHECK(chunks[1] == "cd");
    }

    SECTION("non-multiple — last chunk is smaller") {
        auto chunks = chunk_corpus("abcde", 2);
        REQUIRE(chunks.size() == 3u);
        CHECK(chunks[0] == "ab");
        CHECK(chunks[1] == "cd");
        CHECK(chunks[2] == "e");
    }

    SECTION("chunk larger than data — one chunk") {
        auto chunks = chunk_corpus("abc", 10);
        REQUIRE(chunks.size() == 1u);
        CHECK(chunks[0] == "abc");
    }

    SECTION("single-byte chunks") {
        auto chunks = chunk_corpus("abc", 1);
        REQUIRE(chunks.size() == 3u);
        CHECK(chunks[0] == "a");
        CHECK(chunks[1] == "b");
        CHECK(chunks[2] == "c");
    }

    SECTION("empty data returns empty result") { CHECK(chunk_corpus("", 2).empty()); }

    SECTION("zero chunk_size returns empty result") { CHECK(chunk_corpus("abc", 0).empty()); }
}

TEST_CASE("load_corpus loads checked-in fallback corpus", "[bench][fixture]") {
    auto data = load_corpus("en_mars_utf8.txt");
    CHECK(!data.empty());
}

TEST_CASE("load_corpus returns empty for nonexistent file", "[bench][fixture]") {
    auto data = load_corpus("nonexistent_corpus_file_xyz.txt");
    CHECK(data.empty());
}

TEST_CASE("load_corpus is cached — same pointer on repeated calls", "[bench][fixture]") {
    auto sv1 = load_corpus("en_mars_utf8.txt");
    auto sv2 = load_corpus("en_mars_utf8.txt");
    CHECK(sv1.data() == sv2.data());
}

TEST_CASE("corpus_span returns span over same data as load_corpus", "[bench][fixture]") {
    auto sv = load_corpus("en_mars_utf8.txt");
    auto sp = corpus_span("en_mars_utf8.txt");
    CHECK(sp.size() == sv.size());
    CHECK(sp.data() == sv.data());
}

TEST_CASE("count_elements counts range elements", "[bench][sink]") {
    std::vector<int> v = {1, 2, 3, 4, 5};
    CHECK(count_elements(v) == 5u);
}

TEST_CASE("count_elements returns zero for empty range", "[bench][sink]") {
    std::vector<int> v;
    CHECK(count_elements(v) == 0u);
}

TEST_CASE("count_elements is usable at compile time", "[bench][sink]") {
    constexpr std::array<int, 4> arr = {10, 20, 30, 40};
    CHECK(constify(count_elements(arr)) == 4u);
}

TEST_CASE("volatile_sink accepts values without crashing", "[bench][sink]") {
    int         x = 42;
    std::string s = "hello";
    volatile_sink(x);
    volatile_sink(s);
}

TEST_CASE("describe applies to the next benchmark only", "[bench][metadata]") {
    clear_benchmark_records();
    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg_decode view"});
    begin_benchmark("first");
    end_benchmark();
    begin_benchmark("second");
    end_benchmark();

    const auto& records = benchmark_records();
    REQUIRE(records.size() == 2u);
    CHECK(records[0].name == "first");
    CHECK(records[0].conversion == "UTF-8 → UTF-32");
    CHECK(records[0].library == "beman.transcode");
    CHECK(records[0].api == "whatwg_decode view");
    CHECK(records[1].name == "second");
    CHECK(records[1].conversion.empty());
    clear_benchmark_records();
}

TEST_CASE("a running benchmark records the corpus it reads", "[bench][metadata]") {
    clear_benchmark_records();
    begin_benchmark("reads corpus");
    auto data = corpus_span("en_mars_utf8.txt");
    corpus_span("ar_mars_utf8.txt"); // only the first input is recorded
    end_benchmark();
    corpus_span("ja_mars_utf8.txt"); // outside any benchmark: not recorded

    const auto& records = benchmark_records();
    REQUIRE(records.size() == 1u);
    CHECK(records[0].has_input);
    CHECK(records[0].input == "en_mars_utf8.txt");
    CHECK(records[0].input_bytes == data.size());
    CHECK((records[0].input_source == "data" || records[0].input_source == "corpus"));
    clear_benchmark_records();
}

TEST_CASE("a missing corpus is recorded as missing with zero bytes", "[bench][metadata]") {
    clear_benchmark_records();
    begin_benchmark("reads missing corpus");
    load_corpus("nonexistent_corpus_file_xyz.txt");
    end_benchmark();

    const auto& records = benchmark_records();
    REQUIRE(records.size() == 1u);
    CHECK(records[0].input_source == "missing");
    CHECK(records[0].input_bytes == 0u);
    clear_benchmark_records();
}

TEST_CASE("record_input records a synthetic input", "[bench][metadata]") {
    clear_benchmark_records();
    begin_benchmark("synthetic");
    record_input("4K × 0xC0", 4096);
    end_benchmark();

    const auto& records = benchmark_records();
    REQUIRE(records.size() == 1u);
    CHECK(records[0].input == "4K × 0xC0");
    CHECK(records[0].input_source == "synthetic");
    CHECK(records[0].input_bytes == 4096u);
    clear_benchmark_records();
}

TEST_CASE("a benchmark run twice keeps one record", "[bench][metadata]") {
    clear_benchmark_records();
    describe({.conversion = "c", .library = "l", .api = "a"});
    begin_benchmark("repeated");
    end_benchmark();
    begin_benchmark("repeated");
    end_benchmark();

    const auto& records = benchmark_records();
    REQUIRE(records.size() == 1u);
    CHECK(records[0].conversion == "c");
    clear_benchmark_records();
}

TEST_CASE("metadata_json escapes strings and lists every record", "[bench][metadata]") {
    clear_benchmark_records();
    describe({.conversion = "a \"quoted\" → b", .library = "back\\slash", .api = "api"});
    begin_benchmark("with input");
    record_input("synthetic", 7);
    end_benchmark();
    begin_benchmark("without input");
    end_benchmark();

    std::string json = metadata_json(benchmark_records());
    CHECK(json.find("\"format\": \"beman.transcode.bench-meta/1\"") != std::string::npos);
    CHECK(json.find("\"conversion\": \"a \\\"quoted\\\" → b\"") != std::string::npos);
    CHECK(json.find("\"library\": \"back\\\\slash\"") != std::string::npos);
    CHECK(json.find("\"input_bytes\": 7") != std::string::npos);
    CHECK(json.find("{\"name\": \"without input\", \"conversion\": \"\", \"library\": \"\", \"api\": \"\"}") !=
          std::string::npos);
    CHECK(json.find("\"build_config\": ") != std::string::npos);
    CHECK(json.find("\"commit\": ") != std::string::npos);
    clear_benchmark_records();
}

TEST_CASE("require_corpus does not skip when the corpus exists", "[bench][metadata]") {
    require_corpus("en_mars_utf8.txt");
    SUCCEED();
}

TEST_CASE("require_corpus skips when the corpus is missing", "[bench][metadata]") {
    // Reported as skipped when require_corpus works; the FAIL is reached
    // only if it returns.
    require_corpus("nonexistent_corpus_file_xyz.txt");
    FAIL("require_corpus returned for a missing corpus");
}
