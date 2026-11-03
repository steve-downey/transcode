# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Warnings as errors gate this project's code, not its dependencies.  When the
# dependency provider fetches Catch2, its targets would otherwise inherit
# CMAKE_COMPILE_WARNING_AS_ERROR from the cache and fail on warnings raised in
# Catch2 or the standard library headers it uses.  Shadow the cache entry for
# the fetch, then unset the normal variable to expose it again.
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
find_package(Catch2 3 REQUIRED)
unset(CMAKE_COMPILE_WARNING_AS_ERROR)

include(${PROJECT_SOURCE_DIR}/cmake/beman-xfail-compile.cmake)

# Common include path for test_utilities.hpp (tests/beman/transcode/test_utilities.hpp)
set(TEST_INCLUDE_DIR "${PROJECT_SOURCE_DIR}")

# The component-header tests below exercise header mode: each source includes
# the header under test directly so that its self-containment is checked.  In
# module mode those headers are compiled in the module purview after
# `import std`; their standard-library includes are deliberately suppressed,
# and consumers reach them by importing `beman.transcode` (directly or through
# the <beman/transcode/transcode.hpp> shim).  Run the dedicated import test in
# that configuration instead of compiling the header-mode suite with a module
# configuration header.
if(BEMAN_TRANSCODE_USE_MODULES)
    message(
        STATUS
        "Module mode registers only beman.transcode.tests.module_integration; "
        "the full header-mode suite is registered when BEMAN_TRANSCODE_USE_MODULES=OFF."
    )
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
    return()
endif()

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

