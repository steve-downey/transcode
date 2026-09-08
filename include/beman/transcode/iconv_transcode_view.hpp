// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/concepts.hpp>
#include <beman/transcode/error.hpp>

#include <iconv.h>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <algorithm>
    #include <cerrno>
    #include <iterator>
    #include <memory>
    #include <ranges>
    #include <span>
    #include <utility>

#endif
namespace beman::transcoding {

// \ref{transcode.iconv}, iconv adaptors

//! \omit
inline constexpr size_t iconv_error_rc = static_cast<size_t>(-1);

//! \remarks The three POSIX `iconv` entry points a view calls, as a value the
//! program supplies.  `open` opens a conversion descriptor, `convert`
//! converts, and `close` closes it; each has the signature and the semantics
//! POSIX gives the function of the same name.  The views take this as a
//! template parameter rather than calling `::iconv` directly so that a program
//! can supply a different implementation of the same interface -- another
//! library's, or a test's -- and `make_real_iconv_fns` is the one bound to the
//! platform's.
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
//! \remarks `iconv_transcode_view<IconvFns, R>` presents the bytes of `R`
//! converted from the encoding named by `from` to the encoding named by `to`,
//! one element per output byte, using the `iconv` implementation `IconvFns`
//! names.  Conversion is lazy and proceeds in batches: the view converts into
//! the caller's buffer, yields those bytes, and converts again.
//!
//! What the encoding names mean, which pairs convert, and what a conversion
//! does with input the source encoding does not allow are the implementation's
//! `iconv`'s, not this specification's.  That is the point of the adaptor: it
//! gives an interface a program already has a ranges shape and a lifetime, and
//! it does not restate a table it does not own.
//!
//! A conversion failure is reported as `iconv_error`
//! \iref{transcode.errors}, which is the granularity POSIX reports at:
//! `EILSEQ`, `EINVAL` and `E2BIG` say *that* a byte sequence is not valid,
//! not why, so the WHATWG error vocabulary the other views use would be
//! claiming knowledge the OS does not return.
//! \remarks The output buffer is the caller's, and is not owned by the view.
//! Its contents between two increments are unspecified, and the program must
//! keep it alive for the lifetime of every iterator the view produces.
template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
class iconv_transcode_view : public std::ranges::view_interface<iconv_transcode_view<IconvFns, R>> {
    //! \expos
    R base_;
    //! \expos
    IconvFns fns_;
    //! \expos
    const char* from_;
    //! \expos
    const char* to_;
    //! \expos
    std::span<char> buffer_;

  public:
    //! \expos
    //! \seebelow
    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;

        //! \expos
        iconv_t         handle_;
        //! \expos
        IconvFns fns_;
        //! \expos
        std::span<char> buffer_;
        //! \expos
        char*           output_pos_;
        //! \expos
        char*           output_end_;
        // Accumulates unconsumed input bytes across load() calls so that
        // multi-byte sequences can be assembled before passing to iconv.
        //! \expos
        char      staging_[64];
        //! \expos
        size_t    staging_len_{0};
        //! \expos
        base_iter current_;
        //! \expos
        base_sent end_;
        //! \expos
        bool      done_;
        //! \expos
        bool      flushed_{false};

        // Fills output_pos_/output_end_ with the next batch of converted bytes.
        // Handles EINVAL (incomplete sequence) by accumulating more input, and
        // E2BIG/EILSEQ by yielding partial output or skipping one staging byte.
        // Sets done_ = true when all input and staging bytes are exhausted.
        //! \expos
        void load();

        // Only iconv_transcode_view::begin() may construct an iterator.
        friend class iconv_transcode_view;
        //! \expos
        iterator(iconv_t handle, IconvFns fns, std::span<char> buffer, base_iter current, base_sent end);

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char;
        using difference_type  = std::ptrdiff_t;
        using reference        = char;

        // \ref{transcode.iconv.iterator}, iterator operations
        iterator(const iterator&)            = delete;
        iterator& operator=(const iterator&) = delete;

