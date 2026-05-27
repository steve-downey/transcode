// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <benchmark/benchmark_fixture.hpp>
#include <benchmark/benchmark_fixture.hpp>

#include <benchmark/benchmark_sink.hpp>
#include <benchmark/benchmark_sink.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

using beman::transcoding::bench::chunk_corpus;
using beman::transcoding::bench::corpus_span;
using beman::transcoding::bench::count_elements;
using beman::transcoding::bench::load_corpus;
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
