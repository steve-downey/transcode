// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>

#include <iconv.h>

#include <cerrno>
#include <iterator>
#include <ranges>
#include <span>

namespace beman::transcoding {

// iconv_functions bundles the three POSIX iconv lifecycle callables.
// Inject a mock implementation in tests to avoid OS libc dependencies.
struct iconv_functions {
    iconv_t (*open)(const char* tocode, const char* fromcode);
    size_t (*convert)(iconv_t cd, char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft);
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

        iconv_t         handle_;
        IconvFns        fns_;
        std::span<char> buffer_;
        char*           output_pos_;
        char*           output_end_;
        // Accumulates unconsumed input bytes across load() calls so that
        // multi-byte sequences can be assembled before passing to iconv.
        char      staging_[4];
        size_t    staging_len_;
        base_iter current_;
        base_sent end_;
        bool      done_;

        iterator(const iterator&)            = delete;
        iterator& operator=(const iterator&) = delete;

        // Fills output_pos_/output_end_ with the next batch of converted bytes.
        // Handles EINVAL (incomplete sequence) by accumulating more input, and
        // E2BIG/EILSEQ by yielding partial output or skipping one staging byte.
        // Sets done_ = true when all input and staging bytes are exhausted.
        void load();

        // Only iconv_transcode_view::begin() may construct an iterator.
        friend class iconv_transcode_view;
        iterator(iconv_t handle, IconvFns fns, std::span<char> buffer, base_iter current, base_sent end);

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
    return iterator(fns_.open(to_, from_), fns_, buffer_, std::ranges::begin(base_), std::ranges::end(base_));
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
iconv_transcode_view<IconvFns, R>::iterator::iterator(
    iconv_t handle, IconvFns fns, std::span<char> buffer, base_iter current, base_sent end)
    : handle_(handle),
      fns_(std::move(fns)),
      buffer_(buffer),
      output_pos_(buffer.data()),
      output_end_(buffer.data()),
      staging_len_(0),
      current_(std::move(current)),
      end_(std::move(end)),
      done_(handle == (iconv_t)-1) {
    if (!done_)
        load();
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
void iconv_transcode_view<IconvFns, R>::iterator::load() {
    while (true) {
        // Append one input byte to staging (if any remains).
        if (current_ != end_) {
            staging_[staging_len_++] = static_cast<char>(*current_);
            ++current_;
        } else if (staging_len_ == 0) {
            done_ = true;
            return;
        }
        // else: staging holds bytes from a previous EINVAL; try to convert as-is.

        char*  in_ptr  = staging_;
        size_t inleft  = staging_len_;
        char*  out_ptr = buffer_.data();
        size_t outleft = buffer_.size();
        size_t rc      = fns_.convert(handle_, &in_ptr, &inleft, &out_ptr, &outleft);

        output_pos_ = buffer_.data();
        output_end_ = out_ptr;

        // Shift consumed bytes out of staging_.
        size_t consumed = staging_len_ - inleft;
        if (consumed > 0) {
            for (size_t i = 0; i < inleft; ++i)
                staging_[i] = staging_[consumed + i];
            staging_len_ = inleft;
        }

        if (rc != (size_t)-1) {
            if (output_pos_ < output_end_)
                return;
            if (staging_len_ == 0 && current_ == end_) {
                done_ = true;
                return;
            }
            continue;
        }

        if (errno == EINVAL) {
            // Incomplete input sequence: need more bytes before converting.
            if (current_ == end_) {
                // No more input; discard the partial sequence.
                staging_len_ = 0;
                done_        = true;
                return;
            }
            // Loop to read another byte into staging.
            if (output_pos_ < output_end_)
                return;
            continue;
        }

        // E2BIG or EILSEQ: yield whatever was written to the output buffer.
        if (output_pos_ < output_end_)
            return;
        // Nothing written; skip one staging byte to avoid stalling.
        if (staging_len_ > 0) {
            for (size_t i = 0; i + 1 < staging_len_; ++i)
                staging_[i] = staging_[i + 1];
            --staging_len_;
        }
        if (staging_len_ == 0 && current_ == end_) {
            done_ = true;
            return;
        }
    }
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_view<IconvFns, R>::iterator::iterator(iterator&& other) noexcept
    : handle_(other.handle_),
      fns_(std::move(other.fns_)),
      buffer_(other.buffer_),
      output_pos_(other.output_pos_),
      output_end_(other.output_end_),
      staging_len_(other.staging_len_),
      current_(std::move(other.current_)),
      end_(std::move(other.end_)),
      done_(other.done_) {
    for (size_t i = 0; i < other.staging_len_; ++i)
        staging_[i] = other.staging_[i];
    other.handle_      = (iconv_t)-1;
    other.done_        = true;
    other.staging_len_ = 0;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_view<IconvFns, R>::iterator::operator=(iterator&& other) noexcept -> iterator& {
    if (this != &other) {
        if (handle_ != (iconv_t)-1)
            fns_.close(handle_);
        handle_      = other.handle_;
        fns_         = std::move(other.fns_);
        buffer_      = other.buffer_;
        output_pos_  = other.output_pos_;
        output_end_  = other.output_end_;
        staging_len_ = other.staging_len_;
        for (size_t i = 0; i < other.staging_len_; ++i)
            staging_[i] = other.staging_[i];
        current_           = std::move(other.current_);
        end_               = std::move(other.end_);
        done_              = other.done_;
        other.handle_      = (iconv_t)-1;
        other.done_        = true;
        other.staging_len_ = 0;
    }
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_view<IconvFns, R>::iterator::~iterator() {
    if (handle_ != (iconv_t)-1)
        fns_.close(handle_);
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
char iconv_transcode_view<IconvFns, R>::iterator::operator*() const {
    return *output_pos_;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_view<IconvFns, R>::iterator::operator++() -> iterator& {
    ++output_pos_;
    if (output_pos_ == output_end_)
        load();
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
void iconv_transcode_view<IconvFns, R>::iterator::operator++(int) {
    ++*this;
}

// iconv_transcode_closure<IconvFns> — pipe adapter for iconv_transcode_view.
//
// Stores the callable set, encoding pair, and output buffer so that
// operator| can construct the view lazily:
//   auto v = input | iconv_transcode_closure<iconv_functions>{fns, "UTF-8", "UTF-32LE", buf};
template <typename IconvFns>
struct iconv_transcode_closure {
    IconvFns        fns_;
    const char*     from_;
    const char*     to_;
    std::span<char> buffer_;

    template <legacy_byte_range R>
    auto operator()(R&& r) const {
        return iconv_transcode_view<IconvFns, std::views::all_t<R>>(
            std::views::all(std::forward<R>(r)), fns_, from_, to_, buffer_);
    }

    template <legacy_byte_range R>
    friend auto operator|(R&& r, const iconv_transcode_closure& self) {
        return self(std::forward<R>(r));
    }
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP
