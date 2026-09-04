# cmake/x64-linux-custom.cmake                                -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# The overlay triplet the Makefile selects when vcpkg is available
# (VCPKG_OVERLAY_TRIPLETS=$(CURDIR)/cmake, VCPKG_TARGET_TRIPLET=x64-linux-custom).
# It exists to chainload the project's own toolchain file, exported by the
# Makefile as PROJECT_VCPKG_TOOLCHAIN, so that vcpkg builds the dependencies
# with the same compiler the project is built with.  Catch2 compiled by a
# different compiler than the tests that link it is an ABI mismatch waiting to
# happen.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

message(NOTICE "USE_VCPKG_TOOLCHAIN: $ENV{PROJECT_VCPKG_TOOLCHAIN}")

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{PROJECT_VCPKG_TOOLCHAIN}")
