// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_CONCEPTS_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_CONCEPTS_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <concepts>
    #include <cstddef>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

namespace detail {

template <typename T>
concept legacy_byte_type = std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> ||
                           std::same_as<T, std::byte>;

template <typename Mutable, typename Const, bool = std::common_with<Mutable, Const>>
struct common_or_mutable {
    using type = Mutable;
};

template <typename Mutable, typename Const>
struct common_or_mutable<Mutable, Const, true> {
    using type = std::common_type_t<Mutable, Const>;
};

template <typename R, bool = std::ranges::range<const R>>
struct compatible_iterator {
    using type = std::ranges::iterator_t<R>;
};

template <typename R>
struct compatible_iterator<R, true> {
    using type = typename common_or_mutable<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>>::type;
};

template <typename R>
using compatible_iterator_t = typename compatible_iterator<R>::type;

template <typename R, bool = std::ranges::range<const R>>
struct compatible_sentinel {
    using type = std::ranges::sentinel_t<R>;
};

template <typename R>
struct compatible_sentinel<R, true> {
    using type = typename common_or_mutable<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>::type;
};

template <typename R>
using compatible_sentinel_t = typename compatible_sentinel<R>::type;

template <typename R>
concept const_iterator_compatible_range =
    std::ranges::range<const R> && std::constructible_from<compatible_iterator_t<R>, std::ranges::iterator_t<const R>>;

template <typename R>
concept const_sentinel_compatible_range =
    std::ranges::range<const R> && std::constructible_from<compatible_sentinel_t<R>, std::ranges::sentinel_t<const R>>;

} // namespace detail

template <typename R>
concept legacy_byte_range = std::ranges::range<R> && !std::is_array_v<std::remove_cvref_t<R>> &&
                            detail::legacy_byte_type<std::remove_cv_t<std::ranges::range_value_t<R>>>;

template <typename R>
concept unicode_scalar_range = std::ranges::input_range<R> && !std::is_array_v<std::remove_cvref_t<R>> &&
                               std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, char32_t>;

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_CONCEPTS_HPP
