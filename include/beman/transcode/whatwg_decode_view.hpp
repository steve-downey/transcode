// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/utf8.hpp>
#include <beman/transcode/detail/x_user_defined.hpp>

#include <expected>
#include <iterator>
#include <ranges>

namespace beman::transcoding {

enum class codec { utf_8, replacement, x_user_defined };

// ---------------------------------------------------------------------------
// whatwg_decode_view — decodes bytes to char32_t, replacing errors with U+FFFD
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
class whatwg_decode_view : public std::ranges::view_interface<whatwg_decode_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;

        base_iter current_;
        base_sent end_;
        char32_t  value_{};
        bool      done_{false};

        constexpr void load();

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = char32_t;

        constexpr iterator(base_iter current, base_sent end);

        constexpr char32_t  operator*() const;
        constexpr iterator& operator++();
        constexpr void      operator++(int);

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_decode_view(R base);

    constexpr iterator                begin();
    constexpr std::default_sentinel_t end() const;
};

template <codec C>
struct whatwg_decode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const whatwg_decode_closure& self) {
        return self(std::forward<R>(r));
    }

    // Explicit diagnostic for raw arrays (char[N], const char[N], etc.).
    // Arrays decay to pointers silently, which would silently include the null
    // terminator in the transcoded input.  The static_assert fires with a stable
    // message instead of a deep substitution failure.
    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const whatwg_decode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to whatwg_decode; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

template <codec C>
inline constexpr whatwg_decode_closure<C> whatwg_decode{};

// ---------------------------------------------------------------------------
// whatwg_decode_or_error_view — decodes bytes to expected<char32_t, whatwg_error>
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
class whatwg_decode_or_error_view : public std::ranges::view_interface<whatwg_decode_or_error_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;
        using result_t  = std::expected<char32_t, whatwg_error>;

        base_iter current_;
        base_sent end_;
        result_t  value_{};
        bool      done_{false};

        constexpr void load();

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = result_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = result_t;

        constexpr iterator(base_iter current, base_sent end);

        constexpr result_t  operator*() const;
        constexpr iterator& operator++();
        constexpr void      operator++(int);

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_decode_or_error_view(R base);

    constexpr iterator                begin();
    constexpr std::default_sentinel_t end() const;
};

template <codec C>
struct whatwg_decode_or_error_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const whatwg_decode_or_error_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const whatwg_decode_or_error_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to whatwg_decode_or_error; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

template <codec C>
inline constexpr whatwg_decode_or_error_closure<C> whatwg_decode_or_error{};

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_view
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr whatwg_decode_view<C, R>::whatwg_decode_view(R base) : base_(std::move(base)) {}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr std::default_sentinel_t whatwg_decode_view<C, R>::end() const {
    return std::default_sentinel;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr void whatwg_decode_view<C, R>::iterator::load() {
    if (current_ == end_) {
        done_ = true;
        return;
    }
    if constexpr (C == codec::replacement) {
        while (current_ != end_)
            ++current_;
        value_ = U'\xFFFD';
    } else if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_decode_one(current_, end_);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    }
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr whatwg_decode_view<C, R>::iterator::iterator(base_iter current, base_sent end)
    : current_(std::move(current)), end_(std::move(end)) {
    load();
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr char32_t whatwg_decode_view<C, R>::iterator::operator*() const {
    return value_;
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr void whatwg_decode_view<C, R>::iterator::operator++(int) {
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_closure
// ---------------------------------------------------------------------------

template <codec C>
template <legacy_byte_range R>
constexpr auto whatwg_decode_closure<C>::operator()(R&& r) const {
    return whatwg_decode_view<C, std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_or_error_view
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr whatwg_decode_or_error_view<C, R>::whatwg_decode_or_error_view(R base) : base_(std::move(base)) {}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_or_error_view<C, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr std::default_sentinel_t whatwg_decode_or_error_view<C, R>::end() const {
    return std::default_sentinel;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_or_error_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr void whatwg_decode_or_error_view<C, R>::iterator::load() {
    if (current_ == end_) {
        done_ = true;
        return;
    }
    if constexpr (C == codec::replacement) {
        while (current_ != end_)
            ++current_;
        value_ = std::unexpected(whatwg_error::invalid_byte);
    } else if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_decode_one(current_, end_);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    }
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr whatwg_decode_or_error_view<C, R>::iterator::iterator(base_iter current, base_sent end)
    : current_(std::move(current)), end_(std::move(end)) {
    load();
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_or_error_view<C, R>::iterator::operator*() const -> result_t {
    return value_;
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_or_error_view<C, R>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr void whatwg_decode_or_error_view<C, R>::iterator::operator++(int) {
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_or_error_closure
// ---------------------------------------------------------------------------

template <codec C>
template <legacy_byte_range R>
constexpr auto whatwg_decode_or_error_closure<C>::operator()(R&& r) const {
    return whatwg_decode_or_error_view<C, std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
