# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES
    todo
    custom_single_byte_decoder
    paper_custom_codec
    paper_iconv_view
    paper_from_cstring
    paper_error_visibility
)
# These need std::ranges::to, directly or through the bulk helpers, which
# libstdc++ lacks before GCC 14.
if(BEMAN_TRANSCODE_HAS_RANGES_TO)
    list(
        APPEND ALL_EXAMPLES
        paper_decode_utf8
        paper_shift_jis_to_utf8
        paper_bulk_conversion
    )
endif()
message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.transcode.examples.${example})
    target_sources(beman.transcode.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(
        beman.transcode.examples.${example}
        PRIVATE beman::transcode
    )
    if(BEMAN_TRANSCODE_USE_MODULES)
        set_target_properties(
            beman.transcode.examples.${example}
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()
endforeach()

if(BEMAN_TRANSCODE_BUILD_TESTS)
    add_test(
        NAME beman.transcode.examples.paper_error_visibility
        COMMAND beman.transcode.examples.paper_error_visibility
    )
    add_test(
        NAME beman.transcode.examples.paper_custom_codec
        COMMAND beman.transcode.examples.paper_custom_codec
    )
endif()

# Needs <text_encoding>, which is C++26.  An example and not a test: its output
# depends on the platform's iconv tables.
if(BEMAN_TRANSCODE_HAS_TEXT_ENCODING)
    add_executable(beman.transcode.examples.text_encoding_two_answers)
    target_sources(
        beman.transcode.examples.text_encoding_two_answers
        PRIVATE text_encoding_two_answers.cpp
    )
    target_link_libraries(
        beman.transcode.examples.text_encoding_two_answers
        PRIVATE beman::transcode
    )
    set_target_properties(
        beman.transcode.examples.text_encoding_two_answers
        PROPERTIES CXX_STANDARD 26 CXX_STANDARD_REQUIRED ON
    )
endif()

# The MIME reader decodes through the runtime bulk helpers, which need
# std::ranges::to.
if(BEMAN_TRANSCODE_HAS_RANGES_TO)
    # The MIME reader has several sources and its own include root (the repository,
    # so that its headers are <examples/mime_reader/...>), so it is not one of
    # ALL_EXAMPLES.
    add_executable(beman.transcode.examples.mime_reader)
    target_sources(
        beman.transcode.examples.mime_reader
        PRIVATE
            mime_reader/charset.cpp
            mime_reader/grammar.cpp
            mime_reader/main.cpp
            mime_reader/mime.cpp
            mime_reader/transfer.cpp
    )
    target_include_directories(
        beman.transcode.examples.mime_reader
        PRIVATE "${PROJECT_SOURCE_DIR}"
    )
    target_link_libraries(
        beman.transcode.examples.mime_reader
        PRIVATE beman::transcode
    )
    if(BEMAN_TRANSCODE_USE_MODULES)
        set_target_properties(
            beman.transcode.examples.mime_reader
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()

    # Each message is read, and what the reader prints is compared with the file
    # that make_expected.py wrote from Python's email package.
    if(BEMAN_TRANSCODE_BUILD_TESTS)
        set(MIME_READER_MESSAGES
            iso-2022-jp-alternative
            shift-jis-base64
            windows-1252-quoted-printable
            utf-8-base64
            koi8-r-subject
            nested-multipart
            unknown-charset
        )
        foreach(message ${MIME_READER_MESSAGES})
            add_test(
                NAME beman.transcode.examples.mime_reader.${message}
                COMMAND
                    "${CMAKE_COMMAND}"
                    "-DREADER=$<TARGET_FILE:beman.transcode.examples.mime_reader>"
                    "-DMESSAGE=${CMAKE_CURRENT_SOURCE_DIR}/mime_reader/messages/${message}.eml"
                    "-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/mime_reader/expected/${message}.txt"
                    "-DACTUAL=${CMAKE_CURRENT_BINARY_DIR}/mime_reader_${message}.txt"
                    -P
                    "${CMAKE_CURRENT_SOURCE_DIR}/mime_reader/check_output.cmake"
            )
        endforeach()
    endif()
endif()
