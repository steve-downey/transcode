// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <string_view>
#include <vector>

using namespace beman::transcoding;

bool     error_logged   = false;
char32_t last_processed = U'\0';

void log_warning(const char*) {}
void log_warning(whatwg_error) { error_logged = true; }
void process(char32_t value) { last_processed = value; }

void before_example() {
    // clang-format off
// 2fb71234-33f5-4fff-bf7b-9afcec8afcbd
// No standard way to detect errors
// during transcoding. Either:
// 1. Errors are silently replaced
// 2. Exceptions thrown mid-stream
// 3. Custom state machine required

bool has_errors = false;
std::vector<char32_t> result;
// ... complex manual decoding with
// error tracking interspersed ...
if (has_errors) {
  log_warning("Invalid UTF-8 detected");
}
// 2fb71234-33f5-4fff-bf7b-9afcec8afcbd end
    // clang-format on
}

void after_example(std::string_view input) {
    // clang-format off
// d9624e97-0a08-46c1-99c3-04a729a7324b
for (auto r : input
    | whatwg_decode_or_error<codec::utf_8>) {
  if (r.has_value()) {
    process(*r);
  } else {
    log_warning(r.error());
    process(U'\xFFFD');
  }
}
// d9624e97-0a08-46c1-99c3-04a729a7324b end
    // clang-format on
}

int main() {
    before_example();
    after_example(std::string_view{"\xFF", 1});
    return error_logged && last_processed == U'\xFFFD' ? 0 : 1;
}
