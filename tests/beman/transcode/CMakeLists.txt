# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

# Common include path for test_utilities.hpp (tests/beman/transcode/test_utilities.hpp)
set(TEST_INCLUDE_DIR "${PROJECT_SOURCE_DIR}")

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

add_executable(beman.transcode.tests.concepts)
target_sources(beman.transcode.tests.concepts PRIVATE concepts.test.cpp)
target_link_libraries(
    beman.transcode.tests.concepts
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.null_term)
target_sources(beman.transcode.tests.null_term PRIVATE null_term.test.cpp)
target_include_directories(
    beman.transcode.tests.null_term
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.null_term
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.whatwg_decode)
target_sources(
    beman.transcode.tests.whatwg_decode
    PRIVATE whatwg_decode.test.cpp
)
target_include_directories(
    beman.transcode.tests.whatwg_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.whatwg_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.iconv_transcode)
target_sources(
    beman.transcode.tests.iconv_transcode
    PRIVATE iconv_transcode.test.cpp
)
target_include_directories(
    beman.transcode.tests.iconv_transcode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iconv_transcode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.whatwg_decode_or_error)
target_sources(
    beman.transcode.tests.whatwg_decode_or_error
    PRIVATE whatwg_decode_or_error.test.cpp
)
target_include_directories(
    beman.transcode.tests.whatwg_decode_or_error
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.whatwg_decode_or_error
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.iconv_real)
target_sources(beman.transcode.tests.iconv_real PRIVATE iconv_real.test.cpp)
target_include_directories(
    beman.transcode.tests.iconv_real
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iconv_real
    PRIVATE beman::transcode Catch2::Catch2WithMain Iconv::Iconv
)

add_executable(beman.transcode.tests.iconv_transcode_or_error)
target_sources(
    beman.transcode.tests.iconv_transcode_or_error
    PRIVATE iconv_transcode_or_error.test.cpp
)
target_include_directories(
    beman.transcode.tests.iconv_transcode_or_error
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iconv_transcode_or_error
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.whatwg_encode)
target_sources(
    beman.transcode.tests.whatwg_encode
    PRIVATE whatwg_encode.test.cpp
)
target_include_directories(
    beman.transcode.tests.whatwg_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.whatwg_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.whatwg_encode_or_error)
target_sources(
    beman.transcode.tests.whatwg_encode_or_error
    PRIVATE whatwg_encode_or_error.test.cpp
)
target_include_directories(
    beman.transcode.tests.whatwg_encode_or_error
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.whatwg_encode_or_error
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.utf16_decode)
target_sources(beman.transcode.tests.utf16_decode PRIVATE utf16_decode.test.cpp)
target_include_directories(
    beman.transcode.tests.utf16_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.utf16_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.utf16_encode)
target_sources(beman.transcode.tests.utf16_encode PRIVATE utf16_encode.test.cpp)
target_include_directories(
    beman.transcode.tests.utf16_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.utf16_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.gbk_decode)
target_sources(beman.transcode.tests.gbk_decode PRIVATE gbk_decode.test.cpp)
target_include_directories(
    beman.transcode.tests.gbk_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.gbk_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.gbk_encode)
target_sources(beman.transcode.tests.gbk_encode PRIVATE gbk_encode.test.cpp)
target_include_directories(
    beman.transcode.tests.gbk_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.gbk_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.gb18030_decode)
target_sources(
    beman.transcode.tests.gb18030_decode
    PRIVATE gb18030_decode.test.cpp
)
target_include_directories(
    beman.transcode.tests.gb18030_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.gb18030_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.gb18030_encode)
