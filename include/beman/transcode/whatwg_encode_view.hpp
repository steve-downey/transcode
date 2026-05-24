// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/single_byte.hpp>
#include <beman/transcode/detail/tables/ibm866.hpp>
#include <beman/transcode/detail/tables/iso_8859_10.hpp>
#include <beman/transcode/detail/tables/iso_8859_13.hpp>
#include <beman/transcode/detail/tables/iso_8859_14.hpp>
#include <beman/transcode/detail/tables/iso_8859_15.hpp>
#include <beman/transcode/detail/tables/iso_8859_16.hpp>
#include <beman/transcode/detail/tables/iso_8859_2.hpp>
#include <beman/transcode/detail/tables/iso_8859_3.hpp>
#include <beman/transcode/detail/tables/iso_8859_4.hpp>
#include <beman/transcode/detail/tables/iso_8859_5.hpp>
#include <beman/transcode/detail/tables/iso_8859_6.hpp>
#include <beman/transcode/detail/tables/iso_8859_7.hpp>
#include <beman/transcode/detail/tables/iso_8859_8.hpp>
#include <beman/transcode/detail/tables/koi8_r.hpp>
#include <beman/transcode/detail/tables/koi8_u.hpp>
#include <beman/transcode/detail/tables/macintosh.hpp>
#include <beman/transcode/detail/tables/windows_1250.hpp>
#include <beman/transcode/detail/tables/windows_1251.hpp>
#include <beman/transcode/detail/tables/windows_1252.hpp>
#include <beman/transcode/detail/tables/windows_1253.hpp>
#include <beman/transcode/detail/tables/windows_1254.hpp>
#include <beman/transcode/detail/tables/windows_1255.hpp>
#include <beman/transcode/detail/tables/windows_1256.hpp>
#include <beman/transcode/detail/tables/windows_1257.hpp>
#include <beman/transcode/detail/tables/windows_1258.hpp>
#include <beman/transcode/detail/tables/windows_874.hpp>
#include <beman/transcode/detail/tables/x_mac_cyrillic.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>

#include <expected>
#include <iterator>
#include <ranges>

namespace beman::transcoding {

// ---------------------------------------------------------------------------
// whatwg_encode_view — encodes char32_t codepoints to bytes, replacing
// unmapped codepoints with '?' (0x3F).
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
class whatwg_encode_view : public std::ranges::view_interface<whatwg_encode_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;

        base_iter current_;
        base_sent end_;
        char      value_{};
        bool      done_{false};

        constexpr void load();

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char;
        using difference_type  = std::ptrdiff_t;
        using reference        = char;

        constexpr iterator(base_iter current, base_sent end);

        constexpr char      operator*() const;
        constexpr iterator& operator++();
        constexpr void      operator++(int);

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_encode_view(R base);

    constexpr iterator                begin();
    constexpr std::default_sentinel_t end() const;
};

template <codec C>
struct whatwg_encode_closure {
    template <unicode_scalar_range R>
    constexpr auto operator()(R&& r) const;

    template <unicode_scalar_range R>
    constexpr friend auto operator|(R&& r, const whatwg_encode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const whatwg_encode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to whatwg_encode; "
                      "wrap with std::span or std::vector");
    }
};

template <codec C>
inline constexpr whatwg_encode_closure<C> whatwg_encode{};

// ---------------------------------------------------------------------------
// whatwg_encode_or_error_view — encodes char32_t codepoints to
// expected<char, whatwg_error>, yielding unmapped_codepoint on failure.
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
class whatwg_encode_or_error_view
    : public std::ranges::view_interface<whatwg_encode_or_error_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = std::ranges::iterator_t<R>;
        using base_sent = std::ranges::sentinel_t<R>;
        using result_t  = std::expected<char, whatwg_error>;

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
    constexpr explicit whatwg_encode_or_error_view(R base);

    constexpr iterator                begin();
    constexpr std::default_sentinel_t end() const;
};

template <codec C>
struct whatwg_encode_or_error_closure {
    template <unicode_scalar_range R>
    constexpr auto operator()(R&& r) const;

