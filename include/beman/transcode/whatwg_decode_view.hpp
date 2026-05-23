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
            if (byte < 0x80)
                return static_cast<char32_t>(byte);
            // Placeholder: multi-byte and error handling in later steps
            return U'�';
        }

      public:
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = char32_t;

        iterator(base_iter current, base_sent end) : current_(std::move(current)), end_(std::move(end)) {
            load();
        }

        char32_t operator*() const { return value_; }

        iterator& operator++() {
            load();
            return *this;
        }

        void operator++(int) { ++*this; }

        friend bool operator==(iterator const& it, std::default_sentinel_t) { return it.done_; }
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
    friend auto operator|(R&& r, whatwg_decode_closure const& self) {
        return self(std::forward<R>(r));
    }
};

template <codec C>
inline constexpr whatwg_decode_closure<C> whatwg_decode{};

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_WHATWG_DECODE_VIEW_HPP
