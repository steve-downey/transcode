// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/big5.hpp>
#include <beman/transcode/detail/euc_jp.hpp>
#include <beman/transcode/detail/euc_kr.hpp>
#include <beman/transcode/detail/gb18030.hpp>
#include <beman/transcode/detail/gbk.hpp>
#include <beman/transcode/detail/shift_jis.hpp>
#include <beman/transcode/detail/single_byte.hpp>
#include <beman/transcode/detail/utf8_encode.hpp>
#include <beman/transcode/detail/utf16.hpp>
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
#include <compare>
#include <algorithm>
#include <iterator>
#include <ranges>

namespace beman::transcoding {

namespace detail {

template <codec C>
concept random_access_encode_codec =
    C == codec::ibm866 || C == codec::iso_8859_2 || C == codec::iso_8859_3 || C == codec::iso_8859_4 ||
    C == codec::iso_8859_5 || C == codec::iso_8859_6 || C == codec::iso_8859_7 || C == codec::iso_8859_8 ||
    C == codec::iso_8859_8_i || C == codec::iso_8859_10 || C == codec::iso_8859_13 || C == codec::iso_8859_14 ||
    C == codec::iso_8859_15 || C == codec::iso_8859_16 || C == codec::koi8_r || C == codec::koi8_u ||
    C == codec::macintosh || C == codec::windows_874 || C == codec::windows_1250 || C == codec::windows_1251 ||
    C == codec::windows_1252 || C == codec::windows_1253 || C == codec::windows_1254 || C == codec::windows_1255 ||
    C == codec::windows_1256 || C == codec::windows_1257 || C == codec::windows_1258 || C == codec::x_mac_cyrillic;

template <codec C>
consteval const char32_t (&random_access_encode_table())[128] {
    if constexpr (C == codec::ibm866)
        return tables::ibm866;
    else if constexpr (C == codec::iso_8859_2)
        return tables::iso_8859_2;
    else if constexpr (C == codec::iso_8859_3)
        return tables::iso_8859_3;
    else if constexpr (C == codec::iso_8859_4)
        return tables::iso_8859_4;
    else if constexpr (C == codec::iso_8859_5)
        return tables::iso_8859_5;
    else if constexpr (C == codec::iso_8859_6)
        return tables::iso_8859_6;
    else if constexpr (C == codec::iso_8859_7)
        return tables::iso_8859_7;
    else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i)
        return tables::iso_8859_8;
    else if constexpr (C == codec::iso_8859_10)
        return tables::iso_8859_10;
    else if constexpr (C == codec::iso_8859_13)
        return tables::iso_8859_13;
    else if constexpr (C == codec::iso_8859_14)
        return tables::iso_8859_14;
    else if constexpr (C == codec::iso_8859_15)
        return tables::iso_8859_15;
    else if constexpr (C == codec::iso_8859_16)
        return tables::iso_8859_16;
    else if constexpr (C == codec::koi8_r)
        return tables::koi8_r;
    else if constexpr (C == codec::koi8_u)
        return tables::koi8_u;
    else if constexpr (C == codec::macintosh)
        return tables::macintosh;
    else if constexpr (C == codec::windows_874)
        return tables::windows_874;
    else if constexpr (C == codec::windows_1250)
        return tables::windows_1250;
    else if constexpr (C == codec::windows_1251)
        return tables::windows_1251;
    else if constexpr (C == codec::windows_1252)
        return tables::windows_1252;
    else if constexpr (C == codec::windows_1253)
        return tables::windows_1253;
    else if constexpr (C == codec::windows_1254)
        return tables::windows_1254;
    else if constexpr (C == codec::windows_1255)
        return tables::windows_1255;
    else if constexpr (C == codec::windows_1256)
        return tables::windows_1256;
    else if constexpr (C == codec::windows_1257)
        return tables::windows_1257;
    else if constexpr (C == codec::windows_1258)
        return tables::windows_1258;
    else
        return tables::x_mac_cyrillic;
}

constexpr char encode_random_access_code_point(char32_t cp, const char32_t (&table)[128]) {
    if (cp < 0x80)
        return static_cast<char>(cp);
    for (int index = 0; index < 128; ++index) {
        if (table[index] == cp)
            return static_cast<char>(0x80 + index);
    }
    return '?';
}

} // namespace detail