        iterator(iterator&&) noexcept;
        iterator& operator=(iterator&&) noexcept;
        ~iterator();

        //! \returns The iterator into `R` this iterator reads from, positioned
        //! after the last byte handed to the conversion.
        const base_iter& base() const noexcept { return current_; }

        char      operator*() const;
        iterator& operator++();
        void      operator++(int);

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

    // \ref{transcode.iconv}, construction and access
    explicit iconv_transcode_view(R base, IconvFns fns, const char* from, const char* to, std::span<char> buf);

    //! \returns-equiv
    const R& base() const& noexcept { return base_; }
    //! \returns-equiv
    R base() && { return std::move(base_); }

    iterator                begin();
    //! \returns `default_sentinel`.
    std::default_sentinel_t end() const;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions: iconv_transcode_view
// ---------------------------------------------------------------------------

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \effects Initializes the view with `std::move(base)`, `std::move(fns)`,
//! `from`, `to` and `buf`.  No conversion descriptor is opened: `begin` opens
//! one.
iconv_transcode_view<IconvFns, R>::iconv_transcode_view(
    R base, IconvFns fns, const char* from, const char* to, std::span<char> buf)
    : base_(std::move(base)), fns_(std::move(fns)), from_(from), to_(to), buffer_(buf) {}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \returns An `$iterator$` over `$base$` holding a conversion descriptor
//! opened by `$fns$.open($to$, $from$)`.
//! \remarks Each call opens a descriptor, so a view converts once per `begin`.
//! The iterator owns the descriptor and closes it, which is what makes the
//! adaptor leak-free where the POSIX interface is not.
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

