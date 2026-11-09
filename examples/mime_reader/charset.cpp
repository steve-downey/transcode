// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <examples/mime_reader/charset.hpp>
#include <examples/mime_reader/charset.hpp>

#include <beman/transcode/whatwg/bulk.hpp>
#include <beman/transcode/whatwg/encode_view.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace mime_reader {

namespace whatwg = beman::transcoding::whatwg;

std::expected<std::u32string, std::string> decode_text(std::span<const char> bytes, std::string_view charset_label) {
    // The charset is data, so the encoding is chosen at run time.  The label
    // overload looks it up and dispatches once; nullopt means no encoding has
    // that label.
    auto text = whatwg::decode_to(bytes, charset_label);
    if (!text) {
        return std::unexpected("unknown charset '" + std::string{charset_label} + "'");
    }
    return std::u32string(text->begin(), text->end());
}

std::string encode_utf8(std::u32string_view text) {
    std::string utf8;
    for (char c : text | whatwg::encode<whatwg::encoding::utf_8>) {
        utf8.push_back(c);
    }
    return utf8;
}

} // namespace mime_reader
