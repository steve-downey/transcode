// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>

#include <cerrno>
#include <iconv.h>

#include <stdexcept>
#include <string>
#include <string_view>

std::string before(std::string_view input) {
    iconv_t conversion = iconv_open("UTF-8", "SHIFT_JIS");
    if (conversion == reinterpret_cast<iconv_t>(-1)) {
        throw std::runtime_error("iconv_open");
    }

    std::string result;
    result.resize(input.size() * 4);

    char*  inbuf   = const_cast<char*>(input.data());
    size_t inleft  = input.size();
    char*  outbuf  = result.data();
    size_t outleft = result.size();

    while (inleft > 0) {
        size_t converted = iconv(conversion, &inbuf, &inleft, &outbuf, &outleft);
        if (converted == static_cast<std::size_t>(-1)) {
            if (errno == E2BIG) {
                std::size_t used = static_cast<std::size_t>(outbuf - result.data());
                result.resize(result.size() * 2);
                outbuf  = result.data() + used;
                outleft = result.size() - used;
            }
            else if (errno == EILSEQ) {
                ++inbuf;
                --inleft;
                *outbuf++ = '\xEF';
                *outbuf++ = '\xBF';
                *outbuf++ = '\xBD';
                outleft -= 3;
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

std::string after(std::string_view input) {
    std::string result;
    for (char byte : input | beman::transcoding::transcode<beman::transcoding::codec::shift_jis,
                                                           beman::transcoding::codec::utf_8>) {
        result.push_back(byte);
    }
    return result;
}

int main() {
    constexpr std::string_view input = "Hello";
    auto                       manual = before(input);
    auto                       view   = after(input);
    return manual == view ? 0 : 1;
}