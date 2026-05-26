// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_DECODE_VIEW_HPP

#include <beman/transcode/detail/single_byte_tables.hpp>

#include <iterator>
#include <ranges>

namespace beman::transcoding {

namespace detail {

// Forward declarations for table retrieval
// (defined in single_byte_tables.hpp)

} // namespace detail

template <codec C, std::ranges::random_access_range R>
    requires detail::single_byte_codec<C> && legacy_byte_range<R>
class single_byte_decode_view : public std::ranges::view_interface<single_byte_decode_view<C, R>> {
    R base_;

    class iterator {
        using base_iter = std::ranges::iterator_t<R>;

        base_iter     current_;
        const char32_t* table_;

      public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type       = char32_t;
        using difference_type  = std::ptrdiff_t;
        using reference        = char32_t;

        constexpr iterator() = default;

        constexpr iterator(base_iter current, const char32_t* table) : current_(current), table_(table) {}

        constexpr char32_t operator*() const {
            return detail::decode_single_byte_at(static_cast<unsigned char>(*current_), table_);
        }

        constexpr char32_t operator[](difference_type n) const {
            return detail::decode_single_byte_at(static_cast<unsigned char>(current_[n]), table_);
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

        constexpr iterator operator+(difference_type n) const {
            auto tmp = *this;
            return tmp += n;
        }

        constexpr friend iterator operator+(difference_type n, const iterator& it) { return it + n; }

        constexpr iterator& operator-=(difference_type n) {
            current_ -= n;
            return *this;
        }

        constexpr iterator operator-(difference_type n) const {
            auto tmp = *this;
            return tmp -= n;
        }

        constexpr difference_type operator-(const iterator& other) const { return current_ - other.current_; }

        constexpr friend bool operator==(const iterator& lhs, const iterator& rhs) {
            return lhs.current_ == rhs.current_;
        }

        constexpr friend std::strong_ordering operator<=>(const iterator& lhs, const iterator& rhs) {
            return lhs.current_ <=> rhs.current_;
        }
    };

  public:
    constexpr explicit single_byte_decode_view(R base) : base_(std::move(base)) {}

    constexpr iterator begin() const {
        return iterator(std::ranges::begin(base_), detail::get_decode_table<C>());
    }

    constexpr iterator end() const {
        return iterator(std::ranges::end(base_), detail::get_decode_table<C>());
    }

    constexpr auto size() const
        requires std::ranges::sized_range<R>
    {
        return std::ranges::size(base_);
    }
};

template <codec C>
struct single_byte_decode_closure {
    template <legacy_byte_range R>
        requires std::ranges::random_access_range<R>
    constexpr auto operator()(R&& r) const {
        static_assert(detail::single_byte_codec<C>, "Codec must be single-byte");
        return single_byte_decode_view<C, std::remove_cvref_t<R>>(std::forward<R>(r));
    }

    template <legacy_byte_range R>
        requires std::ranges::random_access_range<R>
    constexpr friend auto operator|(R&& r, const single_byte_decode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const single_byte_decode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "single_byte_decode: raw arrays are not valid input; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

template <codec C>
inline constexpr single_byte_decode_closure<C> single_byte_decode{};


} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_SINGLE_BYTE_DECODE_VIEW_HPP
