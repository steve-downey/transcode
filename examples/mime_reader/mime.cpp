// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <examples/mime_reader/mime.hpp>
#include <examples/mime_reader/mime.hpp>

#include <examples/mime_reader/charset.hpp>
#include <examples/mime_reader/grammar.hpp>
#include <examples/mime_reader/parser.hpp>
#include <examples/mime_reader/transfer.hpp>

#include <beman/transcode/decode_view.hpp>
#include <beman/transcode/whatwg/dynamic_codec.hpp>
#include <beman/transcode/whatwg/label.hpp>

#include <cstddef>
#include <expected>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mime_reader {

namespace {

constexpr int max_depth = 32;

bool equal_ignoring_case(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (parse::ascii_lower(a[i]) != parse::ascii_lower(b[i])) {
            return false;
        }
    }
    return true;
}

std::string describe(std::string_view what, const parse::parse_error& error) {
    return std::string{what} + ": " + error.message + " at byte " + std::to_string(error.offset);
}

std::expected<entity, std::string> parse_entity_at_depth(std::string_view bytes, int depth) {
    if (depth > max_depth) {
        return std::unexpected("multipart parts nested too deeply");
    }
    auto syntax = parse_entity_syntax(bytes);
    if (!syntax) {
        return std::unexpected(describe("header fields", syntax.error()));
    }
    entity result;
    result.headers = std::move(syntax->headers);
    result.body    = syntax->body;

    if (const header_field* field = result.find("Content-Type")) {
        auto type = parse_content_type(field->value);
        if (!type) {
            return std::unexpected(describe("Content-Type", type.error()));
        }
        result.type = std::move(*type);
    }
    if (const header_field* field = result.find("Content-Transfer-Encoding")) {
        auto encoding = parse_transfer_encoding(field->value);
        if (!encoding) {
            return std::unexpected(describe("Content-Transfer-Encoding '" + field->value + "'", encoding.error()));
        }
        result.encoding = *encoding;
    }
    if (result.is_multipart()) {
        std::optional<std::string> boundary = result.type.parameter("boundary");
        if (!boundary || boundary->empty()) {
            return std::unexpected("multipart without a boundary parameter");
        }
        auto parts = parse_multipart_body(result.body, *boundary);
        if (!parts) {
            return std::unexpected(describe("multipart body", parts.error()));
        }
        for (std::string_view part : *parts) {
            auto child = parse_entity_at_depth(part, depth + 1);
            if (!child) {
                return std::unexpected(std::move(child.error()));
            }
            result.parts.push_back(std::move(*child));
        }
    }
    return result;
}

// ASCII, which is what white space and plain header text are, as code points.
void append_ascii(std::u32string& out, std::string_view text) {
    for (char c : text) {
        out.push_back(static_cast<char32_t>(static_cast<unsigned char>(c)));
    }
}

} // namespace

const header_field* entity::find(std::string_view name) const {
    for (const header_field& field : headers) {
        if (equal_ignoring_case(field.name, name)) {
            return &field;
        }
    }
    return nullptr;
}

bool entity::is_multipart() const { return type.type == "multipart"; }

bool entity::is_text() const { return type.type == "text"; }

std::string entity::content_type() const { return type.type + "/" + type.subtype; }

std::string entity::charset() const {
    if (!is_text()) {
        return {};
    }
    std::string name = type.parameter("charset").value_or("us-ascii");
    for (char& c : name) { // charset names are case-insensitive; keep them in lower case
        c = parse::ascii_lower(c);
    }
    return name;
}

std::expected<entity, std::string> parse_entity(std::string_view bytes) { return parse_entity_at_depth(bytes, 0); }

std::string decode_body(const entity& leaf) {
    switch (leaf.encoding) {
    case transfer_encoding::base64:
        return collect_bytes(leaf.body | base64_decode);
    case transfer_encoding::quoted_printable:
        return collect_bytes(leaf.body | quoted_printable_decode);
    case transfer_encoding::seven_bit:
    case transfer_encoding::eight_bit:
    case transfer_encoding::binary:
        break;
    }
    return std::string{leaf.body};
}

std::expected<std::u32string, std::string> decode_body_text(const entity& leaf) {
    namespace bt = beman::transcoding;
    // The part's charset is in the message, so the decoder is chosen at run
    // time.  Nothing is collected until the end: the transfer decoding and the
    // charset decoding are both views, and one pass produces the text.
    const auto charset = leaf.charset();
    const auto e       = bt::whatwg::get_encoding(charset);
    if (!e) {
        return std::unexpected("unknown charset '" + charset + "'");
    }
    const auto text = bt::decode(bt::whatwg::dynamic_decoder{*e});
    switch (leaf.encoding) {
    case transfer_encoding::base64:
        return leaf.body | base64_decode | text | std::ranges::to<std::u32string>();
    case transfer_encoding::quoted_printable:
        return leaf.body | quoted_printable_decode | text | std::ranges::to<std::u32string>();
    case transfer_encoding::seven_bit:
    case transfer_encoding::eight_bit:
    case transfer_encoding::binary:
        break;
    }
    return leaf.body | text | std::ranges::to<std::u32string>();
}

header_text decode_header_text(std::string_view value) {
    header_text result;
    auto        chunks = parse_unstructured(value);
    if (!chunks) { // unstructured text accepts everything, so this is not reached
        append_ascii(result.text, value);
        return result;
    }
    // RFC 2047 section 6.2: white space between two encoded-words is not part of
    // the text.  It is held back until what follows is known.
    std::string held_space;
    bool        after_encoded_word = false;
    for (const header_chunk& chunk : *chunks) {
        if (chunk.what == header_chunk::kind::space) {
            if (after_encoded_word) {
                held_space = chunk.text;
            } else {
                append_ascii(result.text, chunk.text);
            }
            continue;
        }
        std::expected<std::u32string, std::string> decoded;
        bool                                       is_decoded_word = false;
        if (chunk.what == header_chunk::kind::encoded_word) {
            std::string bytes = chunk.encoding == 'B' ? collect_bytes(std::string_view{chunk.text} | base64_decode)
                                                      : collect_bytes(std::string_view{chunk.text} | q_decode);
            decoded           = decode_text(bytes, chunk.charset);
            is_decoded_word   = decoded.has_value();
            if (!decoded) {
                result.problems.push_back(decoded.error());
                decoded = decode_text(chunk.spelling(), "utf-8");
            }
        } else {
            decoded = decode_text(chunk.text, "utf-8");
        }
        if (!(after_encoded_word && is_decoded_word)) {
            append_ascii(result.text, held_space);
        }
        held_space.clear();
        result.text += *decoded;
        after_encoded_word = is_decoded_word;
    }
    append_ascii(result.text, held_space);
    return result;
}

} // namespace mime_reader
