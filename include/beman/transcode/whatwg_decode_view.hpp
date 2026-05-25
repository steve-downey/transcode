// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/detail/single_byte.hpp>
#include <beman/transcode/detail/tables/ibm866.hpp>
#include <beman/transcode/detail/tables/iso_8859_2.hpp>
#include <beman/transcode/detail/tables/iso_8859_3.hpp>
#include <beman/transcode/detail/tables/iso_8859_4.hpp>
#include <beman/transcode/detail/tables/iso_8859_5.hpp>
#include <beman/transcode/detail/tables/iso_8859_6.hpp>
#include <beman/transcode/detail/tables/iso_8859_7.hpp>
#include <beman/transcode/detail/tables/iso_8859_8.hpp>
#include <beman/transcode/detail/tables/iso_8859_10.hpp>
#include <beman/transcode/detail/tables/iso_8859_13.hpp>
#include <beman/transcode/detail/tables/iso_8859_14.hpp>
#include <beman/transcode/detail/tables/iso_8859_15.hpp>
#include <beman/transcode/detail/tables/iso_8859_16.hpp>
#include <beman/transcode/detail/tables/koi8_r.hpp>
#include <beman/transcode/detail/tables/koi8_u.hpp>
#include <beman/transcode/detail/tables/macintosh.hpp>
#include <beman/transcode/detail/tables/windows_874.hpp>
#include <beman/transcode/detail/tables/windows_1250.hpp>
#include <beman/transcode/detail/tables/windows_1251.hpp>
#include <beman/transcode/detail/tables/windows_1252.hpp>
#include <beman/transcode/detail/tables/windows_1253.hpp>
#include <beman/transcode/detail/tables/windows_1254.hpp>
#include <beman/transcode/detail/tables/windows_1255.hpp>
#include <beman/transcode/detail/tables/windows_1256.hpp>
#include <beman/transcode/detail/tables/windows_1257.hpp>
#include <beman/transcode/detail/tables/windows_1258.hpp>
#include <beman/transcode/detail/tables/x_mac_cyrillic.hpp>
#include <beman/transcode/detail/big5.hpp>
#include <beman/transcode/detail/euc_jp.hpp>
#include <beman/transcode/detail/euc_kr.hpp>
#include <beman/transcode/detail/gb18030.hpp>
#include <beman/transcode/detail/gbk.hpp>
#include <beman/transcode/detail/shift_jis.hpp>
#include <beman/transcode/detail/utf8.hpp>
#include <beman/transcode/detail/utf16.hpp>
#include <beman/transcode/detail/x_user_defined.hpp>

#include <expected>
#include <iterator>
#include <ranges>

namespace beman::transcoding {

enum class codec {
    utf_8,
    replacement,
    x_user_defined,
    ibm866,
    iso_8859_2,
    iso_8859_3,
    iso_8859_4,
    iso_8859_5,
    iso_8859_6,
    iso_8859_7,
    iso_8859_8,
    iso_8859_8_i,
    iso_8859_10,
    iso_8859_13,
    iso_8859_14,
    iso_8859_15,
    iso_8859_16,
    koi8_r,
    koi8_u,
    macintosh,
    windows_874,
    windows_1250,
    windows_1251,
    windows_1252,
    windows_1253,
    windows_1254,
    windows_1255,
    windows_1256,
    windows_1257,
    windows_1258,
    x_mac_cyrillic,
    utf_16be,
    utf_16le,
    gbk,
    gb18030,
    big5,
    shift_jis,
    euc_jp,
    iso_2022_jp,
    euc_kr,
};

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

        base_iter     current_;
        base_sent     end_;
        char32_t      value_{};
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
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = char32_t;

        constexpr iterator(base_iter current, base_sent end);

        constexpr char32_t  operator*() const;
        constexpr iterator& operator++();
        constexpr void      operator++(int);

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
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

        base_iter     current_;
        base_sent     end_;
        result_t      value_{};
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
        using iterator_concept = std::input_iterator_tag;
        using value_type       = result_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = result_t;

        constexpr iterator(base_iter current, base_sent end);

