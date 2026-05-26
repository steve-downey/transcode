# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(
    ALL_EXAMPLES
    todo
    paper_decode_utf8
    paper_shift_jis_to_utf8
    paper_from_cstring
    paper_error_visibility
)
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
