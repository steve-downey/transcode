// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The content transfer encodings a MIME reader undoes, as lazy views of bytes.
//
//     body | base64_decode                RFC 2045 section 6.8
//     body | quoted_printable_decode      RFC 2045 section 6.7
//     word | q_decode                     RFC 2047 section 4.2, "Q" encoded-text
//
// Each is a forward range of `char` that decodes as it is iterated, so a part
// can be handed to a decoder without first being copied.  Their input is the
// contiguous bytes of the message, since a body is a slice of it.
//
// Both are lenient, as readers are.  Base64 skips whatever is not in the
// alphabet (line breaks, mostly) and stops at padding.  Quoted-printable keeps
// a stray "=" as it is.

#ifndef EXAMPLES_MIME_READER_TRANSFER_HPP
#define EXAMPLES_MIME_READER_TRANSFER_HPP

#include <array>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

namespace mime_reader {

class base64_decode_view : public std::ranges::view_interface<base64_decode_view> {
  public:
    class iterator;

    base64_decode_view() = default;
    explicit base64_decode_view(std::string_view text);

    iterator                begin() const;
    std::default_sentinel_t end() const;

  private:
    std::string_view text_;
};

class base64_decode_view::iterator {
  public:
    using value_type        = char;
    using difference_type   = std::ptrdiff_t;
    using iterator_concept  = std::forward_iterator_tag;
    using iterator_category = std::input_iterator_tag;

    iterator() = default;

    char      operator*() const;
    iterator& operator++();
    iterator  operator++(int);

    friend bool operator==(const iterator& a, const iterator& b) { return a.pos_ == b.pos_ && a.i_ == b.i_; }
    friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.n_ == 0; }

  private:
    friend class base64_decode_view;
    iterator(std::string_view text, std::size_t pos);

    void fill(); // decode the group of up to four characters at pos_

    std::string_view    text_;
    std::size_t         pos_  = 0;
    std::size_t         next_ = 0;
    std::array<char, 3> buf_{};
    unsigned            n_ = 0; // bytes in the group; 0 at the end
    unsigned            i_ = 0;
};

// The two quoted-printable dialects.  `body` is RFC 2045: "=" at the end of a
// line is a soft line break, and white space at the end of a line is padding
// added in transit.  `encoded_word` is RFC 2047's "Q": "_" is a space, and
// there are no lines.
enum class quoted_printable_dialect { body, encoded_word };

class quoted_printable_decode_view : public std::ranges::view_interface<quoted_printable_decode_view> {
  public:
    class iterator;

    quoted_printable_decode_view() = default;
    quoted_printable_decode_view(std::string_view text, quoted_printable_dialect dialect);

    iterator                begin() const;
    std::default_sentinel_t end() const;

  private:
    std::string_view         text_;
    quoted_printable_dialect dialect_ = quoted_printable_dialect::body;
};

class quoted_printable_decode_view::iterator {
  public:
    using value_type        = char;
    using difference_type   = std::ptrdiff_t;
    using iterator_concept  = std::forward_iterator_tag;
    using iterator_category = std::input_iterator_tag;

    iterator() = default;

    char      operator*() const;
    iterator& operator++();
    iterator  operator++(int);

    friend bool operator==(const iterator& a, const iterator& b) { return a.pos_ == b.pos_; }
    friend bool operator==(const iterator& it, std::default_sentinel_t) { return !it.has_; }

  private:
    friend class quoted_printable_decode_view;
    iterator(std::string_view text, quoted_printable_dialect dialect);

    void fill(); // find the next decoded byte at or after pos_

    std::string_view         text_;
    quoted_printable_dialect dialect_ = quoted_printable_dialect::body;
    std::size_t              pos_     = 0; // where the current byte's encoding starts
    std::size_t              next_    = 0; // where the next one's search starts
    char                     cur_     = 0;
    bool                     has_     = false;
};

struct base64_decode_fn {
    base64_decode_view operator()(std::string_view text) const;

    friend base64_decode_view operator|(std::string_view text, const base64_decode_fn& f) { return f(text); }
};

struct quoted_printable_decode_fn {
    quoted_printable_decode_view operator()(std::string_view text) const;

    friend quoted_printable_decode_view operator|(std::string_view text, const quoted_printable_decode_fn& f) {
        return f(text);
    }
};

struct q_decode_fn {
    quoted_printable_decode_view operator()(std::string_view text) const;

    friend quoted_printable_decode_view operator|(std::string_view text, const q_decode_fn& f) { return f(text); }
};

inline constexpr base64_decode_fn           base64_decode{};
inline constexpr quoted_printable_decode_fn quoted_printable_decode{};
inline constexpr q_decode_fn                q_decode{};

// The bytes of a decoded range, in a string.
template <std::ranges::input_range R>
std::string collect_bytes(R&& decoded) {
    std::string bytes;
    for (char c : decoded) {
        bytes.push_back(c);
    }
    return bytes;
}

} // namespace mime_reader

#endif // EXAMPLES_MIME_READER_TRANSFER_HPP