        constexpr result_t  operator*() const;
        constexpr iterator& operator++();
        constexpr void      operator++(int);

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
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
        value_ = U'\xFFFD';
    } else if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_decode_one(current_, end_);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::x_user_defined) {
        value_ = detail::x_user_defined_decode_one(current_, end_);
    } else if constexpr (C == codec::ibm866) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::ibm866);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_2) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_2);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_3) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_3);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_4) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_4);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_5) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_5);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_6) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_6);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_7) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_7);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_8);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_10) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_10);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_13) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_13);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_14) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_14);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_15) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_15);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_8859_16) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_16);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::koi8_r) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::koi8_r);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::koi8_u) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::koi8_u);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::macintosh) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::macintosh);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_874) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_874);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1250) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1250);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1251) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1251);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1252) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1252);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1253) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1253);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1254) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1254);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1255) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1255);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1256) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1256);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1257) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1257);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::windows_1258) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1258);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::x_mac_cyrillic) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::x_mac_cyrillic);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::utf_16be || C == codec::utf_16le) {
        // Read the first code unit (2 bytes), using pending buffer if available.
        unsigned char b0, b1;
        if (pending_count_ > 0) {
            b0             = pending_[0];
            b1             = pending_[1];
            pending_count_ = 0;
        } else {
            b0 = static_cast<unsigned char>(*current_);
            ++current_;
            if (current_ == end_) {
                value_ = U'\xFFFD';
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
            // High surrogate: peek next 2 bytes
            if (current_ == end_) {
                value_ = U'\xFFFD';
                return;
            }
            auto b2 = static_cast<unsigned char>(*current_);
            ++current_;
            if (current_ == end_) {
                value_ = U'\xFFFD';
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
                // Not a valid low surrogate: emit U+FFFD and save b2/b3 for next call
                value_         = U'\xFFFD';
                pending_[0]    = b2;
                pending_[1]    = b3;
                pending_count_ = 2;
            }
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            value_ = U'\xFFFD';
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
            // Replay bytes: [0] is always 0x30-0x39 (ASCII digit).
            // [1] if present is always 0x81-0xFE (GB lead byte) and must be
            // re-decoded with its trail from the real stream.
            if (byte < 0x80) {
                value_ = static_cast<char32_t>(byte);
            } else {
                // Feed replay lead + remaining stream through the decoder.
                // Wrap in a tiny array prepended to the real iterator.
                // Since gb18030_decode_one can work with const unsigned char*,
                // we copy up to 3 more bytes from the stream into a local buf.
                unsigned char buf[4];
                buf[0]    = byte;
                int count = 1;
                while (count < 4 && current_ != end_) {
                    buf[count++] = static_cast<unsigned char>(*current_);
                    ++current_;
                }
                const unsigned char* bp = buf;
                const unsigned char* be = buf + count;
                auto                 r  = detail::gb18030_decode_one(bp, be);
                // Push un-consumed buf bytes back into replay.
                int left = static_cast<int>(be - bp);
                if (left > 0) {
                    gb_replay_count_ = left;
                    gb_replay_pos_   = 0;
                    for (int i = 0; i < left; ++i)
                        gb_replay_[i] = bp[i];
                }
                value_ = r.is_error ? U'\xFFFD' : r.code_point;
            }
            return;
        }
        auto r = detail::gb18030_decode_one(current_, end_);
        if (r.is_error) {
            value_ = U'\xFFFD';
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
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
        if (r.code_point2 != 0) {
            pending_cp_     = r.code_point2;
            has_pending_cp_ = true;
        }
    } else if constexpr (C == codec::shift_jis) {
        auto r = detail::shift_jis_decode_one(current_, end_);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::euc_jp) {
        auto r = detail::euc_jp_decode_one(current_, end_);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    } else if constexpr (C == codec::iso_2022_jp) {
        // States: 0=ASCII, 1=Roman, 2=Katakana, 3=Lead_Byte, 4=Trail_Byte,
        //         5=Escape_Start, 6=Escape
        // output_state tracks where to return after escape sequences / errors.
        // output_flag detects redundant (same-state) ESC sequences → emit U+FFFD.
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
                // End of stream
                switch (iso2022jp_state_) {
                default: // 0=ASCII, 1=Roman, 2=Katakana, 3=Lead_Byte
                    done_ = true;
                    return;
                case 4: // Trail_Byte: truncated sequence
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = U'\xFFFD';
                    return;
                case 5: // Escape_Start: truncated escape
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = U'\xFFFD';
                    return;
                case 6: // Escape: prepend lead, restore state
                    pending_[0]            = iso2022jp_lead_;
                    pending_count_         = 1;
                    iso2022jp_lead_        = 0;
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = U'\xFFFD';
                    return;
                }
            }

            switch (iso2022jp_state_) {
            case 5: // Escape_Start
                if (byte == 0x24 || byte == 0x28) {
                    iso2022jp_lead_  = byte;
                    iso2022jp_state_ = 6;
                    continue;
                }
                pending_[0]            = byte;
                pending_count_         = 1;
                iso2022jp_state_       = iso2022jp_output_state_;
                iso2022jp_output_flag_ = false;
                value_                 = U'\xFFFD';
                return;

            case 6: { // Escape
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
                } else { // lead == 0x24
                    if (byte == 0x40 || byte == 0x42)
                        new_state = 3;
                }
                if (new_state < 0) {
                    pending_[0]            = lead;
                    pending_[1]            = byte;
                    pending_count_         = 2;
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = U'\xFFFD';
                    return;
                }
                iso2022jp_output_state_ = new_state;
                iso2022jp_state_        = new_state;
                if (iso2022jp_output_flag_) {
                    iso2022jp_output_flag_ = false;
                    value_                 = U'\xFFFD';
                    return;
                }
                iso2022jp_output_flag_ = true;
                continue;
            }

            case 3: // Lead_Byte
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
                value_                 = U'\xFFFD';
                return;

            case 4: { // Trail_Byte
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                iso2022jp_state_       = iso2022jp_output_state_;
                iso2022jp_output_flag_ = false;
                if (byte >= 0x21 && byte <= 0x7E) {
                    int  pointer = (static_cast<int>(iso2022jp_lead_) - 0x21) * 94 + (static_cast<int>(byte) - 0x21);
                    auto cp      = detail::tables::shift_jis[pointer];
                    if (cp != 0) {
                        value_ = cp;
                        return;
                    }
                }
                value_ = U'\xFFFD';
                return;
            }

            default: // 0=ASCII, 1=Roman, 2=Katakana
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                if (byte == 0x0E || byte == 0x0F) {
                    iso2022jp_output_flag_ = false;
                    value_                 = U'\xFFFD';
                    return;
                }
                iso2022jp_output_flag_ = false;
                if (iso2022jp_state_ == 0) { // ASCII
                    value_ = (byte <= 0x7F) ? static_cast<char32_t>(byte) : U'\xFFFD';
                    return;
                }
                if (iso2022jp_state_ == 1) { // Roman
                    if (byte == 0x5C) {
                        value_ = U'\x00A5';
                        return;
                    }
                    if (byte == 0x7E) {
                        value_ = U'\x203E';
                        return;
                    }
                    value_ = (byte <= 0x7F) ? static_cast<char32_t>(byte) : U'\xFFFD';
                    return;
                }
                // Katakana: 0x21-0x5F → U+FF61-U+FF9F
                value_ = (byte >= 0x21 && byte <= 0x5F) ? static_cast<char32_t>(0xFF61 + byte - 0x21) : U'\xFFFD';
                return;
            }
        }
    } else if constexpr (C == codec::euc_kr) {
        auto r = detail::euc_kr_decode_one(current_, end_);
        value_ = r.is_error ? U'\xFFFD' : r.code_point;
    }
}

