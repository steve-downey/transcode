// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The one place the reader turns bytes into text.  Everything before it is
// bytes: the message, its headers, a part's body after its transfer encoding
// is undone.  Everything after it is `char32_t`.

#ifndef EXAMPLES_MIME_READER_CHARSET_HPP
#define EXAMPLES_MIME_READER_CHARSET_HPP

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace mime_reader {

// The text of `bytes`, which are encoded as the charset named `charset_label`
// says, as code points.  The label is a MIME `charset` parameter or an RFC 2047
// encoded-word charset, and is looked up the way a browser does, so "us-ascii"
// and "iso-8859-1" are windows-1252.  Malformed bytes become U+FFFD.
//
// A charset that names no encoding is an error, with a message that says so.
// It is not guessed at.
std::expected<std::u32string, std::string> decode_text(std::span<const char> bytes, std::string_view charset_label);

// `text` as UTF-8, for printing.
std::string encode_utf8(std::u32string_view text);

} // namespace mime_reader

#endif // EXAMPLES_MIME_READER_CHARSET_HPP
