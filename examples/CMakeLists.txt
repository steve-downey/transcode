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
