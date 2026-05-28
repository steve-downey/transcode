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

add_executable(beman.transcode.tests.big5_decode)
target_sources(beman.transcode.tests.big5_decode PRIVATE big5_decode.test.cpp)
target_include_directories(
    beman.transcode.tests.big5_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.big5_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.big5_encode)
target_sources(beman.transcode.tests.big5_encode PRIVATE big5_encode.test.cpp)
target_include_directories(
    beman.transcode.tests.big5_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.big5_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.shift_jis_decode)
target_sources(
    beman.transcode.tests.shift_jis_decode
    PRIVATE shift_jis_decode.test.cpp
)
target_include_directories(
    beman.transcode.tests.shift_jis_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.shift_jis_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.shift_jis_encode)
target_sources(
    beman.transcode.tests.shift_jis_encode
    PRIVATE shift_jis_encode.test.cpp
)
target_include_directories(
    beman.transcode.tests.shift_jis_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.shift_jis_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.euc_jp_decode)
target_sources(
    beman.transcode.tests.euc_jp_decode
    PRIVATE euc_jp_decode.test.cpp
)
target_include_directories(
    beman.transcode.tests.euc_jp_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.euc_jp_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.euc_jp_encode)
target_sources(
    beman.transcode.tests.euc_jp_encode
    PRIVATE euc_jp_encode.test.cpp
)
target_include_directories(
    beman.transcode.tests.euc_jp_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.euc_jp_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.iso2022jp_decode)
target_sources(
    beman.transcode.tests.iso2022jp_decode
    PRIVATE iso2022jp_decode.test.cpp
)
target_include_directories(
    beman.transcode.tests.iso2022jp_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iso2022jp_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.iso2022jp_encode)
target_sources(
    beman.transcode.tests.iso2022jp_encode
    PRIVATE iso2022jp_encode.test.cpp
)
target_include_directories(
    beman.transcode.tests.iso2022jp_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iso2022jp_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.euc_kr_decode)
target_sources(
    beman.transcode.tests.euc_kr_decode
    PRIVATE euc_kr_decode.test.cpp
)
target_include_directories(
    beman.transcode.tests.euc_kr_decode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.euc_kr_decode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.euc_kr_encode)
target_sources(
    beman.transcode.tests.euc_kr_encode
    PRIVATE euc_kr_encode.test.cpp
)
target_include_directories(
    beman.transcode.tests.euc_kr_encode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.euc_kr_encode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.roundtrip)
target_sources(beman.transcode.tests.roundtrip PRIVATE roundtrip.test.cpp)
target_include_directories(
    beman.transcode.tests.roundtrip
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.roundtrip
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_gb18030)
target_sources(beman.transcode.tests.wpt_gb18030 PRIVATE wpt_gb18030.test.cpp)
target_include_directories(
    beman.transcode.tests.wpt_gb18030
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_gb18030
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_utf8)
target_sources(beman.transcode.tests.wpt_utf8 PRIVATE wpt_utf8.test.cpp)
target_include_directories(
    beman.transcode.tests.wpt_utf8
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_utf8
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_iso2022jp)
target_sources(
    beman.transcode.tests.wpt_iso2022jp
    PRIVATE wpt_iso2022jp.test.cpp
)
target_include_directories(
    beman.transcode.tests.wpt_iso2022jp
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_iso2022jp
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_single_byte)
target_sources(
    beman.transcode.tests.wpt_single_byte
    PRIVATE wpt_single_byte.test.cpp
)
target_include_directories(
    beman.transcode.tests.wpt_single_byte
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_single_byte
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_utf16_surrogates)
target_sources(
    beman.transcode.tests.wpt_utf16_surrogates
    PRIVATE wpt_utf16_surrogates.test.cpp
)
target_include_directories(
    beman.transcode.tests.wpt_utf16_surrogates
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_utf16_surrogates
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_fatal)
target_sources(beman.transcode.tests.wpt_fatal PRIVATE wpt_fatal.test.cpp)
target_include_directories(
    beman.transcode.tests.wpt_fatal
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_fatal
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_bom)
target_sources(beman.transcode.tests.wpt_bom PRIVATE wpt_bom.test.cpp)
target_include_directories(
    beman.transcode.tests.wpt_bom
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_bom
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_fatal_single_byte)
target_sources(
    beman.transcode.tests.wpt_fatal_single_byte
    PRIVATE wpt_fatal_single_byte.test.cpp
)
target_include_directories(
    beman.transcode.tests.wpt_fatal_single_byte
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_fatal_single_byte
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_eof)
target_sources(beman.transcode.tests.wpt_eof PRIVATE wpt_eof.test.cpp)
target_include_directories(
    beman.transcode.tests.wpt_eof
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_eof
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_surrogates_utf8)
target_sources(
    beman.transcode.tests.wpt_surrogates_utf8
    PRIVATE wpt_surrogates_utf8.test.cpp
)
target_include_directories(
    beman.transcode.tests.wpt_surrogates_utf8
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_surrogates_utf8
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.wpt_encoder_surrogates)
target_sources(
    beman.transcode.tests.wpt_encoder_surrogates
    PRIVATE wpt_encoder_surrogates.test.cpp
)
target_include_directories(
    beman.transcode.tests.wpt_encoder_surrogates
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.wpt_encoder_surrogates
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.labels)
target_sources(beman.transcode.tests.labels PRIVATE labels.test.cpp)
target_include_directories(
    beman.transcode.tests.labels
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.labels
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.sniff)
target_sources(beman.transcode.tests.sniff PRIVATE sniff.test.cpp)
target_include_directories(
    beman.transcode.tests.sniff
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.sniff
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.transcode)
target_sources(beman.transcode.tests.transcode PRIVATE transcode.test.cpp)
target_include_directories(
    beman.transcode.tests.transcode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.transcode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.transcode_string)
