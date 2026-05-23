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

        void load() {
            if (current_ == end_) {
                done_ = true;
                return;
            }
            value_ = decode_one();
        }

        char32_t decode_one() {
            auto byte = static_cast<unsigned char>(*current_);
            ++current_;
            // All WHATWG codecs share the 7-bit ASCII base (U+0000–U+007F):
            // bytes 0x00–0x7F map directly to their code point value.
            if (byte < 0x80)
                return static_cast<char32_t>(byte);
            // TODO: multi-byte UTF-8 sequences (step 5/6) and WHATWG error
            // replacement rules (step 7) are not yet implemented. Any byte
            // with the high bit set incorrectly emits U+FFFD here.
            return U'�';
        }

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = char32_t;

        iterator(base_iter current, base_sent end) : current_(std::move(current)), end_(std::move(end)) { load(); }

        char32_t operator*() const { return value_; }

        iterator& operator++() {
            load();
            return *this;
        }

        void operator++(int) { ++*this; }

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    explicit whatwg_decode_view(R base) : base_(std::move(base)) {}

    iterator begin() { return iterator(std::ranges::begin(base_), std::ranges::end(base_)); }

    std::default_sentinel_t end() const { return std::default_sentinel; }
};

template <codec C>
struct whatwg_decode_closure {
    template <legacy_byte_range R>
    auto operator()(R&& r) const {
        return whatwg_decode_view<C, std::views::all_t<R>>(std::views::all(std::forward<R>(r)));
    }

    template <legacy_byte_range R>
    friend auto operator|(R&& r, const whatwg_decode_closure& self) {
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