target_sources(
    beman.transcode.tests.gb18030_encode
    PRIVATE gb18030_encode.test.cpp
)
target_include_directories(
    beman.transcode.tests.gb18030_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.gb18030_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(beman.transcode.tests.todo)
catch_discover_tests(beman.transcode.tests.error)
catch_discover_tests(beman.transcode.tests.concepts)
catch_discover_tests(beman.transcode.tests.null_term)
catch_discover_tests(beman.transcode.tests.whatwg_decode)
catch_discover_tests(beman.transcode.tests.whatwg_decode_or_error)
catch_discover_tests(beman.transcode.tests.iconv_transcode)
catch_discover_tests(beman.transcode.tests.iconv_real)
catch_discover_tests(beman.transcode.tests.iconv_transcode_or_error)
catch_discover_tests(beman.transcode.tests.whatwg_encode)
catch_discover_tests(beman.transcode.tests.whatwg_encode_or_error)
catch_discover_tests(beman.transcode.tests.utf16_decode)
catch_discover_tests(beman.transcode.tests.utf16_encode)
catch_discover_tests(beman.transcode.tests.gbk_decode)
catch_discover_tests(beman.transcode.tests.gbk_encode)
catch_discover_tests(beman.transcode.tests.gb18030_decode)
catch_discover_tests(beman.transcode.tests.gb18030_encode)

# iconv_transcode_or_error iterator is move-only (iconv_t is an uncopyable OS handle).
add_library(
    beman.transcode.tests.iconv_transcode_or_error_iterator_copy_fail
    OBJECT
)
target_sources(
    beman.transcode.tests.iconv_transcode_or_error_iterator_copy_fail
    PRIVATE iconv_transcode_or_error_iterator_copy_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.iconv_transcode_or_error_iterator_copy_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.iconv_transcode_or_error_iterator_copy_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME iconv_transcode_or_error_iterator_copy_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.iconv_transcode_or_error_iterator_copy_fail
        --config $<CONFIGURATION>
)
set_tests_properties(
    iconv_transcode_or_error_iterator_copy_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION
            "transcode: iconv_transcode_or_error iterator is move-only"
)

# iconv_transcode iterator is move-only (iconv_t is an uncopyable OS handle).
add_library(beman.transcode.tests.iconv_transcode_iterator_copy_fail OBJECT)
target_sources(
    beman.transcode.tests.iconv_transcode_iterator_copy_fail
    PRIVATE iconv_transcode_iterator_copy_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.iconv_transcode_iterator_copy_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.iconv_transcode_iterator_copy_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME iconv_transcode_iterator_copy_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.iconv_transcode_iterator_copy_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    iconv_transcode_iterator_copy_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION
            "transcode: iconv_transcode iterator is move-only"
)

# ---------------------------------------------------------------------------
# Negative compile tests: verify that constraints cause compile errors for the
# correct reason, not just any compile error.
# Each target is excluded from the default build and registered as a ctest test
# that is expected to fail (WILL_FAIL), with a PASS_REGULAR_EXPRESSION that
# confirms the failure is due to the intended constraint, not an unrelated error.
# ---------------------------------------------------------------------------

# legacy_byte_range rejects raw arrays (char[N]) via !is_array_v
add_library(beman.transcode.tests.concepts_reject_arrays_fail OBJECT)
target_sources(
    beman.transcode.tests.concepts_reject_arrays_fail
    PRIVATE concepts_reject_arrays_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.concepts_reject_arrays_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.concepts_reject_arrays_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME concepts_reject_arrays_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.concepts_reject_arrays_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    concepts_reject_arrays_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION
            "transcode: raw arrays are not valid input to whatwg_decode"
)

# legacy_byte_range rejects unicode char types (char8_t etc.) via legacy_byte_type disjunction
add_library(beman.transcode.tests.concepts_reject_unicode_chars_fail OBJECT)
target_sources(
    beman.transcode.tests.concepts_reject_unicode_chars_fail
    PRIVATE concepts_reject_unicode_chars_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.concepts_reject_unicode_chars_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.concepts_reject_unicode_chars_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME concepts_reject_unicode_chars_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.concepts_reject_unicode_chars_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    concepts_reject_unicode_chars_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION "no operand of the disjunction is satisfied"
)

# unicode_scalar_range rejects non-char32_t ranges (e.g. vector<char>)
add_library(beman.transcode.tests.whatwg_encode_reject_char_range_fail OBJECT)
target_sources(
    beman.transcode.tests.whatwg_encode_reject_char_range_fail
    PRIVATE whatwg_encode_reject_char_range_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.whatwg_encode_reject_char_range_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.whatwg_encode_reject_char_range_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME whatwg_encode_reject_char_range_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.whatwg_encode_reject_char_range_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    whatwg_encode_reject_char_range_fail
    PROPERTIES PASS_REGULAR_EXPRESSION "unicode_scalar_range"
)

# views::null_term rejects non-pointer ranges via is_pointer_v constraint
add_library(beman.transcode.tests.null_term_reject_range_fail OBJECT)
target_sources(
    beman.transcode.tests.null_term_reject_range_fail
    PRIVATE null_term_reject_range_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.null_term_reject_range_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.null_term_reject_range_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME null_term_reject_range_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.null_term_reject_range_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    null_term_reject_range_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION
            "transcode: views::null_term requires a pointer or array type"
)

# legacy_byte_range rejects char32_t ranges (e.g. vector<char32_t>) for UTF-16 decode
add_library(beman.transcode.tests.utf16_decode_reject_char32_range_fail OBJECT)
target_sources(
    beman.transcode.tests.utf16_decode_reject_char32_range_fail
    PRIVATE utf16_decode_reject_char32_range_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.utf16_decode_reject_char32_range_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.utf16_decode_reject_char32_range_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME utf16_decode_reject_char32_range_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.utf16_decode_reject_char32_range_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    utf16_decode_reject_char32_range_fail
    PROPERTIES PASS_REGULAR_EXPRESSION "legacy_byte_range"
)
