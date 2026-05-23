// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/config.hpp>
#include <catch2/catch_all.hpp>
#include <beman/transcode/todo.hpp>

TEST_CASE("todo", "[transcode::todo]") {
    const bool todo = true;
    CHECK(todo);
}