template <codec C, std::ranges::input_range R>
    requires legacy_byte_range<R>
constexpr whatwg_decode_view<C, R>::iterator::iterator(base_iter current, base_sent end)
    : current_(std::move(current)), end_(std::move(end)) {
    load();
    if constexpr (C == codec::utf_8 || C == codec::utf_16le || C == codec::utf_16be) {
        if (!done_ && value_ == U'\xFEFF')
            load();
    }
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
        value_ = std::unexpected(whatwg_error::invalid_byte);
    } else if constexpr (C == codec::utf_8) {
        auto r = detail::utf8_decode_one(current_, end_);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::x_user_defined) {
        value_ = detail::x_user_defined_decode_one(current_, end_);
    } else if constexpr (C == codec::ibm866) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::ibm866);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_2) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_2);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_3) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_3);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_4) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_4);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_5) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_5);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_6) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_6);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_7) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_7);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_8 || C == codec::iso_8859_8_i) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_8);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_10) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_10);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_13) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_13);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_14) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_14);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_15) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_15);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::iso_8859_16) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::iso_8859_16);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::koi8_r) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::koi8_r);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::koi8_u) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::koi8_u);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::macintosh) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::macintosh);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_874) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_874);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1250) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1250);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1251) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1251);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1252) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1252);
        if (r.is_error)
            std::unreachable();
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1253) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1253);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1254) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1254);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1255) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1255);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1256) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1256);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1257) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1257);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::windows_1258) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::windows_1258);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::x_mac_cyrillic) {
        auto r = detail::single_byte_decode_one(current_, end_, detail::tables::x_mac_cyrillic);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::utf_16be || C == codec::utf_16le) {
        unsigned char b0, b1;
        if (pending_count_ > 0) {
            b0             = pending_[0];
            b1             = pending_[1];
            pending_count_ = 0;
        } else {
            b0 = static_cast<unsigned char>(*current_);
            ++current_;
            if (current_ == end_) {
                value_ = std::unexpected(whatwg_error::truncated_sequence);
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
                value_ = std::unexpected(whatwg_error::truncated_sequence);
                return;
            }
            auto b2 = static_cast<unsigned char>(*current_);
            ++current_;
            if (current_ == end_) {
                value_ = std::unexpected(whatwg_error::truncated_sequence);
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
                value_         = std::unexpected(whatwg_error::surrogate_code_point);
                pending_[0]    = b2;
                pending_[1]    = b3;
                pending_count_ = 2;
            }
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            value_ = std::unexpected(whatwg_error::surrogate_code_point);
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
                    value_ = std::unexpected(r.error);
                else
                    value_ = r.code_point;
            }
            return;
        }
        auto r = detail::gb18030_decode_one(current_, end_);
        if (r.is_error) {
            value_ = std::unexpected(r.error);
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
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
        if (r.code_point2 != 0) {
            pending_cp_     = r.code_point2;
            has_pending_cp_ = true;
        }
    } else if constexpr (C == codec::shift_jis) {
        auto r = detail::shift_jis_decode_one(current_, end_);
        if (r.is_error)
            value_ = std::unexpected(r.error);
        else
            value_ = r.code_point;
    } else if constexpr (C == codec::euc_jp) {
        auto r = detail::euc_jp_decode_one(current_, end_);
        if (r.is_error)
            value_ = std::unexpected(r.error);
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
                    value_                 = std::unexpected(whatwg_error::truncated_sequence);
                    return;
                case 5:
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = std::unexpected(whatwg_error::truncated_sequence);
                    return;
                case 6:
                    pending_[0]            = iso2022jp_lead_;
                    pending_count_         = 1;
                    iso2022jp_lead_        = 0;
                    iso2022jp_state_       = iso2022jp_output_state_;
                    iso2022jp_output_flag_ = false;
                    value_                 = std::unexpected(whatwg_error::truncated_sequence);
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
                value_                 = std::unexpected(whatwg_error::invalid_byte);
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
                    value_                 = std::unexpected(whatwg_error::invalid_byte);
                    return;
                }
                iso2022jp_output_state_ = new_state;
                iso2022jp_state_        = new_state;
                if (iso2022jp_output_flag_) {
                    iso2022jp_output_flag_ = false;
                    value_                 = std::unexpected(whatwg_error::invalid_byte);
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
                value_                 = std::unexpected(whatwg_error::invalid_byte);
                return;

            case 4: {
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                iso2022jp_state_       = iso2022jp_output_state_;
                iso2022jp_output_flag_ = false;
                if (byte >= 0x21 && byte <= 0x7E) {
                    int  pointer = (static_cast<int>(iso2022jp_lead_) - 0x21) * 94 + (static_cast<int>(byte) - 0x21);
                    auto cp      = detail::tables::shift_jis[pointer];
                    if (cp != 0) {
                        value_ = cp;
                        return;
                    }
                }
                value_ = std::unexpected(whatwg_error::invalid_byte);
                return;
            }

            default: // 0=ASCII, 1=Roman, 2=Katakana
                if (byte == 0x1B) {
                    iso2022jp_state_ = 5;
                    continue;
                }
                if (byte == 0x0E || byte == 0x0F) {
                    iso2022jp_output_flag_ = false;
                    value_                 = std::unexpected(whatwg_error::invalid_byte);
                    return;
                }
                iso2022jp_output_flag_ = false;
                if (iso2022jp_state_ == 0) {
                    if (byte <= 0x7F)
                        value_ = static_cast<char32_t>(byte);
                    else
                        value_ = std::unexpected(whatwg_error::invalid_byte);
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
                        value_ = std::unexpected(whatwg_error::invalid_byte);
                    return;
                }
                // Katakana: 0x21-0x5F → U+FF61-U+FF9F
                if (byte >= 0x21 && byte <= 0x5F)
                    value_ = static_cast<char32_t>(0xFF61 + byte - 0x21);
                else
                    value_ = std::unexpected(whatwg_error::invalid_byte);
                return;
            }
        }
    } else if constexpr (C == codec::euc_kr) {
        auto r = detail::euc_kr_decode_one(current_, end_);
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
    if constexpr (C == codec::utf_8 || C == codec::utf_16le || C == codec::utf_16be) {
        if (!done_ && value_.has_value() && *value_ == U'\xFEFF')
            load();
    }
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
