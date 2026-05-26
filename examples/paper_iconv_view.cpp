// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <iconv.h>

#include <array>
#include <stdexcept>
#include <string_view>
#include <vector>

std::vector<char> before(std::string_view input) {
    iconv_t conversion = iconv_open("UTF-32LE", "UTF-8");
    if (conversion == reinterpret_cast<iconv_t>(-1)) {
        throw std::runtime_error("iconv_open");
    }

    std::vector<char> result(input.size() * 4);
    char*             inbuf   = const_cast<char*>(input.data());
    std::size_t       inleft  = input.size();
    char*             outbuf  = result.data();
    std::size_t       outleft = result.size();

    while (inleft > 0) {
        std::size_t converted = iconv(conversion, &inbuf, &inleft, &outbuf, &outleft);
        if (converted == static_cast<std::size_t>(-1)) {
            if (errno == E2BIG) {
                std::size_t used = static_cast<std::size_t>(outbuf - result.data());
                result.resize(result.size() * 2);
                outbuf  = result.data() + used;
                outleft = result.size() - used;
            }
            else {
                iconv_close(conversion);
                throw std::runtime_error("iconv");
            }
        }
    }

    result.resize(static_cast<std::size_t>(outbuf - result.data()));
    iconv_close(conversion);
    return result;
}

std::vector<char> after(std::string_view input) {
    std::array<char, 256> buffer{};
    std::vector<char>     result;

    for (char byte : input | beman::transcoding::iconv_transcode("UTF-8", "UTF-32LE", std::span(buffer))) {
        result.push_back(byte);
    }

    return result;
}

int main() {
    constexpr std::string_view input = "Hello, world!";
    auto                       manual = before(input);
    auto                       view   = after(input);
    return manual == view ? 0 : 1;
}
