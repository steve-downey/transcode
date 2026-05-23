// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP
#define BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>

#include <iconv.h>

#include <iterator>
#include <ranges>
#include <span>

namespace beman::transcoding {

// iconv_functions bundles the three POSIX iconv lifecycle callables.
// Inject a mock implementation in tests to avoid OS libc dependencies.
struct iconv_functions {
    iconv_t (*open)(const char* tocode, const char* fromcode);
    size_t (*convert)(iconv_t cd,
                      char**   inbuf,
                      size_t*  inbytesleft,
                      char**   outbuf,
                      size_t*  outbytesleft);
    int (*close)(iconv_t cd);
};

// iconv_transcode_view<IconvFns, R>
//
// Adapts an input range R of legacy bytes into a byte-by-byte output range
// in the target encoding, using an injected iconv-compatible callable set.
// The iterator is intentionally move-only: iconv_t is an OS-managed opaque
// handle that cannot be copied or shared.
//
// The caller provides an external output buffer (std::span<char>) to avoid
// internal heap allocation.
template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
class iconv_transcode_view : public std::ranges::view_interface<iconv_transcode_view<IconvFns, R>> {
    R               base_;
    IconvFns        fns_;
    const char*     from_;
    const char*     to_;
    std::span<char> buffer_;

  public:
    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;

        iconv_t   handle_;
        base_iter current_;
        base_sent end_;
        bool      done_;

        iterator(const iterator&)            = delete;
        iterator& operator=(const iterator&) = delete;

        // Only iconv_transcode_view::begin() may construct an iterator.
        friend class iconv_transcode_view;
        iterator(iconv_t handle, base_iter current, base_sent end)
            : handle_(handle), current_(std::move(current)), end_(std::move(end)), done_(handle == (iconv_t)-1) {}

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char;
        using difference_type  = std::ptrdiff_t;
        using reference        = char;

        iterator(iterator&&) noexcept;
        iterator& operator=(iterator&&) noexcept;
        ~iterator();

        char      operator*() const;
        iterator& operator++();
        void      operator++(int);

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    explicit iconv_transcode_view(R base, IconvFns fns, const char* from, const char* to, std::span<char> buf);

    iterator                begin();
    std::default_sentinel_t end() const;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions: iconv_transcode_view
// ---------------------------------------------------------------------------

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_view<IconvFns, R>::iconv_transcode_view(
    R base, IconvFns fns, const char* from, const char* to, std::span<char> buf)
    : base_(std::move(base)), fns_(std::move(fns)), from_(from), to_(to), buffer_(buf) {}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_view<IconvFns, R>::begin() -> iterator {
    return iterator(fns_.open(to_, from_), std::ranges::begin(base_), std::ranges::end(base_));
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
std::default_sentinel_t iconv_transcode_view<IconvFns, R>::end() const {
    return std::default_sentinel;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: iconv_transcode_view::iterator
// ---------------------------------------------------------------------------

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_view<IconvFns, R>::iterator::iterator(iterator&& other) noexcept
    : handle_(other.handle_),
      current_(std::move(other.current_)),
      end_(std::move(other.end_)),
      done_(other.done_) {
    other.handle_ = (iconv_t)-1;
    other.done_   = true;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_view<IconvFns, R>::iterator::operator=(iterator&& other) noexcept -> iterator& {
    if (this != &other) {
        handle_       = other.handle_;
        current_      = std::move(other.current_);
        end_          = std::move(other.end_);
        done_         = other.done_;
        other.handle_ = (iconv_t)-1;
        other.done_   = true;
    }
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_view<IconvFns, R>::iterator::~iterator() {
    // step 9: call fns_.close(handle_) when handle is valid
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
char iconv_transcode_view<IconvFns, R>::iterator::operator*() const {
    return '\0'; // stub: step 9 implements real output
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_view<IconvFns, R>::iterator::operator++() -> iterator& {
    done_ = true; // stub: step 9 implements real advancement
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
void iconv_transcode_view<IconvFns, R>::iterator::operator++(int) {
    ++*this;
}

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP
