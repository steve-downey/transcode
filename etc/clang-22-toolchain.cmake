# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/gcc-flags.cmake")

set(CMAKE_C_COMPILER clang-22)
set(CMAKE_CXX_COMPILER clang++-22)

find_program(_gcc_16_cxx_compiler g++-16 REQUIRED)
get_filename_component(_gcc_16_cxx_compiler "${_gcc_16_cxx_compiler}" REALPATH)
get_filename_component(_gcc_16_bin_dir "${_gcc_16_cxx_compiler}" DIRECTORY)
get_filename_component(_gcc_16_root "${_gcc_16_bin_dir}" DIRECTORY)

set(CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN "${_gcc_16_root}")
set(CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN "${_gcc_16_root}")
