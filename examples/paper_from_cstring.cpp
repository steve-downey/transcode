// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <clocale>
#include <cstdlib>

#include <stdexcept>
#include <string>

using namespace beman::transcoding;

namespace before_example {

// clang-format off
// 0be53424-b002-4662-90f0-93f262a2dae1
std::wstring from_cstring(const char* s) {
  // Assumes locale is set correctly
  std::setlocale(LC_ALL, "");
  size_t len = std::mbstowcs(nullptr, s, 0);
  if (len == (size_t)-1)
    throw std::runtime_error("mbstowcs");
  std::wstring result(len, L'\0');
  std::mbstowcs(result.data(), s, len + 1);
  return result;
  // Problems:
  // - Global locale state
  // - wchar_t is not portable
  // - No error recovery
  // - Two passes required
}
// 0be53424-b002-4662-90f0-93f262a2dae1 end
// clang-format on

} // namespace before_example

namespace after_example {

// clang-format off
// 742abae0-4d08-4faf-9db8-759404fd46d9
auto from_cstring(const char* s) {
  return views::null_term(s)
    | whatwg_decode<codec::utf_8>;
  // Returns lazy view of char32_t
  // No global state
  // Portable Unicode scalars
  // Single pass, errors yield U+FFFD
}
// 742abae0-4d08-4faf-9db8-759404fd46d9 end
// clang-format on

} // namespace after_example

int main() {
    auto decoded = after_example::from_cstring("Hello");
    return decoded.empty() ? 1 : 0;
}
