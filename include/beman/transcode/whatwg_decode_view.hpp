// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/detail/range_traits.hpp>
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
#include <beman/transcode/detail/tables/shift_jis.hpp>
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
#include <beman/transcode/detail/whatwg_decode_select.hpp>
#include <beman/transcode/error.hpp>
#include <beman/transcode/detail/single_byte.hpp>
#include <beman/transcode/detail/big5.hpp>
#include <beman/transcode/detail/euc_jp.hpp>
#include <beman/transcode/detail/euc_kr.hpp>
#include <beman/transcode/detail/gb18030.hpp>
#include <beman/transcode/detail/gbk.hpp>
#include <beman/transcode/detail/shift_jis.hpp>
#include <beman/transcode/detail/utf8.hpp>
#include <beman/transcode/detail/utf16.hpp>
#include <beman/transcode/detail/x_user_defined.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <expected>
    #include <compare>
    #include <algorithm>
    #include <iterator>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

template <codec C, std::ranges::random_access_range R, transcode_error_kind E = transcode_error_kind::replacement>
    requires legacy_byte_range<R> && detail::random_access_decode_codec<C>
class random_access_whatwg_decode_view
    : public std::ranges::view_interface<random_access_whatwg_decode_view<C, R, E>> {
    R base_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;

        base_iter current_{};

      public:
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type =
            std::conditional_t<E == transcode_error_kind::expected, std::expected<char32_t, whatwg_error>, char32_t>;

        // A failure, as this error kind reports it. In replacement mode the
        // table lookup has already produced U+FFFD, so both arms agree.
        static constexpr value_type error_result(whatwg_error e) {
            if constexpr (E == transcode_error_kind::expected)
                return value_type(std::unexpect, e);
            else
                return U'\xFFFD';
        }
        using difference_type = std::ptrdiff_t;
        using reference       = value_type;

        constexpr iterator() = default;
        constexpr explicit iterator(base_iter current) : current_(current) {}

        constexpr const base_iter& base() const noexcept { return current_; }

        constexpr value_type operator*() const {
            const auto byte = static_cast<unsigned char>(*current_);
            if constexpr (C == codec::x_user_defined) {
                return detail::decode_x_user_defined_code_unit(byte);
            } else {
                const auto cp = detail::decode_random_access_code_unit(byte, detail::random_access_decode_table<C>());
                if (cp == U'\xFFFD' && byte >= 0x80 && detail::random_access_decode_table<C>()[byte - 0x80] == 0)
                    return error_result(whatwg_error::invalid_byte);
                return cp;
            }
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
    constexpr explicit random_access_whatwg_decode_view(R base) : base_(std::move(base)) {}

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
// whatwg_decode_view — decodes bytes to expected<char32_t, whatwg_error>
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R, transcode_error_kind E = transcode_error_kind::replacement>
    requires legacy_byte_range<R>
class whatwg_decode_view : public std::ranges::view_interface<whatwg_decode_view<C, R, E>> {
    R base_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;
        using result_t =
            std::conditional_t<E == transcode_error_kind::expected, std::expected<char32_t, whatwg_error>, char32_t>;

        // Whether the loaded value is this code point, under either error kind.
        constexpr bool holds(char32_t cp) const {
            if constexpr (E == transcode_error_kind::expected)
                return value_.has_value() && *value_ == cp;
            else
                return value_ == cp;
        }

        // A failure, as this error kind reports it.
        static constexpr result_t error_result(whatwg_error e) {
            if constexpr (E == transcode_error_kind::expected)
                return result_t(std::unexpect, e);
            else
                return U'\xFFFD';
        }

        base_iter     current_{};
        base_sent     end_{};
        result_t      value_;
        bool          done_{false};
        int           pending_count_{0};
        unsigned char pending_[2]{};
        char32_t      pending_cp_{};
        bool          has_pending_cp_{false};
        int           iso2022jp_state_{0};
        int           iso2022jp_output_state_{0};
        bool          iso2022jp_output_flag_{false};
        unsigned char iso2022jp_lead_{0};
        unsigned char gb_replay_[3]{};
        int           gb_replay_count_{0};
        int           gb_replay_pos_{0};

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

        constexpr result_t  operator*() const;
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
            return lhs.current_ == rhs.current_ && lhs.value_ == rhs.value_ && lhs.done_ == rhs.done_ &&
                   lhs.pending_count_ == rhs.pending_count_ && lhs.pending_cp_ == rhs.pending_cp_ &&
                   lhs.has_pending_cp_ == rhs.has_pending_cp_ && lhs.iso2022jp_state_ == rhs.iso2022jp_state_ &&
                   lhs.iso2022jp_output_state_ == rhs.iso2022jp_output_state_ &&
                   lhs.iso2022jp_output_flag_ == rhs.iso2022jp_output_flag_ &&
                   lhs.iso2022jp_lead_ == rhs.iso2022jp_lead_ && lhs.gb_replay_count_ == rhs.gb_replay_count_ &&
                   lhs.gb_replay_pos_ == rhs.gb_replay_pos_ &&
                   std::equal(std::begin(lhs.pending_), std::end(lhs.pending_), std::begin(rhs.pending_)) &&
                   std::equal(std::begin(lhs.gb_replay_), std::end(lhs.gb_replay_), std::begin(rhs.gb_replay_));
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit whatwg_decode_view(R base);

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

template <codec C, transcode_error_kind E = transcode_error_kind::replacement>
struct whatwg_decode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const whatwg_decode_closure& self) {
        return self(std::forward<R>(r));
    }

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
inline constexpr auto whatwg_decode = whatwg_decode_closure<C, transcode_error_kind::replacement>{};

template <codec C>
inline constexpr auto whatwg_decode_or_error = whatwg_decode_closure<C, transcode_error_kind::expected>{};

// The names the paired views had before they were unified, kept as aliases so
// that `whatwg_decode_or_error_view<C, R>` still names the view the
// `_or_error` closure produces.
template <codec C, std::ranges::input_range R>
using whatwg_decode_or_error_view = whatwg_decode_view<C, R, transcode_error_kind::expected>;

template <codec C, std::ranges::random_access_range R>
using random_access_whatwg_decode_or_error_view =
    random_access_whatwg_decode_view<C, R, transcode_error_kind::expected>;

template <codec C>
using whatwg_decode_or_error_closure = whatwg_decode_closure<C, transcode_error_kind::expected>;

} // namespace beman::transcoding

namespace std::ranges {

template <beman::transcoding::codec C, random_access_range R, beman::transcoding::transcode_error_kind E>
    requires beman::transcoding::legacy_byte_range<R> && beman::transcoding::detail::random_access_decode_codec<C>
inline constexpr bool enable_borrowed_range<beman::transcoding::random_access_whatwg_decode_view<C, R, E>> =
    borrowed_range<R>;

template <beman::transcoding::codec C, input_range R, beman::transcoding::transcode_error_kind E>
    requires beman::transcoding::legacy_byte_range<R>
inline constexpr bool enable_borrowed_range<beman::transcoding::whatwg_decode_view<C, R, E>> = borrowed_range<R>;

} // namespace std::ranges

namespace beman::transcoding {

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_view
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr whatwg_decode_view<C, R, E>::whatwg_decode_view(R base) : base_(std::move(base)) {}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_));
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr std::default_sentinel_t whatwg_decode_view<C, R, E>::end() const {
    return std::default_sentinel;
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_view::iterator
// ---------------------------------------------------------------------------

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr void whatwg_decode_view<C, R, E>::iterator::load() {
    if (has_pending_cp_) {
        value_          = pending_cp_;
        has_pending_cp_ = false;
        return;
    }
    if (current_ == end_) {
        if (pending_count_ == 0 && iso2022jp_state_ <= 3 && gb_replay_pos_ >= gb_replay_count_) {
            done_ = true;
            return;
        }
    }
    if constexpr (C == codec::replacement) {
        while (current_ != end_)
            ++current_;
        value_ = error_result(whatwg_error::invalid_byte);
    } else if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_decode_one(current_, end_);
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::x_user_defined) {
        value_ = detail::x_user_defined_decode_one(current_, end_);
    } else if constexpr (C == codec::windows_1252) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1252);
        if (r.is_error)
            std::unreachable();
        else
            value_ = r.code_point;
    } else if constexpr (detail::single_byte_table_codec<C>) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::single_byte_decode_table<C>());
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::utf_16be || C == codec::utf_16le) {
        unsigned char b0;
        unsigned char b1;
        if (pending_count_ > 0) {
            b0             = pending_[0];
            b1             = pending_[1];
            pending_count_ = 0;
        } else {
            b0 = static_cast<unsigned char>(*current_);
            ++current_;
            if (current_ == end_) {
                value_ = error_result(whatwg_error::truncated_sequence);
                return;
            }
            b1 = static_cast<unsigned char>(*current_);
            ++current_;
        }
        char16_t unit;
        if constexpr (C == codec::utf_16be)
            unit = static_cast<char16_t>((static_cast<unsigned>(b0) << 8) | b1);
        else
            unit = static_cast<char16_t>((static_cast<unsigned>(b1) << 8) | b0);

        if (unit >= 0xD800 && unit <= 0xDBFF) {
            if (current_ == end_) {
                value_ = error_result(whatwg_error::truncated_sequence);
                return;
            }
            auto b2 = static_cast<unsigned char>(*current_);
            ++current_;
            if (current_ == end_) {
                value_ = error_result(whatwg_error::truncated_sequence);
                return;
            }
            auto b3 = static_cast<unsigned char>(*current_);
            ++current_;
            char16_t low;
            if constexpr (C == codec::utf_16be)
                low = static_cast<char16_t>((static_cast<unsigned>(b2) << 8) | b3);
            else
                low = static_cast<char16_t>((static_cast<unsigned>(b3) << 8) | b2);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                value_ = 0x10000 + ((static_cast<char32_t>(unit - 0xD800) << 10) | (low - 0xDC00));
            } else {
                value_         = error_result(whatwg_error::surrogate_code_point);
                pending_[0]    = b2;
                pending_[1]    = b3;
                pending_count_ = 2;
            }
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            value_ = error_result(whatwg_error::surrogate_code_point);
        } else {
            value_ = static_cast<char32_t>(unit);
        }
    } else if constexpr (C == codec::gbk || C == codec::gb18030) {
        if (gb_replay_pos_ < gb_replay_count_) {
            auto byte = gb_replay_[gb_replay_pos_++];
            if (gb_replay_pos_ == gb_replay_count_) {
                gb_replay_count_ = 0;
                gb_replay_pos_   = 0;
            }
            if (byte < 0x80) {
                value_ = static_cast<char32_t>(byte);
            } else {
                unsigned char buf[4];
                buf[0]    = byte;
                int count = 1;
                while (count < 4 && current_ != end_) {
                    buf[count++] = static_cast<unsigned char>(*current_);
                    ++current_;
                }
                const unsigned char* bp   = buf;
                const unsigned char* be   = buf + count;
                auto                 r    = detail::gb18030_decode_one(bp, be);
                int                  left = static_cast<int>(be - bp);
                if (left > 0) {
                    gb_replay_count_ = left;
                    gb_replay_pos_   = 0;
                    for (int i = 0; i < left; ++i)
                        gb_replay_[i] = bp[i];
                }
                if (r.is_error)
                    value_ = error_result(r.error);
                else
                    value_ = r.code_point;
            }
            return;
        }
        auto r = detail::gb18030_decode_one(current_, end_);
        if (r.is_error) {
            value_ = error_result(r.error);
            if (r.replay_count > 0) {
                gb_replay_count_ = r.replay_count;
                gb_replay_pos_   = 0;
                for (int i = 0; i < r.replay_count; ++i)
                    gb_replay_[i] = r.replay[i];
            }
        } else {
            value_ = r.code_point;
        }
    } else if constexpr (C == codec::big5) {
        auto r = detail::big5_decode_one(current_, end_);
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
        if (r.code_point2 != 0) {
            pending_cp_     = r.code_point2;
            has_pending_cp_ = true;
        }
    } else if constexpr (C == codec::shift_jis) {
        auto r = detail::shift_jis_decode_one(current_, end_);
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::euc_jp) {
        auto r = detail::euc_jp_decode_one(current_, end_);
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_2022_jp) {
        // States: 0=ASCII, 1=Roman, 2=Katakana, 3=Lead_Byte, 4=Trail_Byte,
        //         5=Escape_Start, 6=Escape
        while (true) {
            unsigned char byte;
            if (pending_count_ > 0) {
                byte = pending_[0];
                if (pending_count_ > 1)
                    pending_[0] = pending_[1];
                --pending_count_;
            } else if (current_ != end_) {
                byte = static_cast<unsigned char>(*current_++);
            } else {
                switch (iso2022jp_state_) {
                default: // 0=ASCII, 1=Roman, 2=Katakana, 3=Lead_Byte
                    done_ = true;
                    return;
                case 4:
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = error_result(whatwg_error::truncated_sequence);
                    return;
                case 5:
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = error_result(whatwg_error::truncated_sequence);
                    return;
                case 6:
                    pending_[0]            = iso2022jp_lead_;
                    pending_count_         = 1;
                    iso2022jp_lead_        = 0;
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = error_result(whatwg_error::truncated_sequence);
                    return;
                }
            }

            switch (iso2022jp_state_) {
            case 5:
                if (byte == 0x24 || byte == 0x28) {
                    iso2022jp_lead_  = byte;
                    iso2022jp_state_ = 6;
                    continue;
                }
                pending_[0]            = byte;
                pending_count_         = 1;
                iso2022jp_state_       = iso2022jp_output_state_;
                iso2022jp_output_flag_ = false;
                value_                 = error_result(whatwg_error::invalid_byte);
                return;

            case 6: {
                auto lead       = iso2022jp_lead_;
                iso2022jp_lead_ = 0;
                int new_state   = -1;
                if (lead == 0x28) {
                    if (byte == 0x42)
                        new_state = 0;
                    else if (byte == 0x4A)
                        new_state = 1;
                    else if (byte == 0x49)
                        new_state = 2;
                } else {
                    if (byte == 0x40 || byte == 0x42)
                        new_state = 3;
                }
                if (new_state < 0) {
                    pending_[0]            = lead;
                    pending_[1]            = byte;
                    pending_count_         = 2;
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = error_result(whatwg_error::invalid_byte);
                    return;
                }
                iso2022jp_output_state_ = new_state;
                iso2022jp_state_        = new_state;
                if (iso2022jp_output_flag_) {
                    iso2022jp_output_flag_ = false;
                    value_                 = error_result(whatwg_error::invalid_byte);
                    return;
                }
                iso2022jp_output_flag_ = true;
                continue;
            }

            case 3:
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                if (byte >= 0x21 && byte <= 0x7E) {
                    iso2022jp_lead_  = byte;
                    iso2022jp_state_ = 4;
                    continue;
                }
                iso2022jp_state_       = iso2022jp_output_state_;
                iso2022jp_output_flag_ = false;
                value_                 = error_result(whatwg_error::invalid_byte);
                return;

            case 4: {
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                iso2022jp_state_       = iso2022jp_output_state_;
                iso2022jp_output_flag_ = false;
                if (byte >= 0x21 && byte <= 0x7E) {
                    int  pointer = ((static_cast<int>(iso2022jp_lead_) - 0x21) * 94) + (static_cast<int>(byte) - 0x21);
                    auto cp      = detail::tables::shift_jis[pointer];
                    if (cp != 0) {
                        value_ = cp;
                        return;
                    }
                }
                value_ = error_result(whatwg_error::invalid_byte);
                return;
            }

            default: // 0=ASCII, 1=Roman, 2=Katakana
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                if (byte == 0x0E || byte == 0x0F) {
                    iso2022jp_output_flag_ = false;
                    value_                 = error_result(whatwg_error::invalid_byte);
                    return;
                }
                iso2022jp_output_flag_ = false;
                if (iso2022jp_state_ == 0) {
                    if (byte <= 0x7F)
                        value_ = static_cast<char32_t>(byte);
                    else
                        value_ = error_result(whatwg_error::invalid_byte);
                    return;
                }
                if (iso2022jp_state_ == 1) {
                    if (byte == 0x5C) {
                        value_ = U'\x00A5';
                        return;
                    }
                    if (byte == 0x7E) {
                        value_ = U'\x203E';
                        return;
                    }
                    if (byte <= 0x7F)
                        value_ = static_cast<char32_t>(byte);
                    else
                        value_ = error_result(whatwg_error::invalid_byte);
                    return;
                }
                // Katakana: 0x21-0x5F → U+FF61-U+FF9F
                if (byte >= 0x21 && byte <= 0x5F)
                    value_ = static_cast<char32_t>(0xFF61 + byte - 0x21);
                else
                    value_ = error_result(whatwg_error::invalid_byte);
                return;
            }
        }
    } else if constexpr (C == codec::euc_kr) {
        auto r = detail::euc_kr_decode_one(current_, end_);
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
    }
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr whatwg_decode_view<C, R, E>::iterator::iterator(base_iter current, base_sent end)
    : current_(std::move(current)), end_(std::move(end)) {
    load();
    if constexpr (C == codec::utf_8 || C == codec::utf_16le || C == codec::utf_16be) {
        if (!done_ && holds(U'\xFEFF'))
            load();
    }
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::iterator::operator*() const -> result_t {
    return value_;
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto whatwg_decode_view<C, R, E>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <codec C, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr void whatwg_decode_view<C, R, E>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg_decode_closure
// ---------------------------------------------------------------------------

template <codec C, transcode_error_kind E>
template <legacy_byte_range R>
constexpr auto whatwg_decode_closure<C, E>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    if constexpr (detail::random_access_decode_codec<C> && std::ranges::random_access_range<view_t>)
        return random_access_whatwg_decode_view<C, view_t, E>(std::move(all));
    else
        return whatwg_decode_view<C, view_t, E>(std::move(all));
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