target_sources(
    beman.transcode.tests.transcode_string
    PRIVATE transcode_string.test.cpp
)
target_include_directories(
    beman.transcode.tests.transcode_string
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.transcode_string
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.transcode_string_label)
target_sources(
    beman.transcode.tests.transcode_string_label
    PRIVATE transcode_string_label.test.cpp
)
target_include_directories(
    beman.transcode.tests.transcode_string_label
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.transcode_string_label
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.transcode_view)
target_sources(
    beman.transcode.tests.transcode_view
    PRIVATE transcode_view.test.cpp
)
target_include_directories(
    beman.transcode.tests.transcode_view
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.transcode_view
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.bulk_transcode)
target_sources(
    beman.transcode.tests.bulk_transcode
    PRIVATE bulk_transcode.test.cpp
)
target_include_directories(
    beman.transcode.tests.bulk_transcode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.bulk_transcode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.iconv_bulk)
target_sources(beman.transcode.tests.iconv_bulk PRIVATE iconv_bulk.test.cpp)
target_include_directories(
    beman.transcode.tests.iconv_bulk
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iconv_bulk
    PRIVATE beman::transcode Catch2::Catch2WithMain Iconv::Iconv
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
catch_discover_tests(beman.transcode.tests.big5_decode)
catch_discover_tests(beman.transcode.tests.big5_encode)
catch_discover_tests(beman.transcode.tests.shift_jis_decode)
catch_discover_tests(beman.transcode.tests.shift_jis_encode)
catch_discover_tests(beman.transcode.tests.euc_jp_decode)
catch_discover_tests(beman.transcode.tests.euc_jp_encode)
catch_discover_tests(beman.transcode.tests.iso2022jp_decode)
catch_discover_tests(beman.transcode.tests.iso2022jp_encode)
catch_discover_tests(beman.transcode.tests.euc_kr_decode)
catch_discover_tests(beman.transcode.tests.euc_kr_encode)
catch_discover_tests(beman.transcode.tests.roundtrip)
catch_discover_tests(beman.transcode.tests.wpt_gb18030)
catch_discover_tests(beman.transcode.tests.wpt_utf8)
catch_discover_tests(beman.transcode.tests.wpt_iso2022jp)
catch_discover_tests(beman.transcode.tests.wpt_single_byte)
catch_discover_tests(beman.transcode.tests.wpt_utf16_surrogates)
catch_discover_tests(beman.transcode.tests.wpt_fatal)
catch_discover_tests(beman.transcode.tests.wpt_bom)
catch_discover_tests(beman.transcode.tests.wpt_fatal_single_byte)
catch_discover_tests(beman.transcode.tests.wpt_eof)
catch_discover_tests(beman.transcode.tests.wpt_surrogates_utf8)
catch_discover_tests(beman.transcode.tests.wpt_encoder_surrogates)
catch_discover_tests(beman.transcode.tests.labels)
catch_discover_tests(beman.transcode.tests.sniff)
catch_discover_tests(beman.transcode.tests.transcode)
catch_discover_tests(beman.transcode.tests.transcode_string)
catch_discover_tests(beman.transcode.tests.transcode_string_label)
catch_discover_tests(beman.transcode.tests.transcode_view)
catch_discover_tests(beman.transcode.tests.bulk_transcode)
catch_discover_tests(beman.transcode.tests.iconv_bulk)

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
        PASS_REGULAR_EXPRESSION
            "no operand of the disjunction is satisfied|constraints not satisfied"
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

# transcode_closure rejects raw arrays (char[N]) via a constrained operator|
# with a static_assert that fires when the input is an array type.
add_library(beman.transcode.tests.transcode_view_reject_array_fail OBJECT)
target_sources(
    beman.transcode.tests.transcode_view_reject_array_fail
    PRIVATE transcode_view_reject_array_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.transcode_view_reject_array_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.transcode_view_reject_array_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME transcode_view_reject_array_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.transcode_view_reject_array_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    transcode_view_reject_array_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION
            "transcode: raw arrays are not valid input to transcode"
)

# sniff_encoding requires legacy_byte_range; char32_t ranges must be rejected.
add_library(beman.transcode.tests.sniff_reject_char32_range_fail OBJECT)
target_sources(
    beman.transcode.tests.sniff_reject_char32_range_fail
    PRIVATE sniff_reject_char32_range_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.sniff_reject_char32_range_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.sniff_reject_char32_range_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME sniff_reject_char32_range_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.sniff_reject_char32_range_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    sniff_reject_char32_range_fail
    PROPERTIES PASS_REGULAR_EXPRESSION "legacy_byte_range"
)

# Module integration test (only when modules are enabled)
if(BEMAN_TRANSCODE_USE_MODULES)
    add_executable(beman.transcode.tests.module_integration)
    target_sources(
        beman.transcode.tests.module_integration
        PRIVATE module_integration.test.cpp
    )
    target_include_directories(
        beman.transcode.tests.module_integration
        PRIVATE ${TEST_INCLUDE_DIR}
    )
    target_link_libraries(
        beman.transcode.tests.module_integration
        PRIVATE beman::transcode Catch2::Catch2WithMain
    )
    add_test(
        NAME beman.transcode.tests.module_integration
        COMMAND beman.transcode.tests.module_integration
    )
endif()

# ---------------------------------------------------------------------------
# Pluggable codec protocol tests
# ---------------------------------------------------------------------------

add_executable(beman.transcode.tests.codec_concepts)
target_sources(
    beman.transcode.tests.codec_concepts
    PRIVATE codec_concepts.test.cpp
)
target_include_directories(
    beman.transcode.tests.codec_concepts
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.codec_concepts
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
add_test(
    NAME beman.transcode.tests.codec_concepts
    COMMAND beman.transcode.tests.codec_concepts
)

add_executable(beman.transcode.tests.decode_view)
target_sources(beman.transcode.tests.decode_view PRIVATE decode_view.test.cpp)
target_include_directories(
    beman.transcode.tests.decode_view
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.decode_view
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
add_test(
    NAME beman.transcode.tests.decode_view
    COMMAND beman.transcode.tests.decode_view
)

# Negative compile test: decode rejects raw arrays
add_library(beman.transcode.tests.decode_view_reject_arrays_fail OBJECT)
target_sources(
    beman.transcode.tests.decode_view_reject_arrays_fail
    PRIVATE decode_view_reject_arrays_fail.cpp
)
target_link_libraries(
    beman.transcode.tests.decode_view_reject_arrays_fail
    PRIVATE beman::transcode
)
set_target_properties(
    beman.transcode.tests.decode_view_reject_arrays_fail
    PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true
)
add_test(
    NAME decode_view_reject_arrays_fail
    COMMAND
        ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target
        beman.transcode.tests.decode_view_reject_arrays_fail --config
        $<CONFIGURATION>
)
set_tests_properties(
    decode_view_reject_arrays_fail
    PROPERTIES
        PASS_REGULAR_EXPRESSION
            "transcode: raw arrays are not valid input to decode"
)

add_executable(beman.transcode.tests.blp59001)
target_sources(beman.transcode.tests.blp59001 PRIVATE blp59001.test.cpp)
target_include_directories(
    beman.transcode.tests.blp59001
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.blp59001
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
add_test(
    NAME beman.transcode.tests.blp59001
    COMMAND beman.transcode.tests.blp59001
)

add_executable(beman.transcode.tests.ebcdic037)
target_sources(beman.transcode.tests.ebcdic037 PRIVATE ebcdic037.test.cpp)
target_include_directories(
    beman.transcode.tests.ebcdic037
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.ebcdic037
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
add_test(
    NAME beman.transcode.tests.ebcdic037
    COMMAND beman.transcode.tests.ebcdic037
)

# ---------------------------------------------------------------------------
# Benchmark fixture unit tests
# ---------------------------------------------------------------------------

add_executable(beman.transcode.tests.benchmark_fixture)
target_sources(
    beman.transcode.tests.benchmark_fixture
    PRIVATE benchmark_fixture.test.cpp
)
target_include_directories(
    beman.transcode.tests.benchmark_fixture
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.benchmark_fixture
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
)
catch_discover_tests(beman.transcode.tests.benchmark_fixture)