template <codec C, std::ranges::random_access_range R>
    requires unicode_scalar_range<R> && detail::random_access_encode_codec<C>
class random_access_whatwg_encode_view : public std::ranges::view_interface<random_access_whatwg_encode_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;

        base_iter current_{};

      public:
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using reference         = char;

        constexpr iterator() = default;
        constexpr explicit iterator(base_iter current) : current_(current) {}

        constexpr const base_iter& base() const noexcept { return current_; }

        constexpr char operator*() const {
            return detail::encode_random_access_code_point(static_cast<char32_t>(*current_),
                                                           detail::random_access_encode_table<C>());
        }

        constexpr char operator[](difference_type n) const {
            return detail::encode_random_access_code_point(static_cast<char32_t>(current_[n]),
                                                           detail::random_access_encode_table<C>());
        }

        constexpr iterator& operator++() {
            ++current_;
            return *this;
        }

        constexpr iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        constexpr iterator& operator--() {
            --current_;
            return *this;
        }

        constexpr iterator operator--(int) {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        constexpr iterator& operator+=(difference_type n) {
            current_ += n;
            return *this;
        }

        constexpr iterator& operator-=(difference_type n) {
            current_ -= n;
            return *this;
        }

        constexpr iterator operator+(difference_type n) const {
            auto tmp = *this;
            return tmp += n;
        }

        constexpr iterator operator-(difference_type n) const {
            auto tmp = *this;
            return tmp -= n;
        }

        constexpr difference_type operator-(const iterator& other) const { return current_ - other.current_; }

        constexpr friend iterator operator+(difference_type n, iterator it) { return it += n; }

        constexpr auto operator<=>(const iterator&) const = default;
    };

  public:
    constexpr explicit random_access_whatwg_encode_view(R base) : base_(std::move(base)) {}

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin() { return iterator(std::ranges::begin(base_)); }

    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R>
    {
        return iterator(std::ranges::begin(base_));
    }

    constexpr iterator end() { return iterator(std::ranges::end(base_)); }

    constexpr iterator end() const
        requires detail::const_iterator_compatible_range<R> && std::ranges::range<const R>
    {
        return iterator(std::ranges::end(base_));
    }

    constexpr auto size() const
        requires std::ranges::sized_range<R>
    {
        return std::ranges::size(base_);
    }
};

template <codec C, std::ranges::random_access_range R>
    requires unicode_scalar_range<R> && detail::random_access_encode_codec<C>
