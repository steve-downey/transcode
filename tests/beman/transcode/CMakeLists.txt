# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(Catch2 3 REQUIRED)

add_executable(beman.transcode.tests.todo)
target_sources(beman.transcode.tests.todo PRIVATE todo.test.cpp)
target_link_libraries(
    beman.transcode.tests.todo
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
if(BEMAN_EXEMPLAR_USE_MODULES)
    set_target_properties(
        beman.transcode.tests.todo
        PROPERTIES CXX_MODULE_STD ON
    )
endif()

include(Catch)
catch_discover_tests(beman.transcode.tests.todo)
