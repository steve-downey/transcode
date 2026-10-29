// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: a decoder whose decode_first returns the wrong type
// is a text_decoder but does not model bom_aware_text_decoder.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/codec_result.hpp>

struct wrong_first_decoder {
    template <typename I, typename S>
    constexpr beman::transcoding::decode_result decode_one(I&, S) {
        return {};
    }
    // decode_first must return decode_result.
    template <typename I, typename S>
    constexpr int decode_first(I&, S) {
        return 0;
    }
};

static_assert(beman::transcoding::text_decoder<wrong_first_decoder>);

template <beman::transcoding::bom_aware_text_decoder C>
constexpr void needs_bom_aware_decoder(C) {}

void test() { needs_bom_aware_decoder(wrong_first_decoder{}); }