class random_access_whatwg_encode_or_error_view
    : public std::ranges::view_interface<random_access_whatwg_encode_or_error_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;

        base_iter current_{};

      public:
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = std::expected<char, whatwg_error>;
        using difference_type   = std::ptrdiff_t;
        using reference         = value_type;

        constexpr iterator() = default;
        constexpr explicit iterator(base_iter current) : current_(current) {}

        constexpr const base_iter& base() const noexcept { return current_; }

        constexpr value_type operator*() const {
            const auto cp = static_cast<char32_t>(*current_);
            if (cp < 0x80)
                return static_cast<char>(cp);
            const auto& table = detail::random_access_encode_table<C>();
            for (int index = 0; index < 128; ++index) {
                if (table[index] == cp)
                    return static_cast<char>(0x80 + index);
            }
            return std::unexpected(whatwg_error::unmapped_codepoint);
        }

        constexpr value_type operator[](difference_type n) const { return *(*this + n); }

        constexpr iterator& operator++() {
            ++current_;
            return *this;
        }

        constexpr iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        constexpr iterator& operator--() {
            --current_;
            return *this;
        }

        constexpr iterator operator--(int) {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        constexpr iterator& operator+=(difference_type n) {
            current_ += n;
            return *this;
        }

        constexpr iterator& operator-=(difference_type n) {
            current_ -= n;
            return *this;
        }

        constexpr iterator operator+(difference_type n) const {
            auto tmp = *this;
            return tmp += n;
        }

        constexpr iterator operator-(difference_type n) const {
            auto tmp = *this;
            return tmp -= n;
        }

        constexpr difference_type operator-(const iterator& other) const { return current_ - other.current_; }

        constexpr friend iterator operator+(difference_type n, iterator it) { return it += n; }

        constexpr auto operator<=>(const iterator&) const = default;
    };

  public:
    constexpr explicit random_access_whatwg_encode_or_error_view(R base) : base_(std::move(base)) {}

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin() { return iterator(std::ranges::begin(base_)); }

    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R>
    {
        return iterator(std::ranges::begin(base_));
    }

    constexpr iterator end() { return iterator(std::ranges::end(base_)); }

    constexpr iterator end() const
        requires detail::const_iterator_compatible_range<R> && std::ranges::range<const R>
    {
        return iterator(std::ranges::end(base_));
    }

    constexpr auto size() const
        requires std::ranges::sized_range<R>
    {
        return std::ranges::size(base_);
    }
};

