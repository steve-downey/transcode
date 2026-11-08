// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// A message as a tree of entities, and the decoding that turns the bytes of one
// into something to read: the transfer encoding of a body, the encoded-words of
// a header.  The grammar is in grammar.hpp and the charset step in charset.hpp.

#ifndef EXAMPLES_MIME_READER_MIME_HPP
#define EXAMPLES_MIME_READER_MIME_HPP

#include <examples/mime_reader/grammar.hpp>

#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace mime_reader {

// A message or a body part.  `body` is a view of the message's bytes and is
// still transfer-encoded.  A multipart entity has its parts, parsed in turn,
// and a leaf has none.  A part with no Content-Type is text/plain, and one with
// no Content-Transfer-Encoding is 7bit (RFC 2045 sections 5.2 and 6.1).
struct entity {
    std::vector<header_field> headers;
    media_type                type;
    transfer_encoding         encoding = transfer_encoding::seven_bit;
    std::string_view          body;
    std::vector<entity>       parts;

    // The first header field named `name`, in any case.
    const header_field* find(std::string_view name) const;

    bool is_multipart() const;
    bool is_text() const;

    // "type/subtype"
    std::string content_type() const;

    // The charset parameter of a text part, or "us-ascii" when it has none
    // (RFC 2045 section 5.2), in lower case; empty for a part that is not text.
    std::string charset() const;
};

// The message in `bytes`, or what was wrong with it.  The result refers to
// `bytes`, which must outlive it.
std::expected<entity, std::string> parse_entity(std::string_view bytes);

// The bytes of a leaf's body with its transfer encoding undone.
std::string decode_body(const entity& leaf);

// An unstructured header's text, with its encoded-words decoded.  A charset
// that is not known leaves its encoded-word as written, and says so in
// `problems`.
struct header_text {
    std::u32string           text;
    std::vector<std::string> problems;
};

header_text decode_header_text(std::string_view value);

} // namespace mime_reader

#endif // EXAMPLES_MIME_READER_MIME_HPP
