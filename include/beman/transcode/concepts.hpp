// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_CONCEPTS_HPP
#define INCLUDE_BEMAN_TRANSCODE_CONCEPTS_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <concepts>
    #include <cstddef>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

// \ref{transcode.reqs}, range requirements

namespace detail {

//! \expos
template <typename T>
concept legacy_byte_type = std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> ||
                           std::same_as<T, std::byte>;

} // namespace detail

//! \remarks An array type does not satisfy `legacy_byte_range`, so that the
//! terminating null character of a string literal or of a character array is
//! not transcoded along with the rest of the array.  `views::null_term`
//! adapts a null-terminated character sequence, and `span` a counted buffer.
template <typename R>
concept legacy_byte_range = std::ranges::range<R> && !std::is_array_v<std::remove_cvref_t<R>> &&
                            detail::legacy_byte_type<std::remove_cv_t<std::ranges::range_value_t<R>>>;

//! \remarks `unicode_scalar_range` constrains the type of a range and not its
//! values.  Its `char32_t` elements are interpreted as UTF-32 code units by an
//! encode operation: a code unit that is not a Unicode scalar value is replaced
//! with U+FFFD or reported as an error, according to the operation's error kind.
template <typename R>
concept unicode_scalar_range = std::ranges::input_range<R> && !std::is_array_v<std::remove_cvref_t<R>> &&
                               std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, char32_t>;

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_CONCEPTS_HPP
