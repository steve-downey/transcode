// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_fixture.hpp>

#include <benchmark/benchmark_sink.hpp>
#include <benchmark/benchmark_sink.hpp>

#include <beman/transcode/whatwg/error.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

using beman::transcoding::bench::begin_benchmark;
using beman::transcoding::bench::begin_count;
using beman::transcoding::bench::begin_run;
using beman::transcoding::bench::benchmark_records;
using beman::transcoding::bench::chunk_corpus;
using beman::transcoding::bench::clear_benchmark_records;
using beman::transcoding::bench::corpus_span;
using beman::transcoding::bench::count_elements;
using beman::transcoding::bench::count_runs;
using beman::transcoding::bench::describe;
using beman::transcoding::bench::end_benchmark;
using beman::transcoding::bench::load_corpus;
using beman::transcoding::bench::metadata_json;
using beman::transcoding::bench::parse_count_runs;
using beman::transcoding::bench::parse_load_average;
using beman::transcoding::bench::read_load_average;
using beman::transcoding::bench::record_input;
using beman::transcoding::bench::require_corpus;
using beman::transcoding::bench::set_count_runs;
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

TEST_CASE("count_elements accepts every element type the benchmarks traverse", "[bench][sink]") {
    namespace whatwg = beman::transcoding::whatwg;
    using result_t   = std::expected<char32_t, whatwg::error>;

    constexpr std::array<char32_t, 3>      cps     = {U'a', U'あ', U'\U0001F600'};
    constexpr std::array<char, 3>          chars   = {'a', 'b', 'c'};
    constexpr std::array<unsigned char, 2> uchs    = {0x80, 0xFF};
    constexpr std::array<std::byte, 2>     bytes   = {std::byte{0x01}, std::byte{0xFE}};
    const std::array<result_t, 3>          results = {
        result_t{U'a'}, result_t{std::unexpected(whatwg::error::invalid_byte)}, result_t{U'b'}};

    CHECK(count_elements(cps) == 3u);
    CHECK(count_elements(chars) == 3u);
    CHECK(count_elements(uchs) == 2u);
    CHECK(count_elements(bytes) == 2u);
    CHECK(count_elements(results) == 3u);

    CHECK(constify(count_elements(cps)) == 3u);
    CHECK(constify(count_elements(chars)) == 3u);
    CHECK(constify(count_elements(uchs)) == 2u);
    CHECK(constify(count_elements(bytes)) == 2u);
    CHECK(constify(count_elements(
              std::array<result_t, 2>{result_t{U'a'}, result_t{std::unexpected(whatwg::error::out_of_range)}})) == 2u);
}

TEST_CASE("element_value reduces elements to their value", "[bench][sink]") {
    namespace whatwg = beman::transcoding::whatwg;
    using beman::transcoding::bench::element_value;
    using result_t = std::expected<char32_t, whatwg::error>;

    CHECK(element_value(U'あ') == 0x3042u);
    CHECK(element_value('a') == static_cast<std::uintmax_t>('a'));
    CHECK(element_value(static_cast<unsigned char>(0xFF)) == 0xFFu);
    CHECK(element_value(std::byte{0xFE}) == 0xFEu);
    CHECK(element_value(result_t{U'b'}) == U'b');
    CHECK(element_value(result_t{std::unexpected(whatwg::error::out_of_range)}) ==
          static_cast<std::uintmax_t>(whatwg::error::out_of_range));

    static_assert(element_value(U'あ') == 0x3042u);
    static_assert(element_value(result_t{U'b'}) == U'b');
}

TEST_CASE("volatile_sink accepts values without crashing", "[bench][sink]") {
    int         x = 42;
    std::string s = "hello";
    volatile_sink(x);
    volatile_sink(s);
}

