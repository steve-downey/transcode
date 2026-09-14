# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

include(infra/cmake/enable-experimental-import-std.cmake)

project(
    beman.transcode
    DESCRIPTION "Transcoding Views and Ranges"
    LANGUAGES CXX
    VERSION 0.1.0
)

if(
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13"
)
    message(
        FATAL_ERROR
        "GCC >= 13 is required. Found: ${CMAKE_CXX_COMPILER_VERSION}"
    )
endif()

# The standard-version flag for the capability probes below.  Written once
# because getting it wrong is invisible: a probe compiled with the wrong flag
# reports "not available" for a facility that is, and the diagnostic blames the
# standard library.
if(MSVC)
    set(_beman_transcode_std_flag "/std:c++latest")
else()
    set(_beman_transcode_std_flag "-std=c++23")
endif()

include(CheckCXXSourceCompiles)

# There used to be a `Clang < 19` fatal error here, with the reason in its own
# message: "18 lacks __cpp_concepts >= 202002L needed by libstdc++
# std::expected".  The reason names a *standard library* interaction, and a
# compiler version test cannot see one -- so it also rejected Clang 18 and 17
# against libc++, two configurations the support matrix in README.md claimed
# and this check made impossible to try.
#
# Probe for the thing that is actually required, as the ranges::to check below
# already does.  This is `std::expected` inside a `std::vector`, which is what
# the _or_error views instantiate and what fails on the combination above.
set(CMAKE_REQUIRED_FLAGS "${_beman_transcode_std_flag}")
check_cxx_source_compiles(
    "#include <expected>
     #include <vector>
     int main() {
         std::vector<std::expected<char, int>> v{std::expected<char, int>{'x'}};
         return v.empty() ? 1 : 0;
     }"
    BEMAN_TRANSCODE_HAS_USABLE_EXPECTED
)
unset(CMAKE_REQUIRED_FLAGS)

if(NOT BEMAN_TRANSCODE_HAS_USABLE_EXPECTED)
    message(
        FATAL_ERROR
        "This project needs a working std::expected usable as a vector element. "
        "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} with the selected "
        "standard library does not provide one. The known case is Clang 18 or "
        "earlier with libstdc++, whose std::expected requires "
        "__cpp_concepts >= 202002L. Use a newer compiler, or select libc++."
    )
endif()

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_TRANSCODE_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_TRANSCODE_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_BENCHMARKS]
option(
    BEMAN_TRANSCODE_BUILD_BENCHMARKS
    "Enable building benchmarks. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    BEMAN_TRANSCODE_USE_MODULES
    "Provide beman.transcode as a C++ module"
    OFF
)

if(BEMAN_TRANSCODE_USE_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

# Probe for std::ranges::to — available in libstdc++ since GCC 14, libc++ since Clang 19.
# Uses the standard feature-test macro rather than a compiler version check so that
# third-party builds with patched toolchains or alternate standard libraries are handled
# correctly.
set(CMAKE_REQUIRED_FLAGS "${_beman_transcode_std_flag}")
check_cxx_source_compiles(
    "#include <ranges>
     #if !defined(__cpp_lib_ranges_to_container) || __cpp_lib_ranges_to_container < 202202L
     #  error ranges::to not available
     #endif
     int main() { return 0; }"
    BEMAN_TRANSCODE_HAS_RANGES_TO
)
unset(CMAKE_REQUIRED_FLAGS)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/beman/transcode/config_generated.hpp.in"
    "${PROJECT_BINARY_DIR}/include/beman/transcode/config_generated.hpp"
    @ONLY
)

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

set(CMAKE_CXX_CLANG_TIDY
    "$<$<CONFIG:Tidy>:clang-tidy;--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy-lint>"
)

if(BEMAN_TRANSCODE_USE_MODULES)
    add_library(beman.transcode STATIC)
else()
    add_library(beman.transcode INTERFACE)
endif()
add_library(beman::transcode ALIAS beman.transcode)

if(BEMAN_TRANSCODE_USE_MODULES)
    target_sources(
        beman.transcode
        PUBLIC
            FILE_SET CXX_MODULES
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(beman.transcode PROPERTIES CXX_MODULE_STD ON)
    target_compile_features(beman.transcode PUBLIC cxx_std_23)
else()
    target_sources(
        beman.transcode
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    target_compile_features(beman.transcode INTERFACE cxx_std_23)
    set_target_properties(
        beman.transcode
        PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
    )
endif()

find_package(Iconv REQUIRED)
if(BEMAN_TRANSCODE_USE_MODULES)
    target_link_libraries(beman.transcode PUBLIC Iconv::Iconv)
else()
    target_link_libraries(beman.transcode INTERFACE Iconv::Iconv)
endif()

add_subdirectory(include/beman/transcode)

beman_install_library(
    beman.transcode
    TARGETS beman.transcode
    DEPENDENCIES Iconv
)
configure_build_telemetry()

if(BEMAN_TRANSCODE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/transcode)
endif()

if(BEMAN_TRANSCODE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BEMAN_TRANSCODE_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

# Coverage
configure_file("cmake/gcovr.cfg.in" gcovr.cfg @ONLY)

add_custom_target(
    process_coverage
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running gcovr to process coverage results"
    COMMAND mkdir -p coverage
    COMMAND gcovr --config gcovr.cfg .
)
