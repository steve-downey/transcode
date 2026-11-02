# cmake/beman-xfail-compile-expect.cmake                            -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include_guard(GLOBAL)

# beman_xfail_compile_read_expect
# ===============================
#
#   beman_xfail_compile_read_expect(<source> <out-var>)
#
# Reads the expectation lines of a negative compile test source, of the form
#
#   // EXPECT: "<regex>"
#
# and stores them in <out-var> joined with "|", or "" when there are none.
# It is shared by beman_add_xfail_compile_test(), which uses it to reject a
# source with no expectation at configure time, and by the test driver,
# which reads the file when the test runs so that editing an expectation
# needs neither a reconfigure nor a rebuild.
#
# The file is scanned with string operations rather than file(STRINGS) and a
# list: a regex is free to contain an unbalanced "[" (as in
# "\[with R = .+foo"), and CMake does not split a list at a ";" that follows
# an unmatched "[", which would merge that line with the next.

function(beman_xfail_compile_read_expect source out_var)
    file(READ "${source}" _rest)
    set(_expect "")
    set(_separator "")
    while(TRUE)
        string(
            REGEX MATCH "(^|\n)// EXPECT: \"([^\"\n]*)\"[ \t]*(\n|$)"
            _match
            "${_rest}"
        )
        if("${_match}" STREQUAL "")
            break()
        endif()
        string(APPEND _expect "${_separator}${CMAKE_MATCH_2}")
        set(_separator "|")
        # Resume just before the newline that ended this match, so it can
        # begin the next one.
        string(FIND "${_rest}" "${_match}" _position)
        string(LENGTH "${_match}" _length)
        math(EXPR _next "${_position} + ${_length} - 1")
        string(SUBSTRING "${_rest}" ${_next} -1 _rest)
    endwhile()
    set(${out_var} "${_expect}" PARENT_SCOPE)
endfunction()
