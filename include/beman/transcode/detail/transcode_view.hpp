// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_VIEW_HPP

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>

namespace beman::transcoding {

template <codec From, codec To>
struct transcode_closure {
    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const transcode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const transcode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to transcode; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

template <codec From, codec To>
inline constexpr transcode_closure<From, To> transcode{};

// Out-of-line definition: transcode_closure::operator()
template <codec From, codec To>
template <legacy_byte_range R>
constexpr auto transcode_closure<From, To>::operator()(R&& r) const {
    return std::forward<R>(r) | whatwg_decode<From> | whatwg_encode<To>;
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_TRANSCODE_VIEW_HPP