    template <unicode_scalar_range R>
    constexpr friend auto operator|(R&& r, const whatwg_encode_or_error_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const whatwg_encode_or_error_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to whatwg_encode_or_error; "
                      "wrap with std::span or std::vector");
    }
};

template <codec C>
inline constexpr whatwg_encode_or_error_closure<C> whatwg_encode_or_error{};

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_view
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr whatwg_encode_view<C, R>::whatwg_encode_view(R base) : base_(std::move(base)) {}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_view<C, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr std::default_sentinel_t whatwg_encode_view<C, R>::end() const {
    return std::default_sentinel;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_view<C, R>::iterator::load() {
    if (current_ == end_) {
        done_ = true;
        return;
    }
    auto encode = [&](const char32_t (&table)[128]) {
        auto r   = detail::single_byte_encode_one(current_, end_, table);
        value_   = r.is_error ? '?' : static_cast<char>(r.byte);
    };
    if constexpr (C == codec::ibm866) {
        encode(detail::tables::ibm866);
    } else if constexpr (C == codec::iso_8859_2) {
        encode(detail::tables::iso_8859_2);
    } else if constexpr (C == codec::iso_8859_3) {
        encode(detail::tables::iso_8859_3);
    } else if constexpr (C == codec::iso_8859_4) {
        encode(detail::tables::iso_8859_4);
    } else if constexpr (C == codec::iso_8859_5) {
        encode(detail::tables::iso_8859_5);
    } else if constexpr (C == codec::iso_8859_6) {
        encode(detail::tables::iso_8859_6);
    } else if constexpr (C == codec::iso_8859_7) {
        encode(detail::tables::iso_8859_7);
    } else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i) {
        encode(detail::tables::iso_8859_8);
    } else if constexpr (C == codec::iso_8859_10) {
        encode(detail::tables::iso_8859_10);
    } else if constexpr (C == codec::iso_8859_13) {
        encode(detail::tables::iso_8859_13);
    } else if constexpr (C == codec::iso_8859_14) {
        encode(detail::tables::iso_8859_14);
    } else if constexpr (C == codec::iso_8859_15) {
        encode(detail::tables::iso_8859_15);
    } else if constexpr (C == codec::iso_8859_16) {
        encode(detail::tables::iso_8859_16);
    } else if constexpr (C == codec::koi8_r) {
        encode(detail::tables::koi8_r);
    } else if constexpr (C == codec::koi8_u) {
        encode(detail::tables::koi8_u);
    } else if constexpr (C == codec::macintosh) {
        encode(detail::tables::macintosh);
    } else if constexpr (C == codec::windows_874) {
        encode(detail::tables::windows_874);
    } else if constexpr (C == codec::windows_1250) {
        encode(detail::tables::windows_1250);
    } else if constexpr (C == codec::windows_1251) {
        encode(detail::tables::windows_1251);
    } else if constexpr (C == codec::windows_1252) {
        encode(detail::tables::windows_1252);
    } else if constexpr (C == codec::windows_1253) {
        encode(detail::tables::windows_1253);
    } else if constexpr (C == codec::windows_1254) {
        encode(detail::tables::windows_1254);
    } else if constexpr (C == codec::windows_1255) {
        encode(detail::tables::windows_1255);
    } else if constexpr (C == codec::windows_1256) {
        encode(detail::tables::windows_1256);
    } else if constexpr (C == codec::windows_1257) {
        encode(detail::tables::windows_1257);
    } else if constexpr (C == codec::windows_1258) {
        encode(detail::tables::windows_1258);
    } else if constexpr (C == codec::x_mac_cyrillic) {
        encode(detail::tables::x_mac_cyrillic);
    }
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr whatwg_encode_view<C, R>::iterator::iterator(base_iter current, base_sent end)
    : current_(std::move(current)), end_(std::move(end)) {
    load();
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr char whatwg_encode_view<C, R>::iterator::operator*() const {
    return value_;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_view<C, R>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_view<C, R>::iterator::operator++(int) {
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_closure
// ---------------------------------------------------------------------------

template <codec C>
template <unicode_scalar_range R>
constexpr auto whatwg_encode_closure<C>::operator()(R&& r) const {
    return whatwg_encode_view<C, std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_or_error_view
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr whatwg_encode_or_error_view<C, R>::whatwg_encode_or_error_view(R base)
    : base_(std::move(base)) {}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr std::default_sentinel_t whatwg_encode_or_error_view<C, R>::end() const {
    return std::default_sentinel;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_or_error_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_or_error_view<C, R>::iterator::load() {
    if (current_ == end_) {
        done_ = true;
        return;
    }
    auto encode = [&](const char32_t (&table)[128]) {
        auto r = detail::single_byte_encode_one(current_, end_, table);
        if (r.is_error)
            value_ = std::unexpected(whatwg_error::unmapped_codepoint);
        else
            value_ = static_cast<char>(r.byte);
    };
    if constexpr (C == codec::ibm866) {
        encode(detail::tables::ibm866);
    } else if constexpr (C == codec::iso_8859_2) {
        encode(detail::tables::iso_8859_2);
    } else if constexpr (C == codec::iso_8859_3) {
        encode(detail::tables::iso_8859_3);
    } else if constexpr (C == codec::iso_8859_4) {
        encode(detail::tables::iso_8859_4);
    } else if constexpr (C == codec::iso_8859_5) {
        encode(detail::tables::iso_8859_5);
    } else if constexpr (C == codec::iso_8859_6) {
        encode(detail::tables::iso_8859_6);
    } else if constexpr (C == codec::iso_8859_7) {
        encode(detail::tables::iso_8859_7);
    } else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i) {
        encode(detail::tables::iso_8859_8);
    } else if constexpr (C == codec::iso_8859_10) {
        encode(detail::tables::iso_8859_10);
    } else if constexpr (C == codec::iso_8859_13) {
        encode(detail::tables::iso_8859_13);
    } else if constexpr (C == codec::iso_8859_14) {
        encode(detail::tables::iso_8859_14);
    } else if constexpr (C == codec::iso_8859_15) {
        encode(detail::tables::iso_8859_15);
    } else if constexpr (C == codec::iso_8859_16) {
        encode(detail::tables::iso_8859_16);
    } else if constexpr (C == codec::koi8_r) {
        encode(detail::tables::koi8_r);
    } else if constexpr (C == codec::koi8_u) {
        encode(detail::tables::koi8_u);
    } else if constexpr (C == codec::macintosh) {
        encode(detail::tables::macintosh);
    } else if constexpr (C == codec::windows_874) {
        encode(detail::tables::windows_874);
    } else if constexpr (C == codec::windows_1250) {
        encode(detail::tables::windows_1250);
    } else if constexpr (C == codec::windows_1251) {
        encode(detail::tables::windows_1251);
    } else if constexpr (C == codec::windows_1252) {
        encode(detail::tables::windows_1252);
    } else if constexpr (C == codec::windows_1253) {
        encode(detail::tables::windows_1253);
    } else if constexpr (C == codec::windows_1254) {
        encode(detail::tables::windows_1254);
    } else if constexpr (C == codec::windows_1255) {
        encode(detail::tables::windows_1255);
    } else if constexpr (C == codec::windows_1256) {
        encode(detail::tables::windows_1256);
    } else if constexpr (C == codec::windows_1257) {
        encode(detail::tables::windows_1257);
    } else if constexpr (C == codec::windows_1258) {
        encode(detail::tables::windows_1258);
    } else if constexpr (C == codec::x_mac_cyrillic) {
        encode(detail::tables::x_mac_cyrillic);
    }
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr whatwg_encode_or_error_view<C, R>::iterator::iterator(base_iter current, base_sent end)
    : current_(std::move(current)), end_(std::move(end)) {
    load();
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::iterator::operator*() const -> result_t {
    return value_;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_or_error_view<C, R>::iterator::operator++(int) {
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_or_error_closure
// ---------------------------------------------------------------------------

template <codec C>
template <unicode_scalar_range R>
constexpr auto whatwg_encode_or_error_closure<C>::operator()(R&& r) const {
    return whatwg_encode_or_error_view<C, std::views::all_t<R>>(
        std::views::all(std::forward<R>(r)));
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP
