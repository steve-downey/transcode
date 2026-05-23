# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

add_executable(beman.transcode.tests.todo)
target_sources(beman.transcode.tests.todo PRIVATE todo.test.cpp)
target_link_libraries(
    beman.transcode.tests.todo
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.error)
target_sources(beman.transcode.tests.error PRIVATE error.test.cpp)
target_link_libraries(
    beman.transcode.tests.error
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(beman.transcode.tests.todo)
catch_discover_tests(beman.transcode.tests.error)
