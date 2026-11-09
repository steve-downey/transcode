// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// A parser-combinator kit for the MIME reader example, copied from specgen
// (include/beman/specgen/parse/cursor.hpp, the same author and license), which
// adapted it from compile-time-scheme (src/smd/smdscheme/parser/cursor.hpp).
//
// Changes from specgen's copy: the namespace is example-local; the line and
// column tracking is gone, since a MIME message is bytes and not lines of
// source text; the cursor reads raw bytes, and `char` is only the byte's
// spelling here.  Nothing is interpreted as text until a part's charset says
// how.

#ifndef EXAMPLES_MIME_READER_CURSOR_HPP
#define EXAMPLES_MIME_READER_CURSOR_HPP

#include <cstddef>
#include <string_view>

namespace mime_reader::parse {

// An immutable view of the bytes not yet consumed, with the offset of the
// first of them in the original input.  Advancing returns a new cursor, so a
// parser can checkpoint and backtrack by keeping the old one.
class cursor {
  public:
    constexpr explicit cursor(std::string_view bytes);

    constexpr bool             empty() const;
    constexpr char             peek() const;      // precondition: !empty()
    constexpr cursor           bump() const;      // precondition: !empty()
    constexpr std::size_t      offset() const;    // offset from the start of the input
    constexpr std::string_view remaining() const; // the bytes not yet consumed

    // A cursor that has consumed the first `n` bytes of `remaining()`.
    constexpr cursor advance(std::size_t n) const;

  private:
    std::string_view input_;
    std::size_t      offset_ = 0;
};

constexpr cursor::cursor(std::string_view bytes) : input_{bytes} {}

constexpr bool cursor::empty() const { return input_.empty(); }

constexpr char cursor::peek() const { return input_.front(); }

constexpr cursor cursor::bump() const { return advance(1); }

constexpr cursor cursor::advance(std::size_t n) const {
    cursor next{*this};
    next.input_.remove_prefix(n);
    next.offset_ += n;
    return next;
}

constexpr std::size_t cursor::offset() const { return offset_; }

constexpr std::string_view cursor::remaining() const { return input_; }

} // namespace mime_reader::parse

#endif // EXAMPLES_MIME_READER_CURSOR_HPP
