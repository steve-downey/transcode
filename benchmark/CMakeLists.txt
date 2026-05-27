# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

# Benchmark fixture: corpus loading, chunking helpers, and sink utilities.
# Built as a static library so both benchmark executables and unit tests can
# link against it. The PUBLIC include exposes <benchmark/...> headers to all
# dependents.
add_library(beman.transcode.benchmark_fixture STATIC)
target_sources(beman.transcode.benchmark_fixture PRIVATE benchmark_fixture.cpp)
target_include_directories(
    beman.transcode.benchmark_fixture
    PUBLIC "${PROJECT_SOURCE_DIR}"
)
target_compile_definitions(
    beman.transcode.benchmark_fixture
    PRIVATE
        "BENCHMARK_DATA_DIR=\"${PROJECT_SOURCE_DIR}/data/benchmarks\""
        "BENCHMARK_CORPUS_DIR=\"${PROJECT_SOURCE_DIR}/benchmark/corpus\""
)
target_link_libraries(
    beman.transcode.benchmark_fixture
    PRIVATE beman::transcode
)

add_executable(beman.transcode.benchmarks.smoke)
target_sources(beman.transcode.benchmarks.smoke PRIVATE smoke.bench.cpp)
target_link_libraries(
    beman.transcode.benchmarks.smoke
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
)

add_executable(beman.transcode.benchmarks.utf)
target_sources(beman.transcode.benchmarks.utf PRIVATE utf_benchmarks.bench.cpp)
target_link_libraries(
    beman.transcode.benchmarks.utf
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
)
