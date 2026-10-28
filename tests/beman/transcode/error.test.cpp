// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/error.hpp>
#include <beman/transcode/error.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <catch2/catch_all.hpp>

using namespace beman::transcoding;

TEST_CASE("whatwg::error enum values", "[transcoding::error]") {
    CHECK(whatwg::error::invalid_byte != whatwg::error::truncated_sequence);
    CHECK(whatwg::error::overlong_encoding != whatwg::error::surrogate_code_point);
    CHECK(whatwg::error::out_of_range != whatwg::error::invalid_byte);
}

TEST_CASE("iconv_error enum values", "[transcoding::error]") {
    CHECK(iconv_error::invalid_sequence != iconv_error::incomplete_sequence);
    CHECK(iconv_error::incomplete_sequence != iconv_error::output_full);
    CHECK(iconv_error::output_full != iconv_error::invalid_sequence);
    CHECK(iconv_error::open_failed != iconv_error::system_error);
    CHECK(iconv_error::system_error != iconv_error::invalid_sequence);
}
