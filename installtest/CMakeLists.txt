# installtest/CMakeLists.txt                                        -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30)
project(TestInstalledTranscode)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

set(TRANSCODE_INSTALL_DIR "../.install/lib/cmake/beman.transcode/")
find_package(
    beman.transcode
    REQUIRED
    PATHS ${TRANSCODE_INSTALL_DIR}
    NO_DEFAULT_PATH
)

add_executable(TestInstalledLibrary test.cpp)

target_link_libraries(TestInstalledLibrary beman::transcode)

add_test(NAME RunInstalledTest COMMAND TestInstalledLibrary)

set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
