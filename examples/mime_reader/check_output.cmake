# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Runs the MIME reader on a message and compares its output with the expected
# file, byte for byte.
#
#   cmake -DREADER=<exe> -DMESSAGE=<.eml> -DEXPECTED=<.txt> -DACTUAL=<.txt>
#         -P check_output.cmake
foreach(variable READER MESSAGE EXPECTED ACTUAL)
    if(NOT DEFINED ${variable})
        message(FATAL_ERROR "check_output.cmake: ${variable} is not set")
    endif()
endforeach()

execute_process(
    COMMAND "${READER}" "${MESSAGE}"
    OUTPUT_FILE "${ACTUAL}"
    ERROR_VARIABLE reader_errors
    RESULT_VARIABLE reader_status
)
if(NOT reader_status EQUAL 0)
    message(
        FATAL_ERROR
        "${READER} ${MESSAGE} exited with ${reader_status}: ${reader_errors}"
    )
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${EXPECTED}" "${ACTUAL}"
    RESULT_VARIABLE differ
)
if(differ)
    message(
        FATAL_ERROR
        "output differs from the expected file; compare\n  ${EXPECTED}\n  ${ACTUAL}"
    )
endif()
