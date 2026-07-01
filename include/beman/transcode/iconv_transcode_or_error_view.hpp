// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_OR_ERROR_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_OR_ERROR_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <iconv.h>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <algorithm>
    #include <cerrno>
    #include <expected>
    #include <iterator>
    #include <memory>
    #include <ranges>
    #include <span>
    #include <utility>

#endif
namespace beman::transcoding {

// iconv_transcode_or_error_view<IconvFns, R>
//
// Like iconv_transcode_view but yields std::expected<char, iconv_error> so
// that EILSEQ (invalid sequence), EINVAL-at-end (incomplete sequence), and
// E2BIG-with-nothing-written (output buffer too small) are surfaced to the
// caller instead of being silently discarded.
//
// The iterator is move-only for the same reason as iconv_transcode_view:
// iconv_t is an OS-managed handle that cannot be copied.
template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
class iconv_transcode_or_error_view : public std::ranges::view_interface<iconv_transcode_or_error_view<IconvFns, R>> {
    R               base_;
    IconvFns        fns_;
    const char*     from_;
    const char*     to_;
    std::span<char> buffer_;

  public:
    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;
        using result_t  = std::expected<char, iconv_error>;

        iconv_t         handle_;
        IconvFns        fns_;
        std::span<char> buffer_;
        char*           output_pos_;
        char*           output_end_;
        char            staging_[64];
        size_t          staging_len_{0};
        base_iter       current_;
        base_sent       end_;
        bool            done_;
        bool            flushed_{false};
        bool            has_error_{false};
        iconv_error     error_value_{};

        void load();

        friend class iconv_transcode_or_error_view;
        iterator(iconv_t handle, IconvFns fns, std::span<char> buffer, base_iter current, base_sent end);

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = result_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = result_t;

        iterator(const iterator&)            = delete;
        iterator& operator=(const iterator&) = delete;

        iterator(iterator&&) noexcept;
        iterator& operator=(iterator&&) noexcept;
        ~iterator();

        const base_iter& base() const noexcept { return current_; }

        result_t  operator*() const;
        iterator& operator++();
        void      operator++(int);

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

    explicit iconv_transcode_or_error_view(
        R base, IconvFns fns, const char* from, const char* to, std::span<char> buf);

    const R& base() const& noexcept { return base_; }
    R        base() && { return std::move(base_); }

