// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TRANSCODE_DETAIL_NULL_TERM_HPP
#define BEMAN_TRANSCODE_DETAIL_NULL_TERM_HPP

#include <iterator>
#include <ranges>
#include <type_traits>

namespace beman::transcoding {

struct null_sentinel_t {
    template <std::input_iterator I>
        requires requires(I i) {
            { *i == 0 };
        }
    friend constexpr bool operator==(const I& it, null_sentinel_t) {
        return *it == 0;
    }
};

inline constexpr null_sentinel_t null_sentinel{};

template <std::contiguous_iterator I>
class null_term_view : public std::ranges::view_interface<null_term_view<I>> {
    I ptr_;

  public:
    constexpr null_term_view() = default;
    constexpr explicit null_term_view(I ptr) : ptr_(ptr) {}

    constexpr I               begin() const { return ptr_; }
    constexpr null_sentinel_t end() const { return {}; }
};

template <std::contiguous_iterator I>
null_term_view(I) -> null_term_view<I>;

namespace detail {

struct null_term_fn {
    template <typename T>
        requires std::is_pointer_v<std::decay_t<T>>
    constexpr auto operator()(T&& ptr) const {
        return null_term_view(std::decay_t<T>(ptr));
    }

    template <typename T, std::size_t N>
    constexpr auto operator()(T (&arr)[N]) const {
        return null_term_view(static_cast<const T*>(arr));
    }

    // Explicit diagnostic for non-pointer, non-array types (e.g. std::vector<char>).
    // The static_assert fires with a stable message instead of a deep substitution error.
    template <typename T>
        requires(!std::is_pointer_v<std::decay_t<T>> && !std::is_array_v<std::decay_t<T>>)
    constexpr auto operator()(T&&) const {
        static_assert(std::is_pointer_v<std::decay_t<T>>,
                      "transcode: views::null_term requires a pointer or array type; "
                      "pass a const char* or use a range that already has a null terminator");
    }
};

struct null_term_adaptor : null_term_fn {
    using null_term_fn::operator();

    template <typename R>
    friend constexpr auto operator|(R&& r, const null_term_adaptor& self) {
        return self(std::forward<R>(r));
    }
};

} // namespace detail

namespace views {
inline constexpr detail::null_term_adaptor null_term{};
} // namespace views

} // namespace beman::transcoding

#endif // BEMAN_TRANSCODE_DETAIL_NULL_TERM_HPP
