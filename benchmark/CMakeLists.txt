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

add_executable(beman.transcode.benchmarks.whatwg)
target_sources(
    beman.transcode.benchmarks.whatwg
    PRIVATE whatwg_benchmarks.bench.cpp
)
target_link_libraries(
    beman.transcode.benchmarks.whatwg
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
)

add_executable(beman.transcode.benchmarks.pluggable)
target_sources(
    beman.transcode.benchmarks.pluggable
    PRIVATE pluggable_codec_benchmarks.bench.cpp
)
target_link_libraries(
    beman.transcode.benchmarks.pluggable
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
)

find_package(Iconv REQUIRED)

add_executable(beman.transcode.benchmarks.iconv)
target_sources(
    beman.transcode.benchmarks.iconv
    PRIVATE iconv_benchmarks.bench.cpp
)
target_link_libraries(
    beman.transcode.benchmarks.iconv
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
        Iconv::Iconv
)

# std::codecvt negative baseline.  No extra library needed — it is a standard
# library feature (deprecated in C++17, guarded by __has_include in source).
add_executable(beman.transcode.benchmarks.codecvt)
target_sources(
    beman.transcode.benchmarks.codecvt
    PRIVATE codecvt_benchmarks.bench.cpp
)
target_link_libraries(
    beman.transcode.benchmarks.codecvt
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
)

# encoding_rs baseline — optional, requires Rust/Cargo.
# The Rust crate at benchmark/encoding_rs_bench/ wraps encoding_rs behind a
# plain C FFI and is compiled as a static library.  Cargo must be on PATH.
find_package(Threads REQUIRED)
find_program(CARGO_EXECUTABLE cargo)
if(CARGO_EXECUTABLE)
    message(
        STATUS
        "cargo found: ${CARGO_EXECUTABLE} — enabling encoding_rs benchmarks"
    )

    set(_enc_rs_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/encoding_rs_bench")
    set(_enc_rs_tgt_dir "${CMAKE_CURRENT_BINARY_DIR}/encoding_rs_bench_target")
    set(_enc_rs_lib "${_enc_rs_tgt_dir}/release/libencoding_rs_bench.a")

    add_custom_command(
        OUTPUT "${_enc_rs_lib}"
        COMMAND
            "${CARGO_EXECUTABLE}" build --release --target-dir
            "${_enc_rs_tgt_dir}" --manifest-path "${_enc_rs_src_dir}/Cargo.toml"
        DEPENDS "${_enc_rs_src_dir}/Cargo.toml" "${_enc_rs_src_dir}/src/lib.rs"
        COMMENT "Building encoding_rs_bench Rust crate"
        VERBATIM
    )
    add_custom_target(encoding_rs_bench_build DEPENDS "${_enc_rs_lib}")

    add_library(encoding_rs_bench_imported STATIC IMPORTED)
    set_target_properties(
        encoding_rs_bench_imported
        PROPERTIES IMPORTED_LOCATION "${_enc_rs_lib}"
    )
    add_dependencies(encoding_rs_bench_imported encoding_rs_bench_build)

    add_executable(beman.transcode.benchmarks.encoding_rs)
    target_sources(
        beman.transcode.benchmarks.encoding_rs
        PRIVATE encoding_rs_benchmarks.bench.cpp
    )
    target_include_directories(
        beman.transcode.benchmarks.encoding_rs
        PRIVATE "${_enc_rs_src_dir}/include"
    )
    target_link_libraries(
        beman.transcode.benchmarks.encoding_rs
        PRIVATE
            beman::transcode
            beman.transcode.benchmark_fixture
            Catch2::Catch2WithMain
            encoding_rs_bench_imported
            # Rust static libs on Linux require these system libraries.
            ${CMAKE_DL_LIBS}
            Threads::Threads
    )
    add_dependencies(
        beman.transcode.benchmarks.encoding_rs
        encoding_rs_bench_build
    )
else()
    message(STATUS "cargo not found — skipping encoding_rs benchmarks")
endif()
