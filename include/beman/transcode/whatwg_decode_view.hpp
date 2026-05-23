// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
#define BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/utf8.hpp>

#include <expected>
#include <iterator>
#include <ranges>

namespace beman::transcoding {

enum class codec { utf_8 };

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

        constexpr void load() {
            if (current_ == end_) {
                done_ = true;
                return;
            }
            auto r = detail::utf8_decode_one(current_, end_);
            value_ = r.is_error ? U'�' : r.code_point;
        }

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = char32_t;

        constexpr iterator(base_iter current, base_sent end) : current_(std::move(current)), end_(std::move(end)) {
            load();
        }

        constexpr char32_t operator*() const { return value_; }

        constexpr iterator& operator++() {
            load();
            return *this;
        }

        constexpr void operator++(int) { ++*this; }

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_decode_view(R base) : base_(std::move(base)) {}

    constexpr iterator begin() { return iterator(std::ranges::begin(base_), std::ranges::end(base_)); }

    constexpr std::default_sentinel_t end() const { return std::default_sentinel; }
};

template <codec C>
struct whatwg_decode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const {
        return whatwg_decode_view<C, std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
    }

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

        constexpr void load() {
            if (current_ == end_) {
                done_ = true;
                return;
            }
            auto r = detail::utf8_decode_one(current_, end_);
            if (r.is_error)
                value_ = std::unexpected(r.error);
            else
                value_ = r.code_point;
        }

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = result_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = result_t;

        constexpr iterator(base_iter current, base_sent end) : current_(std::move(current)), end_(std::move(end)) {
            load();
        }

        constexpr result_t operator*() const { return value_; }

        constexpr iterator& operator++() {
            load();
            return *this;
        }

        constexpr void operator++(int) { ++*this; }

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_decode_or_error_view(R base) : base_(std::move(base)) {}

    constexpr iterator begin() { return iterator(std::ranges::begin(base_), std::ranges::end(base_)); }

    constexpr std::default_sentinel_t end() const { return std::default_sentinel; }
};

template <codec C>
struct whatwg_decode_or_error_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const {
        return whatwg_decode_or_error_view<C, std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
    }

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

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