      current_(std::move(current)),
      end_(std::move(end)),
      done_(handle == (iconv_t)-1) {
    if (!done_)
        load();
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
void iconv_transcode_view<IconvFns, R>::iterator::load() {
    char*  out_ptr = buffer_.data();
    size_t outleft = buffer_.size();

    while (outleft > 0) {
        if constexpr (std::contiguous_iterator<base_iter> && std::sized_sentinel_for<base_sent, base_iter>) {
            if (staging_len_ == 0 && current_ != end_) {
                const auto* raw_ptr   = reinterpret_cast<const char*>(std::to_address(current_));
                auto        remaining = static_cast<size_t>(end_ - current_);
                char*       in_ptr    = const_cast<char*>(raw_ptr);
                size_t      inleft    = remaining;
                size_t      rc        = fns_.convert(handle_, &in_ptr, &inleft, &out_ptr, &outleft);
                size_t      consumed  = remaining - inleft;
                current_ += static_cast<std::ptrdiff_t>(consumed);
                if (rc != iconv_error_rc) {
                    if (current_ == end_)
                        break;
                    continue;
                }
                if (errno == E2BIG)
                    break;
                if (errno == EINVAL) {
                    if (inleft > 0 && inleft <= sizeof(staging_)) {
                        std::copy_n(in_ptr, inleft, staging_);
                        staging_len_ = inleft;
                        current_ += static_cast<std::ptrdiff_t>(inleft);
                    }
                    break;
                }
                // EILSEQ: skip one byte
                if (current_ != end_)
                    ++current_;
                if (current_ == end_ && staging_len_ == 0)
                    break;
                continue;
            }
        }

        // Slow path: fill staging from non-contiguous input or drain residue.
        while (staging_len_ < sizeof(staging_) && current_ != end_) {
            staging_[staging_len_++] = static_cast<char>(*current_);
            ++current_;
        }
        if (staging_len_ == 0)
            break;

        char*  in_ptr   = staging_;
        size_t inleft   = staging_len_;
        size_t rc       = fns_.convert(handle_, &in_ptr, &inleft, &out_ptr, &outleft);
        size_t consumed = staging_len_ - inleft;
        if (consumed > 0 && inleft > 0)
            std::copy_n(in_ptr, inleft, staging_);
        staging_len_ = inleft;

        if (rc != iconv_error_rc) {
            if (staging_len_ == 0 && current_ == end_)
                break;
            continue;
        }
        if (errno == E2BIG)
            break;
        if (errno == EINVAL) {
            if (current_ == end_) {
                staging_len_ = 0;
                break;
            }
            continue;
        }
        // EILSEQ: skip one staging byte.
        if (staging_len_ > 0) {
            std::copy_n(staging_ + 1, staging_len_ - 1, staging_);
            --staging_len_;
        }
        if (staging_len_ == 0 && current_ == end_)
            break;
    }

    output_pos_ = buffer_.data();
    output_end_ = out_ptr;

    if (output_pos_ == output_end_) {
        if (!flushed_) {
            flushed_          = true;
            char*  flush_out  = buffer_.data();
            size_t flush_left = buffer_.size();
            fns_.convert(handle_, nullptr, nullptr, &flush_out, &flush_left);
            output_pos_ = buffer_.data();
            output_end_ = flush_out;
        }
        if (output_pos_ == output_end_)
            done_ = true;
    }
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \effects Takes over `other`'s conversion descriptor and leaves `other`
//! holding none.
//! \remarks The iterator is move-only.  A conversion descriptor is an
//! OS-owned handle that cannot be duplicated, so copying one would either
//! close it twice or leak it.
iconv_transcode_view<IconvFns, R>::iterator::iterator(iterator&& other) noexcept
    : handle_(other.handle_),
      fns_(std::move(other.fns_)),
      buffer_(other.buffer_),
      output_pos_(other.output_pos_),
      output_end_(other.output_end_),
      staging_len_(other.staging_len_),
      current_(std::move(other.current_)),
      end_(std::move(other.end_)),
      done_(other.done_),
      flushed_(other.flushed_) {
    for (size_t i = 0; i < other.staging_len_; ++i)
        staging_[i] = other.staging_[i];
    other.handle_      = (iconv_t)-1;
    other.done_        = true;
    other.staging_len_ = 0;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \effects Closes the descriptor `*this` holds, if any, then takes over
//! `other`'s and leaves `other` holding none.
//! \returns `*this`.
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
        flushed_           = other.flushed_;
        other.handle_      = (iconv_t)-1;
        other.done_        = true;
        other.staging_len_ = 0;
    }
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \effects Closes the conversion descriptor, if `*this` holds one.
iconv_transcode_view<IconvFns, R>::iterator::~iterator() {
    if (handle_ != (iconv_t)-1)
        fns_.close(handle_);
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \returns The converted byte at the current position of the output buffer.
char iconv_transcode_view<IconvFns, R>::iterator::operator*() const {
    return *output_pos_;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \effects Advances to the next converted byte, converting more input when
//! the buffer is exhausted; if no input and no unconverted bytes remain, makes
//! `*this` equal to `default_sentinel`.
//! \returns `*this`.
//! \remarks Input the conversion does not accept is skipped one byte at a
//! time -- what POSIX reports as `EILSEQ` -- so the range ends where the input
//! does rather than at the first byte a converter refuses.  A program that
//! needs to know *that* it happened uses `iconv_transcode_or_error`
//! \iref{transcode.iconv}.
auto iconv_transcode_view<IconvFns, R>::iterator::operator++() -> iterator& {
    ++output_pos_;
    if (output_pos_ == output_end_)
        load();
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
//! \effects-equiv
void iconv_transcode_view<IconvFns, R>::iterator::operator++(int) {
    ++*this;
}

// iconv_transcode_closure<IconvFns> — pipe adapter for iconv_transcode_view.
//
// Stores the callable set, encoding pair, and output buffer so that
// operator| can construct the view lazily:
//   auto v = input | iconv_transcode_closure<iconv_functions>{fns, "UTF-8", "UTF-32LE", buf};
template <typename IconvFns>
//! \omit
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
