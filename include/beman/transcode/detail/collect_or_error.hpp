// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The loop behind decode_to_or_error and encode_to_or_error: run an _or_error
// view to completion into a container, or stop at the first element that
// holds an error and return that error instead.
//
// In the vocabulary of the typeclass libraries this is `sequence` over the
// Expected applicative -- a range of expected<T, E> becomes an
// expected<Container, E>, first error wins, elements examined left to right.
// It is written out here rather than taken from a library because it is five
// lines, and because the wording describes the result and not the mechanism.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_COLLECT_OR_ERROR_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_COLLECT_OR_ERROR_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <expected>
    #include <ranges>
    #include <utility>

#endif
namespace beman::transcoding::detail {

// Collect the values of `view`'s elements into a `Container`, in order, or
// return the error of the first element that holds one.  `view` is a range
// whose elements are `std::expected<T, Error>`.
template <typename Container, typename Error, std::ranges::input_range View>
constexpr std::expected<Container, Error> collect_or_error(View&& view);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <typename Container, typename Error, std::ranges::input_range View>
constexpr std::expected<Container, Error> collect_or_error(View&& view) {
    Container out;
    for (auto&& element : view) {
        if (!element)
            return std::unexpected(element.error());
        out.push_back(*element);
    }
    return out;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_COLLECT_OR_ERROR_HPP
