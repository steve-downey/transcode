// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The syntax a MIME reader needs, written close to the ABNF of the RFCs that
// define it.  Every entry point parses a whole input and yields a value or the
// offset and description of the first thing that was wrong.
//
//     RFC 5322  header fields, with folding
//     RFC 2045  Content-Type and Content-Transfer-Encoding
//     RFC 2046  multipart bodies
//     RFC 2047  encoded-words in unstructured header text
//
// Deliberately left out: RFC 2231 parameter continuations and charset
// extensions; comments and other CFWS (only white space and folding);
// obsolete syntax; structured header fields beyond Content-Type and
// Content-Transfer-Encoding, so an address list is read as unstructured text.
//
// Line endings are CRLF, as the RFCs say, or a bare LF, as files have them.

#ifndef EXAMPLES_MIME_READER_GRAMMAR_HPP
#define EXAMPLES_MIME_READER_GRAMMAR_HPP

#include <examples/mime_reader/parser.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mime_reader {

template <class T>
using parsed = std::expected<T, parse::parse_error>;

// A header field.  The value has its folding removed and the white space
// around it trimmed; it is still the bytes of the message.
struct header_field {
    std::string name;
    std::string value;
};

struct media_parameter {
    std::string name; // lower case
    std::string value;
};

// Content-Type.  The type, the subtype and the parameter names are
// case-insensitive, so are held in lower case.  Parameter values are not.
struct media_type {
    std::string                  type    = "text";
    std::string                  subtype = "plain";
    std::vector<media_parameter> parameters;

    std::optional<std::string> parameter(std::string_view name) const;
};

enum class transfer_encoding { seven_bit, eight_bit, binary, base64, quoted_printable };

// The spelling RFC 2045 gives the mechanism, e.g. "quoted-printable".
std::string_view to_string(transfer_encoding encoding);

// A message or a body part: header fields, then the bytes after the blank line.
struct entity_syntax {
    std::vector<header_field> headers;
    std::string_view          body;
};

// A word of unstructured header text: white space, plain text, or an
// encoded-word (RFC 2047) with its charset and its still-encoded text.
struct header_chunk {
    enum class kind { space, text, encoded_word };

    kind        what = kind::text;
    std::string text;           // the bytes for space and text; the encoded-text of an encoded-word
    std::string charset;        // encoded-word only
    char        encoding = 'Q'; // 'B' or 'Q', encoded-word only

    // The encoded-word as it was written, for a reader that cannot decode it.
    std::string spelling() const;
};

parsed<entity_syntax>             parse_entity_syntax(std::string_view bytes);
parsed<media_type>                parse_content_type(std::string_view value);
parsed<transfer_encoding>         parse_transfer_encoding(std::string_view value);
parsed<std::vector<header_chunk>> parse_unstructured(std::string_view value);

// The body parts of a multipart body, each as the bytes between its
// boundaries: its header fields, a blank line, its body.  The preamble and the
// epilogue are skipped.
parsed<std::vector<std::string_view>> parse_multipart_body(std::string_view body, std::string_view boundary);

} // namespace mime_reader

#endif // EXAMPLES_MIME_READER_GRAMMAR_HPP
