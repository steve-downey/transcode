// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <ranges>
#include <string_view>
#include <vector>

using namespace beman::transcoding;

namespace before_example {

// clang-format off
// 80d8fd24-5a96-46d0-8cb3-ab433f9aeaf7
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  std::vector<char32_t> result;
  size_t i = 0;
  while (i < input.size()) {
    unsigned char b = input[i];
    char32_t cp; int extra;
    if (b < 0x80) { cp = b; extra = 0; }
    else if ((b & 0xE0) == 0xC0)
      { cp = b & 0x1F; extra = 1; }
    else if ((b & 0xF0) == 0xE0)
      { cp = b & 0x0F; extra = 2; }
    else if ((b & 0xF8) == 0xF0)
      { cp = b & 0x07; extra = 3; }
    else { result.push_back(U'\xFFFD');
           ++i; continue; }
    if (i + extra >= input.size()) {
      result.push_back(U'\xFFFD'); break;
    }
    for (int j = 0; j < extra; ++j) {
      unsigned char c = input[++i];
      if ((c & 0xC0) != 0x80) {
        cp = U'\xFFFD'; break;
      }
      cp = (cp << 6) | (c & 0x3F);
    }
    // Missing: overlong, surrogate checks
    result.push_back(cp);
    ++i;
  }
  return result;
}
// 80d8fd24-5a96-46d0-8cb3-ab433f9aeaf7 end
// clang-format on

} // namespace before_example

namespace after_example {

// clang-format off
// 352e080f-46ee-46de-8da4-9280c42866e4
std::vector<char32_t> decode_utf8(
    std::string_view input) {
  return input
    | whatwg_decode<codec::utf_8>
    | std::ranges::to<std::vector>();
}
// 352e080f-46ee-46de-8da4-9280c42866e4 end
// clang-format on

} // namespace after_example

int main() {
    constexpr std::string_view input = "Hello";
    return before_example::decode_utf8(input) == after_example::decode_utf8(input) ? 0 : 1;
}
