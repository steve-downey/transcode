// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: an encoder whose flush() returns the wrong type must
// not satisfy flushable_text_encoder.
// This file must NOT compile -- it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/codec_result.hpp>

#include <optional>

struct wrong_flush_encoder {
    constexpr beman::transcoding::encode_result encode_one(char32_t) { return {}; }
    // flush() must return std::optional<encode_result>.
    constexpr std::optional<beman::transcoding::decode_result> flush() { return std::nullopt; }
};

template <beman::transcoding::flushable_text_encoder C>
constexpr void needs_flushable_encoder(C) {}

void test() { needs_flushable_encoder(wrong_flush_encoder{}); }
