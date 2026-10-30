// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: an encoder that does not model single_byte_text_encoder
// (it can emit more than one byte per scalar value) gives an encode view that is
// not a random_access_range, so it cannot be passed where one is required.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/encode_view.hpp>

#include <ranges>
#include <span>

struct multibyte_encoder {
    constexpr beman::transcoding::encode_result encode_one(char32_t) const {
        beman::transcoding::encode_result r{};
        r.bytes[0] = 0xC2;
        r.bytes[1] = 0x80;
        r.count    = 2;
        return r;
    }
};

template <std::ranges::random_access_range R>
constexpr void needs_random_access(R&&) {}

void test() {
    std::span<const char32_t> src;
    needs_random_access(src | beman::transcoding::encode(multibyte_encoder{}));
}
