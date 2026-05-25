// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_utf16_surrogates_vectors.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <vector>

using namespace beman::transcoding;

namespace {

std::vector<char32_t> decode_utf16le(const std::vector<uint8_t>& input) {
    std::vector<char>     bytes(input.begin(), input.end());
    std::vector<char32_t> result;
    for (char32_t cp : bytes | whatwg_decode<codec::utf_16le>)
        result.push_back(cp);
    return result;
}

std::vector<std::expected<char32_t, whatwg_error>> decode_utf16le_or_error(const std::vector<uint8_t>& input) {
    std::vector<char>                                  bytes(input.begin(), input.end());
    std::vector<std::expected<char32_t, whatwg_error>> result;
    for (auto&& r : bytes | whatwg_decode_or_error<codec::utf_16le>)
        result.push_back(r);
    return result;
}

} // namespace

TEST_CASE("WPT UTF-16LE surrogate decode: surrogates produce U+FFFD", "[wpt::utf_16le]") {
    const auto& all = beman::transcoding::tests::wpt::utf16le_surrogate_wpt_decode_vectors;
    for (std::size_t idx = 0; idx < std::size(all); ++idx) {
        const auto& v = all[idx];
        INFO("Case [" << idx << "]: " << v.description);
        CHECK(decode_utf16le(v.input) == v.expected);
    }
}

TEST_CASE("WPT UTF-16LE surrogate or_error: surrogates produce errors", "[wpt::utf_16le_or_error]") {
    const auto& all = beman::transcoding::tests::wpt::utf16le_surrogate_wpt_decode_vectors;
    for (std::size_t idx = 0; idx < std::size(all); ++idx) {
        const auto& v = all[idx];
        INFO("Case [" << idx << "]: " << v.description);
        auto result    = decode_utf16le_or_error(v.input);
        bool has_error = false;
        for (const auto& r : result)
            if (!r.has_value())
                has_error = true;
        CHECK(has_error);
    }
}
