// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <examples/mime_reader/transfer.hpp>
#include <examples/mime_reader/transfer.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>

namespace mime_reader {

namespace {

// The base64 alphabet, RFC 2045 table 1; -1 for any other byte.
int base64_value(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if (c == '+') {
        return 62;
    }
    if (c == '/') {
        return 63;
    }
    return -1;
}

int hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

bool is_wsp(char c) { return c == ' ' || c == '\t'; }

// The length of the line break at `i`: 2 for CRLF, 1 for a bare LF, else 0.
std::size_t line_break_at(std::string_view text, std::size_t i) {
    if (i < text.size() && text[i] == '\n') {
        return 1;
    }
    if (i + 1 < text.size() && text[i] == '\r' && text[i + 1] == '\n') {
        return 2;
    }
    return 0;
}

} // namespace

// ---------------------------------------------------------------------------
// base64
// ---------------------------------------------------------------------------

base64_decode_view::base64_decode_view(std::string_view text) : text_{text} {}

base64_decode_view::iterator base64_decode_view::begin() const { return iterator{text_, 0}; }

std::default_sentinel_t base64_decode_view::end() const { return std::default_sentinel; }

base64_decode_view::iterator::iterator(std::string_view text, std::size_t pos) : text_{text}, pos_{pos} { fill(); }

char base64_decode_view::iterator::operator*() const { return buf_[i_]; }

base64_decode_view::iterator& base64_decode_view::iterator::operator++() {
    if (++i_ == n_) {
        pos_ = next_;
        i_   = 0;
        fill();
    }
    return *this;
}

base64_decode_view::iterator base64_decode_view::iterator::operator++(int) {
    iterator before{*this};
    ++*this;
    return before;
}

void base64_decode_view::iterator::fill() {
    std::array<unsigned, 4> sextets{};
    unsigned                count = 0;
    std::size_t             p     = pos_;
    while (count < 4 && p < text_.size()) {
        char c = text_[p];
        int  v = base64_value(c);
        if (v >= 0) {
            sextets[count++] = static_cast<unsigned>(v);
            ++p;
        } else if (c == '=') {
            p = text_.size(); // padding: nothing follows that is data
            break;
        } else {
            ++p; // line breaks and other noise
        }
    }
    next_ = p;
    if (count < 2) { // not even one whole byte: the end
        n_   = 0;
        pos_ = next_ = text_.size();
        return;
    }
    n_      = count - 1;
    buf_[0] = static_cast<char>((sextets[0] << 2) | (sextets[1] >> 4));
    buf_[1] = static_cast<char>(((sextets[1] & 0xFU) << 4) | (sextets[2] >> 2));
    buf_[2] = static_cast<char>(((sextets[2] & 0x3U) << 6) | sextets[3]);
}

// ---------------------------------------------------------------------------
// quoted-printable
// ---------------------------------------------------------------------------

quoted_printable_decode_view::quoted_printable_decode_view(std::string_view text, quoted_printable_dialect dialect)
    : text_{text}, dialect_{dialect} {}

quoted_printable_decode_view::iterator quoted_printable_decode_view::begin() const {
    return iterator{text_, dialect_};
}

std::default_sentinel_t quoted_printable_decode_view::end() const { return std::default_sentinel; }

quoted_printable_decode_view::iterator::iterator(std::string_view text, quoted_printable_dialect dialect)
    : text_{text}, dialect_{dialect} {
    fill();
}

char quoted_printable_decode_view::iterator::operator*() const { return cur_; }

quoted_printable_decode_view::iterator& quoted_printable_decode_view::iterator::operator++() {
    pos_ = next_;
    fill();
    return *this;
}

quoted_printable_decode_view::iterator quoted_printable_decode_view::iterator::operator++(int) {
    iterator before{*this};
    ++*this;
    return before;
}

void quoted_printable_decode_view::iterator::fill() {
    const bool in_body = dialect_ == quoted_printable_dialect::body;
    while (pos_ < text_.size()) {
        const char c = text_[pos_];
        if (c == '=') {
            if (pos_ + 2 < text_.size() && hex_value(text_[pos_ + 1]) >= 0 && hex_value(text_[pos_ + 2]) >= 0) {
                cur_  = static_cast<char>(hex_value(text_[pos_ + 1]) * 16 + hex_value(text_[pos_ + 2]));
                next_ = pos_ + 3;
                has_  = true;
                return;
            }
            if (in_body) {
                if (std::size_t n = line_break_at(text_, pos_ + 1)) { // soft line break
                    pos_ += 1 + n;
                    continue;
                }
            }
            cur_  = '='; // not an escape: keep it
            next_ = pos_ + 1;
            has_  = true;
            return;
        }
        if (!in_body && c == '_') {
            cur_  = ' ';
            next_ = pos_ + 1;
            has_  = true;
            return;
        }
        if (in_body && is_wsp(c)) {
            std::size_t q = pos_;
            while (q < text_.size() && is_wsp(text_[q])) {
                ++q;
            }
            if (q == text_.size() || line_break_at(text_, q) != 0) { // padding at the end of a line
                pos_ = q;
                continue;
            }
        }
        cur_  = c;
        next_ = pos_ + 1;
        has_  = true;
        return;
    }
    has_ = false;
    pos_ = next_ = text_.size();
}

// ---------------------------------------------------------------------------
// the adaptors
// ---------------------------------------------------------------------------

base64_decode_view base64_decode_fn::operator()(std::string_view text) const { return base64_decode_view{text}; }

quoted_printable_decode_view quoted_printable_decode_fn::operator()(std::string_view text) const {
    return quoted_printable_decode_view{text, quoted_printable_dialect::body};
}

quoted_printable_decode_view q_decode_fn::operator()(std::string_view text) const {
    return quoted_printable_decode_view{text, quoted_printable_dialect::encoded_word};
}

} // namespace mime_reader
