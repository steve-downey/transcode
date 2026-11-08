// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <string_view>
#include <vector>

using namespace beman::transcoding;

namespace before_example {

// clang-format off
// b63db4b8-c004-42ab-8c9e-6a4fbba64f52
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  std::vector<char32_t> result;
  for (char32_t cp : input
      | whatwg::decode<whatwg::encoding::utf_8>) {
    result.push_back(cp);
  }
  return result;
}
// b63db4b8-c004-42ab-8c9e-6a4fbba64f52 end
// clang-format on

} // namespace before_example

namespace after_example {

// clang-format off
// 04be596b-c72b-4cc4-9ebd-5966c7e92502
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  return whatwg::decode_to<whatwg::encoding::utf_8>(input);
}
// 04be596b-c72b-4cc4-9ebd-5966c7e92502 end
// clang-format on

} // namespace after_example

namespace caller_buffer_example {

// clang-format off
// e32e3260-7a8e-4cc8-8a4f-9adb6d9797ec
std::size_t decode_into_buffer(
    std::string_view bytes,
    std::array<char32_t, 256>& buf) {
  auto [in, out] = std::ranges::copy(
      bytes
        | whatwg::decode<whatwg::encoding::utf_8>
        | std::views::take(buf.size()),
      buf.begin());
  return static_cast<std::size_t>(out - buf.begin());
}
// e32e3260-7a8e-4cc8-8a4f-9adb6d9797ec end
// clang-format on

} // namespace caller_buffer_example

int main() {
    constexpr std::string_view input = "Hello";
    std::array<char32_t, 256>  buf{};
    const std::size_t          n = caller_buffer_example::decode_into_buffer(input, buf);
    const bool                 buffer_ok =
        n == input.size() &&
        std::ranges::equal(std::views::take(buf, n), input, {}, {}, [](char c) { return static_cast<char32_t>(c); });
    return before_example::decode_utf8(input) == after_example::decode_utf8(input) && buffer_ok ? 0 : 1;
}
