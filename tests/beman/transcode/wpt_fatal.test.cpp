// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_fatal_vectors.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <vector>

using namespace beman::transcoding;

namespace {

std::vector<std::expected<char32_t, whatwg_error>> decode_utf8_or_error(const std::vector<uint8_t>& input) {
    std::vector<char>                                  bytes(input.begin(), input.end());
    std::vector<std::expected<char32_t, whatwg_error>> result;
    for (auto&& r : bytes | whatwg_decode_or_error<codec::utf_8>)
        result.push_back(r);
    return result;
}

std::vector<std::expected<char32_t, whatwg_error>> decode_utf16le_or_error(const std::vector<uint8_t>& input) {
    std::vector<char>                                  bytes(input.begin(), input.end());
    std::vector<std::expected<char32_t, whatwg_error>> result;
    for (auto&& r : bytes | whatwg_decode_or_error<codec::utf_16le>)
        result.push_back(r);
    return result;
}

bool has_error(const std::vector<std::expected<char32_t, whatwg_error>>& result) {
    for (const auto& r : result)
        if (!r.has_value())
            return true;
    return false;
}

} // namespace

TEST_CASE("WPT fatal UTF-8: all invalid sequences produce errors", "[wpt::fatal::utf_8]") {
    const auto& all = beman::transcoding::tests::wpt::utf8_fatal_wpt_vectors;
    for (std::size_t idx = 0; idx < std::size(all); ++idx) {
        const auto& v = all[idx];
        INFO("Case [" << idx << "]: " << v.description);
        CHECK(has_error(decode_utf8_or_error(v.input)));
    }
}

TEST_CASE("WPT fatal UTF-16LE: all invalid sequences produce errors", "[wpt::fatal::utf_16le]") {
    const auto& all = beman::transcoding::tests::wpt::utf16le_fatal_wpt_vectors;
    for (std::size_t idx = 0; idx < std::size(all); ++idx) {
        const auto& v = all[idx];
        INFO("Case [" << idx << "]: " << v.description);
        CHECK(has_error(decode_utf16le_or_error(v.input)));
    }
}
