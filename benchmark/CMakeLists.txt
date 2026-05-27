# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

add_executable(beman.transcode.benchmarks.smoke)
target_sources(beman.transcode.benchmarks.smoke PRIVATE smoke.bench.cpp)
target_link_libraries(
    beman.transcode.benchmarks.smoke
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
