// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/error.hpp>
#include <catch2/catch_all.hpp>

using namespace beman::transcoding;

TEST_CASE("whatwg_error enum values", "[transcoding::error]") {
    CHECK(whatwg_error::invalid_byte != whatwg_error::truncated_sequence);
    CHECK(whatwg_error::overlong_encoding != whatwg_error::surrogate_code_point);
    CHECK(whatwg_error::out_of_range != whatwg_error::invalid_byte);
}

TEST_CASE("iconv_error enum values", "[transcoding::error]") {
    CHECK(iconv_error::invalid_sequence != iconv_error::incomplete_sequence);
    CHECK(iconv_error::incomplete_sequence != iconv_error::output_full);
    CHECK(iconv_error::output_full != iconv_error::invalid_sequence);
}
