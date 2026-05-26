// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/detail/table_codec.hpp>

#include <array>

using namespace beman::transcoding;

inline constexpr std::array<char32_t, 128> latin1_upper = [] {
    std::array<char32_t, 128> t{};
    for (int i = 0; i < 128; ++i)
        t[static_cast<std::size_t>(i)] = static_cast<char32_t>(0x80 + i);
    return t;
}();

using latin1_codec = table_codec<latin1_upper>;

void test() {
    char bytes[] = "hello";
    auto view    = bytes | decode(latin1_codec{});
    (void)view;
}
