// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_CONCEPTS_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_CONCEPTS_HPP

#include <concepts>
#include <cstddef>
#include <ranges>
#include <type_traits>

namespace beman::transcoding {

namespace detail {

template <typename T>
concept legacy_byte_type = std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> ||
                           std::same_as<T, std::byte>;

} // namespace detail

template <typename R>
concept legacy_byte_range = std::ranges::range<R> && !std::is_array_v<std::remove_cvref_t<R>> &&
                            detail::legacy_byte_type<std::remove_cv_t<std::ranges::range_value_t<R>>>;

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_CONCEPTS_HPP
