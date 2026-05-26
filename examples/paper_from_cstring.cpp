// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <clocale>
#include <cstdlib>

#include <stdexcept>
#include <string>
#include <vector>

std::wstring before(const char* text) {
    std::setlocale(LC_ALL, "");
    std::size_t length = std::mbstowcs(nullptr, text, 0);
    if (length == static_cast<std::size_t>(-1)) {
        throw std::runtime_error("mbstowcs");
    }
    std::wstring result(length + 1, L'\0');
    std::mbstowcs(result.data(), text, length + 1);
    result.resize(length);
    return result;
}

auto after(const char* text) {
    return beman::transcoding::views::null_term(text)
         | beman::transcoding::whatwg_decode<beman::transcoding::codec::utf_8>;
}

int main() {
    constexpr const char* input = "Hello";
    auto                  legacy = before(input);
    std::vector<char32_t> decoded;
    for (char32_t code_point : after(input)) {
        decoded.push_back(code_point);
    }
    return !legacy.empty() && !decoded.empty() ? 0 : 1;
}
