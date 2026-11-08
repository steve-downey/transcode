// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Turning a whatwg::encoding known only at run time into the codec for it.
// Implementation detail of whatwg/dynamic_codec.hpp and the runtime bulk
// helpers.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_DISPATCH_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_DISPATCH_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/whatwg_encode_select.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <cstddef>
    #include <optional>
    #include <type_traits>
    #include <utility>
    #include <variant>

#endif
namespace beman::transcoding::detail {

// The enumerators are numbered from zero with no gaps, so an encoding's value
// is its index in a variant over every encoding.
inline constexpr std::size_t whatwg_encoding_count = static_cast<std::size_t>(whatwg::encoding::euc_kr) + 1;

// Stands in for whatwg::encoder<C> where the Encoding Standard defines no
// encoder, so that the encoder variant keeps one alternative per encoding.
// make_dynamic_encoder never creates a dynamic_encoder holding one; if one
// existed, every scalar value would be an error.
template <whatwg::encoding C>
struct no_encoder {
    constexpr encode_result encode_one(char32_t) const {
        encode_result r;
        r.is_error = true;
        return r;
    }
    friend constexpr bool operator==(const no_encoder&, const no_encoder&) = default;
};

// Selected by specialization, not std::conditional_t: naming
// whatwg::encoder<C> for an encoding without an encoder is ill-formed even in
// the branch that is not taken.
template <whatwg::encoding C, bool = whatwg_encode_codec<C>>
struct encoder_or_placeholder_of {
    using type = no_encoder<C>;
};

template <whatwg::encoding C>
struct encoder_or_placeholder_of<C, true> {
    using type = whatwg::encoder<C>;
};

template <whatwg::encoding C>
using encoder_or_placeholder = encoder_or_placeholder_of<C>::type;

template <std::size_t... I>
auto decoder_variant_for(std::index_sequence<I...>)
    -> std::variant<whatwg::decoder<static_cast<whatwg::encoding>(I)>...>;

template <std::size_t... I>
auto encoder_variant_for(std::index_sequence<I...>)
    -> std::variant<encoder_or_placeholder<static_cast<whatwg::encoding>(I)>...>;

using decoder_variant = decltype(decoder_variant_for(std::make_index_sequence<whatwg_encoding_count>{}));
using encoder_variant = decltype(encoder_variant_for(std::make_index_sequence<whatwg_encoding_count>{}));

template <typename Variant, std::size_t... I>
constexpr Variant make_variant_at(whatwg::encoding e, std::index_sequence<I...>) {
    Variant    v;
    const auto index = static_cast<std::size_t>(e);
    ((index == I ? (void)v.template emplace<I>() : void()), ...);
    return v;
}

constexpr decoder_variant make_decoder_variant(whatwg::encoding e) {
    return make_variant_at<decoder_variant>(e, std::make_index_sequence<whatwg_encoding_count>{});
}

constexpr encoder_variant make_encoder_variant(whatwg::encoding e) {
    return make_variant_at<encoder_variant>(e, std::make_index_sequence<whatwg_encoding_count>{});
}

// Calls f(std::integral_constant<whatwg::encoding, E>{}) for the E that equals
// e, once, so that a whole-buffer operation dispatches on the encoding once
// and then runs the statically typed loop for it.  Every call to f must return
// the same type.
template <typename F, std::size_t... I>
constexpr auto with_encoding_impl(whatwg::encoding e, F& f, std::index_sequence<I...>) {
    using R = decltype(f(std::integral_constant<whatwg::encoding, whatwg::encoding{}>{}));
    if constexpr (std::is_void_v<R>) {
        (void)((static_cast<std::size_t>(e) == I
                    ? (f(std::integral_constant<whatwg::encoding, static_cast<whatwg::encoding>(I)>{}), true)
                    : false) ||
               ...);
    } else {
        std::optional<R> result;
        (void)((static_cast<std::size_t>(e) == I
                    ? (result.emplace(f(std::integral_constant<whatwg::encoding, static_cast<whatwg::encoding>(I)>{})),
                       true)
                    : false) ||
               ...);
        return R(std::move(*result));
    }
}

template <typename F>
constexpr auto with_encoding(whatwg::encoding e, F&& f) {
    return with_encoding_impl(e, f, std::make_index_sequence<whatwg_encoding_count>{});
}

// Whether whatwg::encoder<e> exists: the same question whatwg::has_encoder
// answers, decided by the concept that constrains whatwg::encoder.
constexpr bool has_whatwg_encoder(whatwg::encoding e) {
    return with_encoding(
        e, []<whatwg::encoding E>(std::integral_constant<whatwg::encoding, E>) { return whatwg_encode_codec<E>; });
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_WHATWG_DISPATCH_HPP
