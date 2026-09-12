// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_encoder_surrogates_vectors.hpp>

#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::wpt::wpt_encoder_surrogates_vectors;

// WPT names these USVString-handling cases: they exercise Web IDL's conversion
// to scalar values above the WHATWG encoder, not a surrogate branch in that
// encoder.  The same vectors exercise this library's UTF-32 validation at the
// same layer.  The Python parser combines a valid surrogate pair into one
// char32_t and leaves lone surrogates for C++, so the astral case does not test
// surrogate handling here; a char32_t range also cannot split a pair across
// chunks.
TEST_CASE("WPT USVString vectors: UTF-32 validation matches scalar conversion", "[wpt::encoder_surrogates]") {
    for (const auto& v : wpt_encoder_surrogates_vectors) {
        std::vector<char> bytes;
        for (char c : v.input | whatwg_encode<codec::utf_8>)
            bytes.push_back(c);
        std::vector<char32_t> got;
        for (char32_t cp : bytes | whatwg_decode<codec::utf_8>)
            got.push_back(cp);
        INFO("name=" << v.name);
        CHECK(got == v.expected);
    }
}