TEST_CASE("describe applies to the next benchmark only", "[bench][metadata]") {
    clear_benchmark_records();
    describe({.conversion = "UTF-8 → UTF-32", .library = "beman.transcode", .api = "whatwg::decode view"});
    begin_benchmark("first");
    end_benchmark();
    begin_benchmark("second");
    end_benchmark();

    const auto& records = benchmark_records();
    REQUIRE(records.size() == 2u);
    CHECK(records[0].name == "first");
    CHECK(records[0].conversion == "UTF-8 → UTF-32");
    CHECK(records[0].library == "beman.transcode");
    CHECK(records[0].api == "whatwg::decode view");
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

TEST_CASE("parse_load_average reads the first three fields of /proc/loadavg", "[bench][metadata]") {
    auto load = parse_load_average("1.43 1.56 1.93 3/8707 2545026");
    REQUIRE(load.has_value());
    CHECK(load->one == 1.43);
    CHECK(load->five == 1.56);
    CHECK(load->fifteen == 1.93);
    CHECK(parse_load_average("0.00 0.01 0.05").has_value());
}

TEST_CASE("parse_load_average rejects text that is not a load average", "[bench][metadata]") {
    CHECK_FALSE(parse_load_average("").has_value());
    CHECK_FALSE(parse_load_average("1.43 1.56").has_value());
    CHECK_FALSE(parse_load_average("busy 1.56 1.93").has_value());
}

TEST_CASE("metadata_json records the machine state", "[bench][metadata]") {
    clear_benchmark_records();
    begin_benchmark("one");
    end_benchmark();

    std::string before = metadata_json(benchmark_records());
    CHECK(before.find("\"load_start\": null") != std::string::npos);
    for (const char* key : {"\"kernel\": ", "\"cpus\": ", "\"governor\": ", "\"turbo\": ", "\"load_end\": "})
        CHECK(before.find(key) != std::string::npos);

    begin_run();
    std::string after = metadata_json(benchmark_records());
    if (read_load_average().has_value())
        CHECK(after.find("\"load_start\": [") != std::string::npos);
    else
        CHECK(after.find("\"load_start\": null") != std::string::npos);
    clear_benchmark_records();
}

TEST_CASE("parse_count_runs reads a positive count", "[bench][count]") {
    CHECK(parse_count_runs("5") == 5u);
    CHECK(parse_count_runs("100") == 100u);
}

TEST_CASE("parse_count_runs turns anything else into 0", "[bench][count]") {
    CHECK(parse_count_runs(nullptr) == 0u);
    CHECK(parse_count_runs("") == 0u);
    CHECK(parse_count_runs("0") == 0u);
    CHECK(parse_count_runs("-1") == 0u);
    CHECK(parse_count_runs("5x") == 0u);
    CHECK(parse_count_runs("many") == 0u);
}

TEST_CASE("begin_count counts each benchmark once", "[bench][count]") {
    clear_benchmark_records();
    set_count_runs(3);
    CHECK(count_runs() == 3u);
    CHECK(begin_count() == 0u); // no benchmark running

    begin_benchmark("first");
    CHECK(begin_count() == 3u);
    CHECK(begin_count() == 0u);
    end_benchmark();

    begin_benchmark("first");
    CHECK(begin_count() == 0u);
    end_benchmark();

    begin_benchmark("second");
    CHECK(begin_count() == 3u);
    end_benchmark();
    clear_benchmark_records();
}

TEST_CASE("begin_count counts nothing outside count mode", "[bench][count]") {
    clear_benchmark_records();
    CHECK(count_runs() == 0u);
    begin_benchmark("timed");
    CHECK(begin_count() == 0u);
    end_benchmark();
    clear_benchmark_records();
}

TEST_CASE("metadata_json records the count mode", "[bench][count]") {
    clear_benchmark_records();
    begin_benchmark("one");
    end_benchmark();
    CHECK(metadata_json(benchmark_records()).find("\"count_runs\": 0\n") != std::string::npos);
    set_count_runs(7);
    CHECK(metadata_json(benchmark_records()).find("\"count_runs\": 7\n") != std::string::npos);
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
