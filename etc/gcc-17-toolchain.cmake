include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/gcc-flags.cmake")

set(CMAKE_C_COMPILER gcc-17)
set(CMAKE_CXX_COMPILER g++-17)
set(GCOV_EXECUTABLE "gcov-17" CACHE STRING "GCOV executable" FORCE)

set(CMAKE_CXX_FLAGS_ASAN
    "${CMAKE_CXX_FLAGS_ASAN} -Wno-maybe-uninitialized"
    CACHE STRING
    "C++ ASAN Flags"
    FORCE
)