// ---------------------------------------------------------------------------
// whatwg_encode_view — encodes char32_t codepoints to bytes, replacing
// unmapped codepoints with '?' (0x3F).
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
class whatwg_encode_view : public std::ranges::view_interface<whatwg_encode_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;

        base_iter current_{};
        base_sent end_{};
        char      buf_[8]{};
        int       len_{0};
        int       pos_{0};
        bool      done_{false};
        int       iso2022jp_state_{0};

        constexpr void load();

      public:
        constexpr iterator() = default;

        static constexpr iterator terminal()
            requires std::ranges::forward_range<R>
        {
            iterator it;
            it.done_ = true;
            return it;
        }

        using iterator_concept =
            std::conditional_t<std::ranges::forward_range<R>, std::forward_iterator_tag, std::input_iterator_tag>;
        using iterator_category = iterator_concept;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using reference         = char;

        constexpr iterator(base_iter current, base_sent end);

        constexpr const base_iter& base() const noexcept { return current_; }

        constexpr char      operator*() const;
        constexpr iterator& operator++();
        constexpr iterator  operator++(int)
            requires std::ranges::forward_range<R>;
        constexpr void operator++(int)
            requires(!std::ranges::forward_range<R>);

        constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            requires std::ranges::forward_range<R>
        {
            if (lhs.done_ || rhs.done_)
                return lhs.done_ == rhs.done_;
            return lhs.current_ == rhs.current_ && lhs.len_ == rhs.len_ && lhs.pos_ == rhs.pos_ &&
                   lhs.done_ == rhs.done_ && lhs.iso2022jp_state_ == rhs.iso2022jp_state_ &&
                   std::equal(std::begin(lhs.buf_), std::end(lhs.buf_), std::begin(rhs.buf_));
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_encode_view(R base);

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin();
    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
    constexpr iterator end()
        requires std::ranges::forward_range<R> && std::ranges::common_range<R>;
    constexpr iterator end() const
        requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
                 detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
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
class whatwg_encode_or_error_view : public std::ranges::view_interface<whatwg_encode_or_error_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;
        using result_t  = std::expected<char, whatwg_error>;

        base_iter current_{};
        base_sent end_{};
        result_t  buf_[8]{};
        int       len_{0};
        int       pos_{0};
        bool      done_{false};
        int       iso2022jp_state_{0};

        constexpr void load();

      public:
        constexpr iterator() = default;

        static constexpr iterator terminal()
            requires std::ranges::forward_range<R>
        {
            iterator it;
            it.done_ = true;
            return it;
        }

        using iterator_concept =
            std::conditional_t<std::ranges::forward_range<R>, std::forward_iterator_tag, std::input_iterator_tag>;
        using iterator_category = iterator_concept;
        using value_type        = result_t;
        using difference_type   = std::ptrdiff_t;
        using reference         = result_t;

        constexpr iterator(base_iter current, base_sent end);

        constexpr const base_iter& base() const noexcept { return current_; }

        constexpr auto      operator*() const;
        constexpr iterator& operator++();
        constexpr iterator  operator++(int)
            requires std::ranges::forward_range<R>;
        constexpr void operator++(int)
            requires(!std::ranges::forward_range<R>);

        constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            requires std::ranges::forward_range<R>
        {
            if (lhs.done_ || rhs.done_)
                return lhs.done_ == rhs.done_;
            return lhs.current_ == rhs.current_ && lhs.len_ == rhs.len_ && lhs.pos_ == rhs.pos_ &&
                   lhs.done_ == rhs.done_ && lhs.iso2022jp_state_ == rhs.iso2022jp_state_ &&
                   std::equal(std::begin(lhs.buf_), std::end(lhs.buf_), std::begin(rhs.buf_));
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_encode_or_error_view(R base);

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin();
    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
    constexpr iterator end()
        requires std::ranges::forward_range<R> && std::ranges::common_range<R>;
    constexpr iterator end() const
        requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
                 detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
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

} // namespace beman::transcoding

namespace std::ranges {

template <beman::transcoding::codec C, random_access_range R>
    requires beman::transcoding::unicode_scalar_range<R> && beman::transcoding::detail::random_access_encode_codec<C>
inline constexpr bool enable_borrowed_range<beman::transcoding::random_access_whatwg_encode_view<C, R>> =
    borrowed_range<R>;

template <beman::transcoding::codec C, random_access_range R>
    requires beman::transcoding::unicode_scalar_range<R> && beman::transcoding::detail::random_access_encode_codec<C>
inline constexpr bool enable_borrowed_range<beman::transcoding::random_access_whatwg_encode_or_error_view<C, R>> =
    borrowed_range<R>;

template <beman::transcoding::codec C, input_range R>
    requires beman::transcoding::unicode_scalar_range<R>
inline constexpr bool enable_borrowed_range<beman::transcoding::whatwg_encode_view<C, R>> = borrowed_range<R>;

template <beman::transcoding::codec C, input_range R>
    requires beman::transcoding::unicode_scalar_range<R>
inline constexpr bool enable_borrowed_range<beman::transcoding::whatwg_encode_or_error_view<C, R>> = borrowed_range<R>;

} // namespace std::ranges

namespace beman::transcoding {

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
constexpr auto whatwg_encode_view<C, R>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_view<C, R>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr std::default_sentinel_t whatwg_encode_view<C, R>::end() const {
    return std::default_sentinel;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_view<C, R>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_view<C, R>::iterator::load() {
    if (current_ == end_) {
        if constexpr (C == codec::iso_2022_jp) {
            if (iso2022jp_state_ != 0) {
                buf_[0]          = '\x1B';
                buf_[1]          = '\x28';
                buf_[2]          = '\x42';
                len_             = 3;
                pos_             = 0;
                iso2022jp_state_ = 0;
                return;
            }
        }
        done_ = true;
        return;
    }
    auto encode_single = [&](const char32_t (&table)[128]) {
        auto r  = detail::single_byte_encode_one(current_, end_, table);
        buf_[0] = r.is_error ? '?' : static_cast<char>(r.byte);
        len_    = 1;
        pos_    = 0;
    };
    if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            buf_[0] = '\xEF';
            buf_[1] = '\xBF';
            buf_[2] = '\xBD';
            len_    = 3;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = r.bytes[i];
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::ibm866) {
        encode_single(detail::tables::ibm866);
    } else if constexpr (C == codec::iso_8859_2) {
        encode_single(detail::tables::iso_8859_2);
    } else if constexpr (C == codec::iso_8859_3) {
        encode_single(detail::tables::iso_8859_3);
    } else if constexpr (C == codec::iso_8859_4) {
        encode_single(detail::tables::iso_8859_4);
    } else if constexpr (C == codec::iso_8859_5) {
        encode_single(detail::tables::iso_8859_5);
    } else if constexpr (C == codec::iso_8859_6) {
        encode_single(detail::tables::iso_8859_6);
    } else if constexpr (C == codec::iso_8859_7) {
        encode_single(detail::tables::iso_8859_7);
    } else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i) {
        encode_single(detail::tables::iso_8859_8);
    } else if constexpr (C == codec::iso_8859_10) {
        encode_single(detail::tables::iso_8859_10);
    } else if constexpr (C == codec::iso_8859_13) {
        encode_single(detail::tables::iso_8859_13);
    } else if constexpr (C == codec::iso_8859_14) {
        encode_single(detail::tables::iso_8859_14);
    } else if constexpr (C == codec::iso_8859_15) {
        encode_single(detail::tables::iso_8859_15);
    } else if constexpr (C == codec::iso_8859_16) {
        encode_single(detail::tables::iso_8859_16);
    } else if constexpr (C == codec::koi8_r) {
        encode_single(detail::tables::koi8_r);
    } else if constexpr (C == codec::koi8_u) {
        encode_single(detail::tables::koi8_u);
    } else if constexpr (C == codec::macintosh) {
        encode_single(detail::tables::macintosh);
    } else if constexpr (C == codec::windows_874) {
        encode_single(detail::tables::windows_874);
    } else if constexpr (C == codec::windows_1250) {
        encode_single(detail::tables::windows_1250);
    } else if constexpr (C == codec::windows_1251) {
        encode_single(detail::tables::windows_1251);
    } else if constexpr (C == codec::windows_1252) {
        encode_single(detail::tables::windows_1252);
    } else if constexpr (C == codec::windows_1253) {
        encode_single(detail::tables::windows_1253);
    } else if constexpr (C == codec::windows_1254) {
        encode_single(detail::tables::windows_1254);
    } else if constexpr (C == codec::windows_1255) {
        encode_single(detail::tables::windows_1255);
    } else if constexpr (C == codec::windows_1256) {
        encode_single(detail::tables::windows_1256);
    } else if constexpr (C == codec::windows_1257) {
        encode_single(detail::tables::windows_1257);
    } else if constexpr (C == codec::windows_1258) {
        encode_single(detail::tables::windows_1258);
    } else if constexpr (C == codec::x_mac_cyrillic) {
        encode_single(detail::tables::x_mac_cyrillic);
    } else if constexpr (C == codec::utf_16be) {
        auto r = detail::utf16be_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            // U+FFFD in UTF-16BE: 0xFF 0xFD
            buf_[0] = '\xFF';
            buf_[1] = '\xFD';
            len_    = 2;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = r.bytes[i];
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::utf_16le) {
        auto r = detail::utf16le_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            // U+FFFD in UTF-16LE: 0xFD 0xFF
            buf_[0] = '\xFD';
            buf_[1] = '\xFF';
            len_    = 2;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = r.bytes[i];
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::gbk) {
        auto r = detail::gbk_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            buf_[0] = '?';
            len_    = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = static_cast<char>(r.bytes[i]);
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::gb18030) {
        auto r = detail::gb18030_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        for (int i = 0; i < r.count; ++i)
            buf_[i] = static_cast<char>(r.bytes[i]);
        len_ = r.count;
        pos_ = 0;
    } else if constexpr (C == codec::big5) {
        auto r = detail::big5_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            buf_[0] = '?';
            len_    = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = static_cast<char>(r.bytes[i]);
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::shift_jis) {
        auto r = detail::shift_jis_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            buf_[0] = '?';
            len_    = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = static_cast<char>(r.bytes[i]);
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::euc_jp) {
        auto r = detail::euc_jp_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            buf_[0] = '?';
            len_    = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = static_cast<char>(r.bytes[i]);
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::iso_2022_jp) {
        auto cp = static_cast<char32_t>(*current_);
        ++current_;
        // Roman state: U+00A5 (YEN SIGN) and U+203E (OVERLINE)
        if (cp == 0x00A5 || cp == 0x203E) {
            char ascii_byte = (cp == 0x00A5) ? '\x5C' : '\x7E';
            if (iso2022jp_state_ != 1) {
                buf_[0]          = '\x1B';
                buf_[1]          = '\x28';
                buf_[2]          = '\x4A';
                buf_[3]          = ascii_byte;
                len_             = 4;
                iso2022jp_state_ = 1;
            } else {
                buf_[0] = ascii_byte;
                len_    = 1;
            }
            pos_ = 0;
            return;
        }
        if (cp < 0x80) {
            char ascii_byte = static_cast<char>(cp);
            if (iso2022jp_state_ != 0) {
                buf_[0]          = '\x1B';
                buf_[1]          = '\x28';
                buf_[2]          = '\x42';
                buf_[3]          = ascii_byte;
                len_             = 4;
                iso2022jp_state_ = 0;
            } else {
                buf_[0] = ascii_byte;
                len_    = 1;
            }
            pos_ = 0;
            return;
        }
        for (int i = 0; i < 8836; ++i) {
            if (detail::tables::shift_jis[i] == cp) {
                int lead  = (i / 94) + 0x21;
                int trail = (i % 94) + 0x21;
                if (iso2022jp_state_ != 2) {
                    buf_[0]          = '\x1B';
                    buf_[1]          = '\x24';
                    buf_[2]          = '\x42';
                    buf_[3]          = static_cast<char>(lead);
                    buf_[4]          = static_cast<char>(trail);
                    len_             = 5;
                    iso2022jp_state_ = 2;
                } else {
                    buf_[0] = static_cast<char>(lead);
                    buf_[1] = static_cast<char>(trail);
                    len_    = 2;
                }
                pos_ = 0;
                return;
            }
        }
        // Unmapped
        if (iso2022jp_state_ != 0) {
            buf_[0]          = '\x1B';
            buf_[1]          = '\x28';
            buf_[2]          = '\x42';
            buf_[3]          = '?';
            len_             = 4;
            iso2022jp_state_ = 0;
        } else {
            buf_[0] = '?';
            len_    = 1;
        }
        pos_ = 0;
    } else if constexpr (C == codec::euc_kr) {
        auto r = detail::euc_kr_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            buf_[0] = '?';
            len_    = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                buf_[i] = static_cast<char>(r.bytes[i]);
            len_ = r.count;
        }
        pos_ = 0;
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
    return buf_[pos_];
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_view<C, R>::iterator::operator++() -> iterator& {
    if (++pos_ < len_)
        return *this;
    load();
    return *this;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_view<C, R>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_view<C, R>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_closure
// ---------------------------------------------------------------------------

template <codec C>
template <unicode_scalar_range R>
constexpr auto whatwg_encode_closure<C>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    if constexpr (detail::random_access_encode_codec<C> && std::ranges::random_access_range<view_t>)
        return random_access_whatwg_encode_view<C, view_t>(std::move(all));
    else
        return whatwg_encode_view<C, view_t>(std::move(all));
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_or_error_view
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr whatwg_encode_or_error_view<C, R>::whatwg_encode_or_error_view(R base) : base_(std::move(base)) {}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr std::default_sentinel_t whatwg_encode_or_error_view<C, R>::end() const {
    return std::default_sentinel;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_or_error_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_or_error_view<C, R>::iterator::load() {
    using result_value = std::remove_cvref_t<decltype(this->buf_[0])>;

    if (current_ == end_) {
        if constexpr (C == codec::iso_2022_jp) {
            if (iso2022jp_state_ != 0) {
                this->buf_[0]    = result_value{'\x1B'};
                this->buf_[1]    = result_value{'\x28'};
                this->buf_[2]    = result_value{'\x42'};
                len_             = 3;
                pos_             = 0;
                iso2022jp_state_ = 0;
                return;
            }
        }
        done_ = true;
        return;
    }
    auto encode_single = [&](const char32_t (&table)[128]) {
        auto r = detail::single_byte_encode_one(current_, end_, table);
        if (r.is_error)
            this->buf_[0] = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
        else
            this->buf_[0] = result_value{static_cast<char>(r.byte)};
        len_ = 1;
        pos_ = 0;
    };
    if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, r.error};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::ibm866) {
        encode_single(detail::tables::ibm866);
    } else if constexpr (C == codec::iso_8859_2) {
        encode_single(detail::tables::iso_8859_2);
    } else if constexpr (C == codec::iso_8859_3) {
        encode_single(detail::tables::iso_8859_3);
    } else if constexpr (C == codec::iso_8859_4) {
        encode_single(detail::tables::iso_8859_4);
    } else if constexpr (C == codec::iso_8859_5) {
        encode_single(detail::tables::iso_8859_5);
    } else if constexpr (C == codec::iso_8859_6) {
        encode_single(detail::tables::iso_8859_6);
    } else if constexpr (C == codec::iso_8859_7) {
        encode_single(detail::tables::iso_8859_7);
    } else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i) {
        encode_single(detail::tables::iso_8859_8);
    } else if constexpr (C == codec::iso_8859_10) {
        encode_single(detail::tables::iso_8859_10);
    } else if constexpr (C == codec::iso_8859_13) {
        encode_single(detail::tables::iso_8859_13);
    } else if constexpr (C == codec::iso_8859_14) {
        encode_single(detail::tables::iso_8859_14);
    } else if constexpr (C == codec::iso_8859_15) {
        encode_single(detail::tables::iso_8859_15);
    } else if constexpr (C == codec::iso_8859_16) {
        encode_single(detail::tables::iso_8859_16);
    } else if constexpr (C == codec::koi8_r) {
        encode_single(detail::tables::koi8_r);
    } else if constexpr (C == codec::koi8_u) {
        encode_single(detail::tables::koi8_u);
    } else if constexpr (C == codec::macintosh) {
        encode_single(detail::tables::macintosh);
    } else if constexpr (C == codec::windows_874) {
        encode_single(detail::tables::windows_874);
    } else if constexpr (C == codec::windows_1250) {
        encode_single(detail::tables::windows_1250);
    } else if constexpr (C == codec::windows_1251) {
        encode_single(detail::tables::windows_1251);
    } else if constexpr (C == codec::windows_1252) {
        encode_single(detail::tables::windows_1252);
    } else if constexpr (C == codec::windows_1253) {
        encode_single(detail::tables::windows_1253);
    } else if constexpr (C == codec::windows_1254) {
        encode_single(detail::tables::windows_1254);
    } else if constexpr (C == codec::windows_1255) {
        encode_single(detail::tables::windows_1255);
    } else if constexpr (C == codec::windows_1256) {
        encode_single(detail::tables::windows_1256);
    } else if constexpr (C == codec::windows_1257) {
        encode_single(detail::tables::windows_1257);
    } else if constexpr (C == codec::windows_1258) {
        encode_single(detail::tables::windows_1258);
    } else if constexpr (C == codec::x_mac_cyrillic) {
        encode_single(detail::tables::x_mac_cyrillic);
    } else if constexpr (C == codec::utf_16be) {
        auto r = detail::utf16be_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::surrogate_code_point};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::utf_16le) {
        auto r = detail::utf16le_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::surrogate_code_point};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::gbk) {
        auto r = detail::gbk_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::gb18030) {
        auto r = detail::gb18030_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        for (int i = 0; i < r.count; ++i)
            this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
        len_ = r.count;
        pos_ = 0;
    } else if constexpr (C == codec::big5) {
        auto r = detail::big5_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::shift_jis) {
        auto r = detail::shift_jis_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::euc_jp) {
        auto r = detail::euc_jp_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
    } else if constexpr (C == codec::iso_2022_jp) {
        auto cp = static_cast<char32_t>(*current_);
        ++current_;
        // Roman state: U+00A5 (YEN SIGN) and U+203E (OVERLINE)
        if (cp == 0x00A5 || cp == 0x203E) {
            char ascii_byte = (cp == 0x00A5) ? '\x5C' : '\x7E';
            if (iso2022jp_state_ != 1) {
                this->buf_[0]    = result_value{'\x1B'};
                this->buf_[1]    = result_value{'\x28'};
                this->buf_[2]    = result_value{'\x4A'};
                this->buf_[3]    = result_value{ascii_byte};
                len_             = 4;
                iso2022jp_state_ = 1;
            } else {
                this->buf_[0] = result_value{ascii_byte};
                len_          = 1;
            }
            pos_ = 0;
            return;
        }
        if (cp < 0x80) {
            char ascii_byte = static_cast<char>(cp);
            if (iso2022jp_state_ != 0) {
                this->buf_[0]    = result_value{'\x1B'};
                this->buf_[1]    = result_value{'\x28'};
                this->buf_[2]    = result_value{'\x42'};
                this->buf_[3]    = result_value{ascii_byte};
                len_             = 4;
                iso2022jp_state_ = 0;
            } else {
                this->buf_[0] = result_value{ascii_byte};
                len_          = 1;
            }
            pos_ = 0;
            return;
        }
        for (int i = 0; i < 8836; ++i) {
            if (detail::tables::shift_jis[i] == cp) {
                int lead  = (i / 94) + 0x21;
                int trail = (i % 94) + 0x21;
                if (iso2022jp_state_ != 2) {
                    this->buf_[0]    = result_value{'\x1B'};
                    this->buf_[1]    = result_value{'\x24'};
                    this->buf_[2]    = result_value{'\x42'};
                    this->buf_[3]    = result_value{static_cast<char>(lead)};
                    this->buf_[4]    = result_value{static_cast<char>(trail)};
                    len_             = 5;
                    iso2022jp_state_ = 2;
                } else {
                    this->buf_[0] = result_value{static_cast<char>(lead)};
                    this->buf_[1] = result_value{static_cast<char>(trail)};
                    len_          = 2;
                }
                pos_ = 0;
                return;
            }
        }
        // Unmapped: reset to ASCII, emit error
        iso2022jp_state_ = 0;
        this->buf_[0]    = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
        len_             = 1;
        pos_             = 0;
    } else if constexpr (C == codec::euc_kr) {
        auto r = detail::euc_kr_encode_one(static_cast<char32_t>(*current_));
        ++current_;
        if (r.is_error) {
            this->buf_[0] = result_value{std::unexpect, whatwg_error::unmapped_codepoint};
            len_          = 1;
        } else {
            for (int i = 0; i < r.count; ++i)
                this->buf_[i] = result_value{static_cast<char>(r.bytes[i])};
            len_ = r.count;
        }
        pos_ = 0;
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
constexpr auto whatwg_encode_or_error_view<C, R>::iterator::operator*() const {
    return this->buf_[this->pos_];
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::iterator::operator++() -> iterator& {
    if (++pos_ < len_)
        return *this;
    load();
    return *this;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto whatwg_encode_or_error_view<C, R>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <codec C, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void whatwg_encode_or_error_view<C, R>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_encode_or_error_closure
// ---------------------------------------------------------------------------

template <codec C>
template <unicode_scalar_range R>
constexpr auto whatwg_encode_or_error_closure<C>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    if constexpr (detail::random_access_encode_codec<C> && std::ranges::random_access_range<view_t>)
        return random_access_whatwg_encode_or_error_view<C, view_t>(std::move(all));
    else
        return whatwg_encode_or_error_view<C, view_t>(std::move(all));
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODE_VIEW_HPP