add_executable(beman.transcode.tests.reverse_index)
target_sources(
    beman.transcode.tests.reverse_index
    PRIVATE reverse_index.test.cpp
)
target_include_directories(
    beman.transcode.tests.reverse_index
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.reverse_index
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

add_executable(beman.transcode.tests.whatwg_decoder)
target_sources(
    beman.transcode.tests.whatwg_decoder
    PRIVATE whatwg_decoder.test.cpp
)
target_include_directories(
    beman.transcode.tests.whatwg_decoder
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.whatwg_decoder
    PRIVATE beman::transcode Catch2::Catch2WithMain
)

add_executable(beman.transcode.tests.whatwg_encoder)
target_sources(
    beman.transcode.tests.whatwg_encoder
    PRIVATE whatwg_encoder.test.cpp
)
target_include_directories(
    beman.transcode.tests.whatwg_encoder
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.whatwg_encoder
    PRIVATE
        beman::transcode
        beman.transcode.benchmark_fixture
        Catch2::Catch2WithMain
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

if(BEMAN_TRANSCODE_HAS_RANGES_TO)
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
endif()

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

add_executable(beman.transcode.tests.iconv_null_term)
target_sources(
    beman.transcode.tests.iconv_null_term
    PRIVATE iconv_null_term.test.cpp
)
target_include_directories(
    beman.transcode.tests.iconv_null_term
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.iconv_null_term
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
catch_discover_tests(beman.transcode.tests.gbk_decode)
catch_discover_tests(beman.transcode.tests.gbk_encode)
catch_discover_tests(beman.transcode.tests.gb18030_decode)
catch_discover_tests(beman.transcode.tests.gb18030_encode)
catch_discover_tests(beman.transcode.tests.big5_decode)
catch_discover_tests(beman.transcode.tests.big5_encode)
catch_discover_tests(beman.transcode.tests.shift_jis_decode)
catch_discover_tests(beman.transcode.tests.shift_jis_encode)
catch_discover_tests(beman.transcode.tests.reverse_index)
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
catch_discover_tests(beman.transcode.tests.whatwg_decoder)
catch_discover_tests(beman.transcode.tests.whatwg_encoder)
catch_discover_tests(beman.transcode.tests.transcode_view)
if(BEMAN_TRANSCODE_HAS_RANGES_TO)
    catch_discover_tests(beman.transcode.tests.bulk_transcode)
endif()
catch_discover_tests(beman.transcode.tests.iconv_bulk)
catch_discover_tests(beman.transcode.tests.iconv_null_term)

# iconv_transcode_or_error iterator is move-only (iconv_t is an uncopyable OS handle).
beman_add_xfail_compile_test(
    iconv_transcode_or_error_iterator_copy_fail
    SOURCE iconv_transcode_or_error_iterator_copy_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# iconv_transcode iterator is move-only (iconv_t is an uncopyable OS handle).
beman_add_xfail_compile_test(
    iconv_transcode_iterator_copy_fail
    SOURCE iconv_transcode_iterator_copy_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# ---------------------------------------------------------------------------
# Negative compile tests: verify that constraints cause compile errors for the
# correct reason, not just any compile error.
# Each one is registered with beman_add_xfail_compile_test() from
# cmake/beman-xfail-compile.cmake: the source becomes an OBJECT library that is
# excluded from the default build, and the test builds just that target and
# passes only when the build fails with output matching the source's
# `// EXPECT: "<regex>"` lines.  Where GCC and Clang word the diagnostic
# differently the source carries one EXPECT line per compiler.  All of them
# carry the CTest label "xfail-compile".
# ---------------------------------------------------------------------------

# flushable_text_encoder rejects an encoder whose flush() has the wrong type
beman_add_xfail_compile_test(
    flushable_encoder_reject_fail
    SOURCE flushable_encoder_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::decoder::decode_byte is not callable for an encoding that is not single-byte
beman_add_xfail_compile_test(
    whatwg_decoder_decode_byte_reject_fail
    SOURCE whatwg_decoder_decode_byte_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# a decoder whose decode_first returns the wrong type does not model bom_aware_text_decoder
beman_add_xfail_compile_test(
    bom_aware_decoder_reject_fail
    SOURCE bom_aware_decoder_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::decoder::decode_first is not callable for an encoding that does not strip a BOM
beman_add_xfail_compile_test(
    whatwg_decoder_decode_first_reject_fail
    SOURCE whatwg_decoder_decode_first_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::decoder::flush is not callable for an encoding that carries no decode state
beman_add_xfail_compile_test(
    whatwg_decoder_flush_reject_fail
    SOURCE whatwg_decoder_flush_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::encoder::encode_byte is not callable for an encoding that is not single-byte
beman_add_xfail_compile_test(
    whatwg_encoder_encode_byte_reject_fail
    SOURCE whatwg_encoder_encode_byte_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# an encode view over an encoder that is not single-byte is not random access
beman_add_xfail_compile_test(
    random_access_encode_view_reject_fail
    SOURCE random_access_encode_view_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::encoder<C> is not defined for an encoding WHATWG gives no encoder
beman_add_xfail_compile_test(
    whatwg_encoder_reject_fail
    SOURCE whatwg_encoder_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::encoder<C>::flush is not callable for an encoding that owes nothing
beman_add_xfail_compile_test(
    whatwg_encoder_flush_reject_fail
    SOURCE whatwg_encoder_flush_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# legacy_byte_range rejects raw arrays (char[N]) via !is_array_v
beman_add_xfail_compile_test(
    concepts_reject_arrays_fail
    SOURCE concepts_reject_arrays_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# legacy_byte_range rejects unicode char types (char8_t etc.) via legacy_byte_type disjunction
beman_add_xfail_compile_test(
    concepts_reject_unicode_chars_fail
    SOURCE concepts_reject_unicode_chars_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# unicode_scalar_range rejects non-char32_t ranges (e.g. vector<char>)
beman_add_xfail_compile_test(
    whatwg_encode_reject_char_range_fail
    SOURCE whatwg_encode_reject_char_range_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# WHATWG has no UTF-16BE encoder.
beman_add_xfail_compile_test(
    whatwg_encode_utf16be_reject_fail
    SOURCE whatwg_encode_utf16be_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# WHATWG has no UTF-16LE encoder.
beman_add_xfail_compile_test(
    whatwg_encode_utf16le_or_error_reject_fail
    SOURCE whatwg_encode_utf16le_or_error_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# views::null_term rejects non-pointer ranges via is_pointer_v constraint
beman_add_xfail_compile_test(
    null_term_reject_range_fail
    SOURCE null_term_reject_range_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# legacy_byte_range rejects char32_t ranges (e.g. vector<char32_t>) for UTF-16 decode
beman_add_xfail_compile_test(
    utf16_decode_reject_char32_range_fail
    SOURCE utf16_decode_reject_char32_range_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# The transcode adaptor rejects raw arrays (char[N]) via a constrained operator|
# with a static_assert that fires when the input is an array type.
beman_add_xfail_compile_test(
    transcode_view_reject_array_fail
    SOURCE transcode_view_reject_array_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# transcode(from, to) requires a text_encoder second; a decoder must be rejected.
beman_add_xfail_compile_test(
    transcode_decoder_as_encoder_fail
    SOURCE transcode_decoder_as_encoder_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::transcode<From, To> needs an encoder for To; WHATWG has no UTF-16BE encoder.
beman_add_xfail_compile_test(
    whatwg_transcode_utf16be_reject_fail
    SOURCE whatwg_transcode_utf16be_reject_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::bom_sniff requires legacy_byte_range; char32_t ranges must be rejected.
beman_add_xfail_compile_test(
    sniff_reject_char32_range_fail
    SOURCE sniff_reject_char32_range_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# whatwg::bom_sniff requires forward_range; single-pass ranges must be rejected,
# because examining the mark advances the iterator over it.
beman_add_xfail_compile_test(
    sniff_reject_input_range_fail
    SOURCE sniff_reject_input_range_fail.cpp
    LINK_LIBRARIES beman::transcode
)

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
beman_add_xfail_compile_test(
    decode_view_reject_arrays_fail
    SOURCE decode_view_reject_arrays_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# Negative compile test: a decode view over an owning base is not borrowed
beman_add_xfail_compile_test(
    decode_view_dangling_fail
    SOURCE decode_view_dangling_fail.cpp
    LINK_LIBRARIES beman::transcode
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

add_executable(beman.transcode.tests.encode_view)
target_sources(beman.transcode.tests.encode_view PRIVATE encode_view.test.cpp)
target_include_directories(
    beman.transcode.tests.encode_view
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.encode_view
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
add_test(
    NAME beman.transcode.tests.encode_view
    COMMAND beman.transcode.tests.encode_view
)

if(BEMAN_TRANSCODE_HAS_RANGES_TO)
    add_executable(beman.transcode.tests.pluggable_bulk)
    target_sources(
        beman.transcode.tests.pluggable_bulk
        PRIVATE pluggable_bulk.test.cpp
    )
    target_include_directories(
        beman.transcode.tests.pluggable_bulk
        PRIVATE ${TEST_INCLUDE_DIR}
    )
    target_link_libraries(
        beman.transcode.tests.pluggable_bulk
        PRIVATE beman::transcode Catch2::Catch2WithMain
    )
    add_test(
        NAME beman.transcode.tests.pluggable_bulk
        COMMAND beman.transcode.tests.pluggable_bulk
    )
endif()

add_executable(beman.transcode.tests.pluggable_transcode)
target_sources(
    beman.transcode.tests.pluggable_transcode
    PRIVATE pluggable_transcode.test.cpp
)
target_include_directories(
    beman.transcode.tests.pluggable_transcode
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.pluggable_transcode
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
add_test(
    NAME beman.transcode.tests.pluggable_transcode
    COMMAND beman.transcode.tests.pluggable_transcode
)

# Negative compile test: encode rejects types that don't satisfy text_encoder
beman_add_xfail_compile_test(
    encode_view_reject
    SOURCE encode_view_reject.cpp
    LINK_LIBRARIES beman::transcode
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

# reverse_index stores pointer + 1 in 16 bits, so tables of 0xFFFF or more
# entries are rejected by a static_assert.
beman_add_xfail_compile_test(
    reverse_index_reject_wide_table_fail
    SOURCE reverse_index_reject_wide_table_fail.cpp
    LINK_LIBRARIES beman::transcode
)

# <text_encoding> is declared only in C++26 mode (libstdc++), so this target is
# built as C++26 and exists only where the top-level probe says it can.
if(BEMAN_TRANSCODE_HAS_TEXT_ENCODING)
    add_executable(beman.transcode.tests.text_encoding)
    target_sources(
        beman.transcode.tests.text_encoding
        PRIVATE text_encoding.test.cpp
    )
    target_include_directories(
        beman.transcode.tests.text_encoding
        PRIVATE ${TEST_INCLUDE_DIR}
    )
    target_link_libraries(
        beman.transcode.tests.text_encoding
        PRIVATE beman::transcode Catch2::Catch2WithMain
    )
    set_target_properties(
        beman.transcode.tests.text_encoding
        PROPERTIES CXX_STANDARD 26 CXX_STANDARD_REQUIRED ON
    )
    catch_discover_tests(beman.transcode.tests.text_encoding)
endif()
