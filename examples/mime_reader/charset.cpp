// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <examples/mime_reader/charset.hpp>
#include <examples/mime_reader/charset.hpp>

#include <beman/transcode/whatwg/decode_view.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/transcode_string.hpp>

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace mime_reader {

namespace whatwg = beman::transcoding::whatwg;

std::expected<std::u32string, std::string> decode_text(std::span<const char> bytes, std::string_view charset_label) {
    // TODO(stage 6): replace with dynamic_decoder.  The library has no runtime
    // decoder to code points yet, so go through UTF-8, which it does have.
    auto utf8 = whatwg::transcode_string(bytes, charset_label, "utf-8");
    if (!utf8) {
        return std::unexpected("unknown charset '" + std::string{charset_label} + "'");
    }
    std::u32string text;
    for (char32_t cp : *utf8 | whatwg::decode<whatwg::encoding::utf_8>) {
        text.push_back(cp);
    }
    return text;
}

std::string encode_utf8(std::u32string_view text) {
    std::string utf8;
    for (char c : text | whatwg::encode<whatwg::encoding::utf_8>) {
        utf8.push_back(c);
    }
    return utf8;
}

} // namespace mime_reader