    iterator                begin();
    std::default_sentinel_t end() const;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions: iconv_transcode_or_error_view
// ---------------------------------------------------------------------------

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_or_error_view<IconvFns, R>::iconv_transcode_or_error_view(
    R base, IconvFns fns, const char* from, const char* to, std::span<char> buf)
    : base_(std::move(base)), fns_(std::move(fns)), from_(from), to_(to), buffer_(buf) {}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_or_error_view<IconvFns, R>::begin() -> iterator {
    return iterator(fns_.open(to_, from_), fns_, buffer_, std::ranges::begin(base_), std::ranges::end(base_));
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
std::default_sentinel_t iconv_transcode_or_error_view<IconvFns, R>::end() const {
    return std::default_sentinel;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: iconv_transcode_or_error_view::iterator
// ---------------------------------------------------------------------------

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_or_error_view<IconvFns, R>::iterator::iterator(
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
void iconv_transcode_or_error_view<IconvFns, R>::iterator::load() {
    char*  out_ptr = buffer_.data();
    size_t outleft = buffer_.size();

    // Fast path: contiguous+sized input with empty staging — pass directly to iconv.
    if constexpr (std::contiguous_iterator<base_iter> && std::sized_sentinel_for<base_sent, base_iter>) {
        if (staging_len_ == 0 && current_ != end_) {
            const auto* raw_ptr   = reinterpret_cast<const char*>(std::to_address(current_));
            auto        remaining = static_cast<size_t>(end_ - current_);
            char*       in_ptr    = const_cast<char*>(raw_ptr);
            size_t      inleft    = remaining;
            size_t      rc        = fns_.convert(handle_, &in_ptr, &inleft, &out_ptr, &outleft);
            size_t      consumed  = remaining - inleft;
            current_ += static_cast<std::ptrdiff_t>(consumed);

            output_pos_ = buffer_.data();
            output_end_ = out_ptr;

            if (rc != iconv_error_rc) {
                if (output_pos_ == output_end_ && current_ == end_)
                    goto do_flush;
                return;
            }
            if (errno == E2BIG) {
                if (output_pos_ < output_end_)
                    return;
                // No output: buffer too small for one conversion unit.
                if (current_ != end_)
                    ++current_;
                has_error_   = true;
                error_value_ = iconv_error::output_full;
                return;
            }
            if (errno == EINVAL) {
                // Save unconsumed residue to staging for next call.
                if (inleft > 0 && inleft <= sizeof(staging_)) {
                    std::copy_n(in_ptr, inleft, staging_);
                    staging_len_ = inleft;
                    current_ += static_cast<std::ptrdiff_t>(inleft);
                }
                if (output_pos_ < output_end_)
                    return;
                if (staging_len_ == 0 || current_ == end_) {
                    staging_len_ = 0;
                    has_error_   = true;
                    error_value_ = iconv_error::incomplete_sequence;
                }
                return;
            }
            // EILSEQ
            if (output_pos_ < output_end_)
                return; // yield output; invalid byte still at current_ for retry
            if (current_ != end_)
                ++current_;
            has_error_   = true;
            error_value_ = iconv_error::invalid_sequence;
            return;
        }
    }

    {
        // Slow path: fill staging from non-contiguous input or drain residue.
        while (staging_len_ < sizeof(staging_) && current_ != end_) {
            staging_[staging_len_++] = static_cast<char>(*current_);
            ++current_;
        }
        if (staging_len_ == 0) {
            output_pos_ = output_end_ = buffer_.data();
            goto do_flush;
        }

        char*  in_ptr   = staging_;
        size_t inleft   = staging_len_;
        size_t rc       = fns_.convert(handle_, &in_ptr, &inleft, &out_ptr, &outleft);
        size_t consumed = staging_len_ - inleft;
        if (consumed > 0 && inleft > 0)
            std::copy_n(in_ptr, inleft, staging_);
        staging_len_ = inleft;

        output_pos_ = buffer_.data();
        output_end_ = out_ptr;

        if (rc != iconv_error_rc) {
            if (output_pos_ == output_end_ && staging_len_ == 0 && current_ == end_)
                goto do_flush;
            return;
        }
        if (errno == EINVAL) {
            if (output_pos_ < output_end_)
                return;
            if (current_ == end_) {
                staging_len_ = 0;
                has_error_   = true;
                error_value_ = iconv_error::incomplete_sequence;
            }
            return;
        }
        if (errno == EILSEQ) {
            if (output_pos_ < output_end_)
                return; // yield output; residue still in staging for retry
            if (staging_len_ > 0) {
                std::copy_n(staging_ + 1, staging_len_ - 1, staging_);
                --staging_len_;
            }
            has_error_   = true;
            error_value_ = iconv_error::invalid_sequence;
            return;
        }
        // E2BIG
        if (output_pos_ < output_end_)
            return;
        if (staging_len_ > 0) {
            std::copy_n(staging_ + 1, staging_len_ - 1, staging_);
            --staging_len_;
        }
        has_error_   = true;
        error_value_ = iconv_error::output_full;
        return;
    }

do_flush:
    if (!flushed_) {
        flushed_          = true;
        char*  flush_out  = buffer_.data();
        size_t flush_left = buffer_.size();
        fns_.convert(handle_, nullptr, nullptr, &flush_out, &flush_left);
        output_pos_ = buffer_.data();
        output_end_ = flush_out;
        if (output_pos_ < output_end_)
            return;
    }
    done_ = true;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_or_error_view<IconvFns, R>::iterator::iterator(iterator&& other) noexcept
    : handle_(other.handle_),
      fns_(std::move(other.fns_)),
      buffer_(other.buffer_),
      output_pos_(other.output_pos_),
      output_end_(other.output_end_),
      staging_len_(other.staging_len_),
      current_(std::move(other.current_)),
      end_(std::move(other.end_)),
      done_(other.done_),
      flushed_(other.flushed_),
      has_error_(other.has_error_),
      error_value_(other.error_value_) {
    for (size_t i = 0; i < other.staging_len_; ++i)
        staging_[i] = other.staging_[i];
    other.handle_      = (iconv_t)-1;
    other.done_        = true;
    other.staging_len_ = 0;
    other.has_error_   = false;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_or_error_view<IconvFns, R>::iterator::operator=(iterator&& other) noexcept -> iterator& {
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
        has_error_         = other.has_error_;
        error_value_       = other.error_value_;
        other.handle_      = (iconv_t)-1;
        other.done_        = true;
        other.staging_len_ = 0;
        other.has_error_   = false;
    }
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
iconv_transcode_or_error_view<IconvFns, R>::iterator::~iterator() {
    if (handle_ != (iconv_t)-1)
        fns_.close(handle_);
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_or_error_view<IconvFns, R>::iterator::operator*() const -> result_t {
    if (has_error_)
        return std::unexpected(error_value_);
    return *output_pos_;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
auto iconv_transcode_or_error_view<IconvFns, R>::iterator::operator++() -> iterator& {
    if (has_error_) {
        has_error_ = false;
        load();
        return *this;
    }
    ++output_pos_;
    if (output_pos_ == output_end_)
        load();
    return *this;
}

template <typename IconvFns, std::ranges::input_range R>
    requires legacy_byte_range<R>
void iconv_transcode_or_error_view<IconvFns, R>::iterator::operator++(int) {
    ++*this;
}

// iconv_transcode_or_error_closure<IconvFns> — pipe adapter for iconv_transcode_or_error_view.
template <typename IconvFns>
struct iconv_transcode_or_error_closure {
    IconvFns        fns_;
    const char*     from_;
    const char*     to_;
    std::span<char> buffer_;

    template <legacy_byte_range R>
    auto operator()(R&& r) const {
        return iconv_transcode_or_error_view<IconvFns, std::views::all_t<R>>(
            std::views::all(std::forward<R>(r)), fns_, from_, to_, buffer_);
    }

    template <legacy_byte_range R>
    friend auto operator|(R&& r, const iconv_transcode_or_error_closure& self) {
        return self(std::forward<R>(r));
    }
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_TRANSCODE_OR_ERROR_VIEW_HPP
