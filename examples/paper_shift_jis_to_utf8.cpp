// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <cerrno>
#include <iconv.h>

#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace beman::transcoding;

namespace before_example {

// clang-format off
// ca504c43-4e44-4ee7-89f0-672cac1e1050
std::string shift_jis_to_utf8(
    std::string_view input) {
  iconv_t cd = iconv_open("UTF-8",
                          "SHIFT_JIS");
  if (cd == (iconv_t)-1)
    throw std::runtime_error("iconv_open");

  std::string result;
  result.resize(input.size() * 4);

  char* inbuf = const_cast<char*>(
                  input.data());
  size_t inleft = input.size();
  char* outbuf = result.data();
  size_t outleft = result.size();

  while (inleft > 0) {
    size_t r = iconv(cd, &inbuf, &inleft,
                     &outbuf, &outleft);
    if (r == (size_t)-1) {
      if (errno == E2BIG) {
        size_t used = outbuf - result.data();
        result.resize(result.size() * 2);
        outbuf = result.data() + used;
        outleft = result.size() - used;
      } else if (errno == EILSEQ) {
        // Skip invalid byte
        ++inbuf; --inleft;
        // Append replacement char
        *outbuf++ = '\xEF';
        *outbuf++ = '\xBF';
        *outbuf++ = '\xBD';
        outleft -= 3;
      } else {
        iconv_close(cd);
        throw std::runtime_error("iconv");
      }
    }
  }
  result.resize(outbuf - result.data());
  iconv_close(cd);
  return result;
}
// ca504c43-4e44-4ee7-89f0-672cac1e1050 end
// clang-format on

} // namespace before_example

namespace after_example {

// clang-format off
// 307799ab-fe64-4f57-9d10-063361239a41
std::string shift_jis_to_utf8(
    std::string_view input) {
  return input
    | whatwg_decode<codec::shift_jis>
    | whatwg_encode<codec::utf_8>
    | std::ranges::to<std::string>();
}
// 307799ab-fe64-4f57-9d10-063361239a41 end
// clang-format on

} // namespace after_example

int main() {
    constexpr std::string_view input = "Hello";
    return before_example::shift_jis_to_utf8(input) == after_example::shift_jis_to_utf8(input) ? 0 : 1;
}
