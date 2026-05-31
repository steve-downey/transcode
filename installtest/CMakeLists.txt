# installtest/CMakeLists.txt                                        -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30)
project(TestInstalledTranscode)

enable_testing()

find_package(beman.transcode REQUIRED)

# Test 1: Header mode (C++23)
set(CMAKE_CXX_STANDARD 23)
add_executable(TestInstalledLibrary test.cpp)
target_link_libraries(TestInstalledLibrary beman::transcode)
add_test(NAME RunInstalledTest COMMAND TestInstalledLibrary)

# Test 2: Module mode (C++23) — only if the installed package supports modules
get_target_property(BEMAN_TRANSCODE_TARGET_TYPE beman::transcode TYPE)
if(BEMAN_TRANSCODE_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
    # Package was built with modules enabled
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    add_executable(TestInstalledModule test_module.cpp)
    target_link_libraries(TestInstalledModule beman::transcode)
    add_test(NAME RunInstalledModuleTest COMMAND TestInstalledModule)
endif()

set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
