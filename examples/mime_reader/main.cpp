// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// A MIME reader: prints a plain-text summary of the message in the file named
// on the command line, as UTF-8.
//
// A message is bytes until a part says how it is encoded.  The bytes are
// parsed with the grammar in grammar.cpp, a body's transfer encoding is undone
// by the views in transfer.hpp, and `decode_text` in charset.cpp turns the
// result into code points using the charset the part declares.

#include <examples/mime_reader/charset.hpp>
#include <examples/mime_reader/mime.hpp>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

using mime_reader::entity;

// Line breaks in text are CRLF or LF in the message; the summary has LF.
std::u32string normalize_line_breaks(const std::u32string& text) {
    std::u32string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == U'\r' && i + 1 < text.size() && text[i + 1] == U'\n') {
            continue;
        }
        out.push_back(text[i]);
    }
    return out;
}

void print_header(std::string_view label, const entity& message, std::string_view name) {
    const mime_reader::header_field* field = message.find(name);
    if (field == nullptr) {
        return;
    }
    mime_reader::header_text decoded = mime_reader::decode_header_text(field->value);
    std::cout << label << ": " << mime_reader::encode_utf8(decoded.text) << '\n';
    for (const std::string& problem : decoded.problems) {
        std::cout << "  error: " << problem << '\n';
    }
}

// Each line of the text, marked so that blank lines and leading white space
// are visible and unambiguous.
void print_text(const std::u32string& text) {
    std::size_t start = 0;
    while (start < text.size()) {
        std::size_t         end  = text.find(U'\n', start);
        std::size_t         stop = end == std::u32string::npos ? text.size() : end;
        std::u32string_view line{text.data() + start, stop - start};
        std::cout << (line.empty() ? "  >" : "  > ") << mime_reader::encode_utf8(line) << '\n';
        start = stop + 1;
    }
}

void print_leaf(const entity& leaf, const std::string& path) {
    std::cout << "part " << path << '\n';
    std::cout << "  content-type: " << leaf.content_type() << '\n';
    std::cout << "  charset: " << (leaf.is_text() ? leaf.charset() : std::string{"-"}) << '\n';
    std::cout << "  transfer-encoding: " << mime_reader::to_string(leaf.encoding) << '\n';

    std::string bytes = mime_reader::decode_body(leaf);
    if (!leaf.is_text()) {
        std::cout << "  bytes: " << bytes.size() << '\n';
        return;
    }
    auto text = mime_reader::decode_text(bytes, leaf.charset());
    if (!text) {
        std::cout << "  error: " << text.error() << '\n';
        return;
    }
    std::u32string lines = normalize_line_breaks(*text);
    std::cout << "  characters: " << lines.size() << '\n';
    std::cout << "  text:\n";
    print_text(lines);
}

void print_parts(const entity& node, const std::string& path) {
    if (!node.is_multipart()) {
        print_leaf(node, path.empty() ? "1" : path);
        return;
    }
    for (std::size_t i = 0; i < node.parts.size(); ++i) {
        std::string child = (path.empty() ? "" : path + ".") + std::to_string(i + 1);
        print_parts(node.parts[i], child);
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " message-file\n";
        return 2;
    }
    std::ifstream file{argv[1], std::ios::binary};
    if (!file) {
        std::cerr << "cannot read " << argv[1] << '\n';
        return 2;
    }
    std::string bytes{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};

    auto message = mime_reader::parse_entity(bytes);
    if (!message) {
        std::cerr << "error: " << message.error() << '\n';
        return 1;
    }
    print_header("Subject", *message, "Subject");
    print_header("From", *message, "From");
    print_parts(*message, "");
    return 0;
}
