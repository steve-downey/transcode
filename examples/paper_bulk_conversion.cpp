// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

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
      | whatwg_decode<codec::utf_8>) {
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
  return decode_to<codec::utf_8>(input);
}
// 04be596b-c72b-4cc4-9ebd-5966c7e92502 end
// clang-format on

} // namespace after_example

int main() {
    constexpr std::string_view input = "Hello";
    return before_example::decode_utf8(input) == after_example::decode_utf8(input) ? 0 : 1;
}
