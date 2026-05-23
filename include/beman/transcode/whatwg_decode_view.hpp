// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
#define BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP

#include <beman/transcode/detail/concepts.hpp>

#include <iterator>
#include <ranges>

namespace beman::transcoding {

enum class codec { utf_8 };

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
            value_ = decode_one();
        }

        constexpr char32_t decode_one() {
            auto byte = static_cast<unsigned char>(*current_);
            ++current_;

            // ASCII fast path: all WHATWG codecs share the 7-bit base.
            if (byte < 0x80)
                return static_cast<char32_t>(byte);

            // Determine sequence length and initial code point bits.
            int      extra;
            char32_t cp;
            if (byte >= 0xC2 && byte <= 0xDF) {
                extra = 1;
                cp    = byte & 0x1F;
            } else if (byte >= 0xE0 && byte <= 0xEF) {
                extra = 2;
                cp    = byte & 0x0F;
            } else if (byte >= 0xF0 && byte <= 0xF4) {
                extra = 3;
                cp    = byte & 0x07;
            } else {
                // 0x80–0xBF: unexpected continuation byte.
                // 0xC0–0xC1: always-overlong leads (pre-rejected).
                // 0xF5–0xFF: out-of-range leads.
                return U'�';
            }

            // Consume continuation bytes (0x80–0xBF each).
            for (int i = 0; i < extra; ++i) {
                if (current_ == end_)
                    return U'�'; // truncated sequence
                auto cont = static_cast<unsigned char>(*current_);
                if ((cont & 0xC0) != 0x80)
                    return U'�'; // bad continuation; do NOT advance —
                                 // next load() re-processes this byte
                ++current_;
                cp = (cp << 6) | (cont & 0x3F);
            }

            // Validity: surrogates and out-of-Unicode-range.
            if (cp >= 0xD800 && cp <= 0xDFFF)
                return U'�';
            if (cp > 0x10FFFF)
                return U'�';

            // Overlong check (should not be reachable given lead byte range
            // above, but guard explicitly for clarity).
            if (extra == 1 && cp < 0x80)
                return U'�';
            if (extra == 2 && cp < 0x800)
                return U'�';
            if (extra == 3 && cp < 0x10000)
                return U'�';

            return cp;
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

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
