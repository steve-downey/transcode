// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Negative compile test: a decode_view over an owning base is not a
// borrowed_range, so ranges::find returns ranges::dangling and the result
// cannot be dereferenced.

#include <beman/transcode/decode_view.hpp>

#include <algorithm>
#include <iterator>
#include <ranges>
#include <string>

using namespace beman::transcoding;

struct identity_codec {
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, [[maybe_unused]] S end) {
        auto byte = static_cast<unsigned char>(*current);
        ++current;
        return {static_cast<char32_t>(byte), {}, false};
    }
};

// This must fail to compile: the iterator is std::ranges::dangling.
char32_t first_a() { return *std::ranges::find(std::string{"ab"} | decode(identity_codec{}), U'a'); }
