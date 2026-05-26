// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <string_view>
#include <vector>

std::vector<char32_t> before(std::string_view input) {
    std::vector<char32_t> result;
    std::size_t           index = 0;
    while (index < input.size()) {
        unsigned char byte = static_cast<unsigned char>(input[index]);
        char32_t      code_point{};
        int           extra = 0;
        if (byte < 0x80) {
            code_point = byte;
        }
        else if ((byte & 0xE0) == 0xC0) {
            code_point = byte & 0x1F;
            extra      = 1;
        }
        else if ((byte & 0xF0) == 0xE0) {
            code_point = byte & 0x0F;
            extra      = 2;
        }
        else if ((byte & 0xF8) == 0xF0) {
            code_point = byte & 0x07;
            extra      = 3;
        }
        else {
            result.push_back(U'\xFFFD');
            ++index;
            continue;
        }
        if (index + static_cast<std::size_t>(extra) >= input.size()) {
            result.push_back(U'\xFFFD');
            break;
        }
        for (int continuation = 0; continuation != extra; ++continuation) {
            unsigned char next = static_cast<unsigned char>(input[++index]);
            if ((next & 0xC0) != 0x80) {
                code_point = U'\xFFFD';
                break;
            }
            code_point = (code_point << 6) | (next & 0x3F);
        }
        result.push_back(code_point);
        ++index;
    }
    return result;
}

std::vector<char32_t> after(std::string_view input) {
    std::vector<char32_t> result;
    for (char32_t code_point : input | beman::transcoding::whatwg_decode<beman::transcoding::codec::utf_8>) {
        result.push_back(code_point);
    }
    return result;
}

int main() {
    constexpr std::string_view input = "Hello";
    auto                       manual = before(input);
    auto                       view   = after(input);
    return manual == view ? 0 : 1;
}