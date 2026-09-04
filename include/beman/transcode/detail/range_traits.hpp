// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Range traits used by the views to decide what a const-qualified view can
// iterate with. Implementation detail: the wording states the const-iteration
// requirements in prose, so these must not appear in a specification header.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_RANGE_TRAITS_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_RANGE_TRAITS_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <concepts>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding::detail {

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
    using type = common_or_mutable<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>>::type;
};

template <typename R>
using compatible_iterator_t = compatible_iterator<R>::type;

template <typename R, bool = std::ranges::range<const R>>
struct compatible_sentinel {
    using type = std::ranges::sentinel_t<R>;
};

template <typename R>
struct compatible_sentinel<R, true> {
    using type = common_or_mutable<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>::type;
};

template <typename R>
using compatible_sentinel_t = compatible_sentinel<R>::type;

template <typename R>
concept const_iterator_compatible_range =
    std::ranges::range<const R> && std::constructible_from<compatible_iterator_t<R>, std::ranges::iterator_t<const R>>;

template <typename R>
concept const_sentinel_compatible_range =
    std::ranges::range<const R> && std::constructible_from<compatible_sentinel_t<R>, std::ranges::sentinel_t<const R>>;

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_RANGE_TRAITS_HPP
